/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>

#include <oak/file.h>
#include <oak/module.h>
#include <oak/instance.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

#include "bin.h"
#include "opcodes.h"
#include "jit.h"


#define WASMPAGESIZE    65536
#define roundup(x)      ((x + WASMPAGESIZE-1) & ~(WASMPAGESIZE - 1))


static Error *initmem(Instance *instance);
static ExportDecl *searchexport(Module *m, String *field);


Error *
instantiate(Instance *ins, Array *imports)
{
    u32         i, j;
    Error       *err;
    Module      *m;
    Import      *import;
    Function    fn, *pfn;
    ImportDecl  *importdecl;

    m = ins->module;

    ins->mem.data = NULL;
    ins->mem.len = 0;

    err = initmem(ins);
    if (slow(err != NULL)) {
        return err;
    }

    ins->funcs = newarray(len(imports) + len(m->funcs), sizeof(Function));
    if (slow(ins->funcs == NULL)) {
        return newerror("failed to create function space");
    }

    if (slow(len(m->imports) > len(imports))) {
        return newerror("missing imports");
    }

    /* TODO(i4k): build a hash map at compile() time */
    for (i = 0; i < len(m->imports); i++) {
        importdecl = arrayget(m->imports, i);

        for (j = 0; j < len(imports); j++) {
            import = arrayget(imports, j);

            if (stringcmp(importdecl->module, &import->module)
                && stringcmp(importdecl->field, &import->field))
            {
                fn.fn = import->fn;
                fn.sig = importdecl->u.type;

                /* TODO(i4k): grab module's instance */
                fn.instance = ins;
                fn.code = NULL;

                if (slow(arrayadd(ins->funcs, &fn) != OK)) {
                    return newerror("failed to add function");
                }
            }
        }
    }

    if (slow(len(ins->funcs) != len(m->imports))) {
        return newerror("some imports are missing");
    }

    for (i = 0; i < len(m->funcs); i++) {
        pfn = arrayget(m->funcs, i);
        pfn->instance = ins;

        if (slow(arrayadd(ins->funcs, pfn) != OK)) {
            return newerror("failed to add function");
        }
    }

    /* TODO(i4k): we should detect the maximum stack needed during vldt */
    ins->stack = newarray(32, sizeof(Value));
    if (slow(ins->stack == NULL)) {
        return newerror("failed to allocate stack");
    }

    return NULL;
}


static Error *
initmem(Instance *ins)
{
    Module      *m;
    MemoryDecl  *memdecl;

    m = ins->module;

    if (len(m->memories) < 1) {
        return NULL;
    }

    memdecl = arrayget(m->memories, 0);

    ins->mem.len = roundup(memdecl->limit.initial);
    ins->mem.data = zmalloc(ins->mem.len);
    if (slow(ins->mem.data == NULL)) {
        return newerror("allocating memory: %s", strerror(errno));
    }

    return NULL;
}


void
closeinstance(Instance *ins)
{
    if (ins->mem.len > 0) {
        free(ins->mem.data);
    }

    freearray(ins->stack);

    if (ins->funcs != NULL) {
        freearray(ins->funcs);
    }
}


Error *
vminvoke(Instance *ins, const char *func, Array *returns)
{
    u32         i, nlocals, nrets;
    Error       *err;
    Value       value;
    Local       local;
    String      field;
    Module      *m;
    Function    *fn, *function;
    ExportDecl  *export;

    m = ins->module;

    cstr(&field, (u8 *) func);

    export = searchexport(m, &field);
    if (slow(export == NULL)) {
        return newerror("export not found");
    }

    if (slow(export->kind != FunctionKind)) {
        return newerror("export is not a function but %o(extkind)", export->kind);
    }

    function = NULL;

    for (i = 0; i < len(ins->funcs); i++) {
        fn = arrayget(ins->funcs, i);

        if (fn->sig.index == export->u.type.index) {
            function = fn;
        }
    }

    expect(function != NULL);

    /* TODO(i4k): allocate in the heap if nlocals + nrets > MAXALLOCA */

    nlocals = nlocalitems(function);
    nrets   = len(function->sig.rets);

    local.locals = alloca(arraytotalsize(nlocals, sizeof(Value)));
    local.returns = alloca(arraytotalsize(nrets, sizeof(Value)));

    initarrayfrom(local.locals, nlocals, sizeof(Value), 0);
    initarrayfrom(local.returns, nrets, sizeof(Value), 0);

    /* TODO(i4k): pass 1337 as params for now */

    for (i = 0; i < len(function->sig.params); i++) {
        value.type = I32;
        value.u.ival = 1337;

        arrayadd(local.locals, &value);
    }

    for (i = 0; i < len(function->code->locals); i++) {
        value.type = I32;
        value.u.ival = 0;

        arrayadd(local.locals, &value);
    }

    expect(len(local.locals) ==
           (len(function->sig.params) + len(function->code->locals)));

    if (function->fn != interp) {
        err = mkexec(&function->jitfn);
        if (slow(err != NULL)) {
            return err;
        }
    }

    err = function->fn(function, &local);
    if (slow(err != NULL)) {
        return error(err, "invoking function %s: %e", func, err);
    }

    if (function->fn != interp) {
        freejit(&function->jitfn);
    }

    for (i = 0; i < len(local.returns); i++) {
        if (slow(arrayadd(returns, arrayget(local.returns, i)) != OK)) {
            return newerror("failed to add elements");
        }
    }

    return NULL;
}


Error *
interp(Function *thisfn, Local *local)
{
    u8        *begin, *end;
    i32       i32val;
    u32       i, u32val;
    Error     *err;
    Array     *sp;
    Value     constant;
    Local     l;
    Function  *fn;
    Instance  *ins;

    begin = (u8 *) thisfn->code->start;
    end = (u8 *) thisfn->code->end;

    ins = thisfn->instance;
    sp = ins->stack;    /* TODO(i4k): alloca() */
    sp->len = 0;

#define maxstacklocals  5
#define maxstackreturns 2
    /* default argument and return size, use heap otherwise */
    l.locals = alloca(arraytotalsize(maxstacklocals, sizeof(Value)));
    l.returns = alloca(arraytotalsize(maxstackreturns, sizeof(Value)));

    initarrayfrom(l.locals, maxstacklocals, sizeof(Value), 0);
    initarrayfrom(l.returns, maxstackreturns, sizeof(Value), 0);

    while (begin < end) {
        switch (*begin) {
        case Opreturn:
            if (slow(len(sp) != local->returns->nalloc)) {
                return newerror("return %d values but expect %d", len(sp),
                                local->returns->nalloc);
            }

            while (len(sp) > 0) {
                arraypush(local->returns, arraypop(sp));
            }

            return OK;
            break;

        case Opi32const:
            begin++;

            if (slow(s32vdecode(&begin, end, &i32val) != OK)) {
                return newerror("failed to decode i32.const");
            }

            constant.type = I32;
            constant.u.ival = i32val;
            arraypush(sp, &constant);
            break;
        case Opcall:
            begin++;
            if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
                return newerror("failed to decode");
            }

            fn = arrayget(ins->funcs, u32val);
            if (slow(fn == NULL)) {
                return newerror("function %d not found", u32val);
            }

            if (slow(len(sp) < len(fn->sig.params))) {
                return newerror("function expects %d args but given %d",
                                len(fn->sig.params), len(sp));
            }

            if(slow(nlocalitems(fn) > maxstacklocals)) {
                return newerror("TODO: max stack locals, add heap support");
            }

            for (i = 0; i < len(fn->sig.params); i++) {
                if (slow(arrayadd(l.locals, arraypop(sp)) != OK)) {
                    return newerror("failed to populate locals");
                }
            }

            if (nlocalitems(fn) > len(fn->sig.params)) {
                arrayzerorange(l.locals, len(fn->sig.params), nlocalitems(fn) - 1);
            }

            err = fn->fn(fn, &l);
            if (slow(err != NULL)) {
                return error(err, "executing %d: %e", u32val, err);
            }

            break;

        default:
            return newerror("unrecognized opcode %x", *begin);
        }
    }

    return OK;
}


static ExportDecl *
searchexport(Module *m, String *field)
{
    u32         i;
    ExportDecl  *export;

    for (i = 0; i < len(m->exports); i++) {
        export = arrayget(m->exports, i);
        if (stringcmp(export->field, field)) {
            return export;
        }
    }

    return NULL;
}
