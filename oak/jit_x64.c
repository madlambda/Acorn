/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include <oak/instance.h>

#include <stddef.h>     /* offsetof(s, m) */
#include <sys/types.h>
#include <errno.h>
#include "opcodes.h"
#include "jit.h"
#include "bin.h"
#include "x64.h"


/* emitters push instructions onto blocks */
static Error *emitprologue(Block *b, i64 *allocsize);
static Error *emitepilogue(Block *b, i64 *restoresize);
static Error *emitmovimmreg(Block *b, i32 src, Reg dst);
static Error *emitfuncall(Block *b, u32 fn);
static Error *emitpreparelocals(Block *b, Array *scratch, Array *usedregs,
    u32 nlocals, u32 nrets, i64 *spsize);


void
pushreg(Array *a, Reg r)
{
    arraypush(a, &r);
}


Reg
popreg(Array *a)
{
    Reg  *r;

    r = arraypop(a);
    return *r;
}


Error *
x64compile(Module *m, Function *fn)
{
    u8        *begin;
    i32       i32val;
    u32       u32val;
    Reg       reg;
    Error     *err;
    Array     *calleeregs;
    Array     *callerregs;
    Array     *usedregs;
    Jitfn     *j;
    Block     *current;
    TypeDecl  *fndecl;
    const u8  *end;

    j = &fn->jitfn;

    calleeregs = newarray(6, sizeof(Reg));
    expect(calleeregs != NULL);

    callerregs = newarray(10, sizeof(Reg));
    expect(callerregs != NULL);

    usedregs = newarray(10, sizeof(Reg));
    expect(usedregs != NULL);

    pushreg(callerregs, R15);
    pushreg(callerregs, R14);
    pushreg(callerregs, R13);
    pushreg(callerregs, R12);
    pushreg(callerregs, RBX);

    pushreg(calleeregs, R11);
    pushreg(calleeregs, R10);
    pushreg(calleeregs, RBP);
    pushreg(calleeregs, R9);
    pushreg(calleeregs, R8);
    pushreg(calleeregs, RCX);
    pushreg(calleeregs, RDX);

    begin = (u8 *) fn->code->start;
    end = (u8 *) fn->code->end;

    current = arrayget(j->blocks, 0);

    j->allocstack = 0x10; /* fn(Function, Locals) */

    emitprologue(current, &j->allocstack);

    while (begin < end) {
        switch (*begin) {
        case Opreturn:
            emitepilogue(current, &j->allocstack);
            break;

        case Opi32const:
            begin++;

            if (slow(s32vdecode(&begin, end, &i32val) != OK)) {
                return newerror("failed to decode i32.const");
            }

            reg = popreg(calleeregs);
            pushreg(usedregs, reg);

            emitmovimmreg(current, i32val, reg);
            break;
        case Opcall:
            begin++;
            if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
                return newerror("failed to decode");
            }

            fndecl = arrayget(m->functypes, u32val);
            if (slow(fn == NULL)) {
                return newerror("function %d not found", u32val);
            }

            if (slow(len(usedregs) < len(fndecl->params))) {
                return newerror("function expects %d args but given %d",
                                len(fndecl->params), len(usedregs));
            }

            err = emitpreparelocals(current, calleeregs, usedregs, 10, 10,
                                    &j->allocstack);

            if (slow(err != NULL)) {
                return error(err, "preparing locals for funcall");
            }

            err = emitfuncall(current, u32val);
            if (slow(err != NULL)) {
                return error(err, "preparing funcall");
            }

            break;

        default:
            return newerror("unrecognized opcode %x", *begin);
        }
    }

    err = emitepilogue(current, &j->allocstack);
    if (slow(err != NULL)) {
        return err;
    }

    copyptr((ptr) &fn->fn, (ptr) &j->data);

    freearray(usedregs);
    freearray(calleeregs);
    freearray(callerregs);
    return NULL;
}


static Error *
emitprologue(Block *block, i64 *allocsize)
{
    Insdata  data;

    data.encoder = prologue;
    data.args.stacksize = allocsize;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
emitepilogue(Block *block, i64 *restoresize)
{
    Insdata  data;

    data.encoder = epilogue;
    data.args.stacksize = restoresize;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
emitmovimmreg(Block *block, i32 src, Reg dst)
{
    Insdata  data;

    immreg(&data.args, src, dst);
    data.encoder = movq;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
emitfuncall(Block *block, u32 fnindex)
{
    Insdata   data;

    indreg(&data.args, 0, RSP, RAX);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    indreg(&data.args, offsetof(Function, instance), RAX, RDI);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    indreg(&data.args, offsetof(Instance, getfn), RDI, RAX);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immreg(&data.args, fnindex, RSI);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    relreg(&data.args, RAX);
    data.encoder = callq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    regreg(&data.args, RAX, RDI);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    indreg(&data.args, 0x18, RSP, RSI);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    indreg(&data.args, offsetof(Function, fn), RAX, RAX);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    relreg(&data.args, RAX);
    data.encoder = callq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
emitpreparelocals(Block *block, Array *scratch, Array *usedregs, u32 nlocals,
    u32 nrets, i64 *spsize)
{
    u32      i, totallocals, localsoff, retsoff;
    Reg      r1, r2, tmp;
    Insdata  data;

    totallocals = len(usedregs) + nlocals;

    *spsize += sizeof(Local) + arraytotalsize(totallocals, sizeof(Value))
                + arraytotalsize(nrets, sizeof(Value));

    r1 = popreg(scratch);
    r2 = popreg(scratch);

    /* r1 = localbuf */
    indreg(&data.args, 0x18, RSP, r1);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    /* r1 = localbuf.locals */
    immreg(&data.args, offsetof(Local, locals), r1);
    data.encoder = add;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    localsoff = 0x18 + sizeof(Local);

    /* r2 = localsarraybuf */
    indreg(&data.args, localsoff, RSP, r2);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    /* localsbuf.locals = localsarraybuf */
    regind(&data.args, r2, r1, 0);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, len(usedregs), r2, offsetof(Array, len));
    data.encoder = movl;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, totallocals, r2, offsetof(Array, nalloc));
    data.encoder = movl;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, sizeof(Value), r2, offsetof(Array, size));
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, 0, r2, offsetof(Array, heap));
    data.encoder = movb;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    localsoff += sizeof(Array);
    indreg(&data.args, localsoff, RSP, r1);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    regind(&data.args, r1, r2, offsetof(Array, items));
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    for (i = 0; i < len(usedregs); i++) {
        tmp = popreg(usedregs);

        regind(&data.args, tmp, r1, i * sizeof(Value) + offsetof(Value, u.ival));
        data.encoder = movl;
        if (slow(arrayadd(block->insdata, &data) != OK)) {
            return newerror("failed to add instruction to block");
        }
    }

    /* RCX = localbuf */
    indreg(&data.args, 0x18, RSP, r1);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    /* RCX = localbuf.returns */
    immreg(&data.args, offsetof(Local, returns), r1);
    data.encoder = add;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    /* RDX = returnsarraybuf */
    retsoff = localsoff + (sizeof(Value) * totallocals);
    indreg(&data.args, retsoff, RSP, r2);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    /* localbuf.returns = returnsarraybuf */
    regind(&data.args, r2, r1, 0);
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, 0, r2, offsetof(Array, len));
    data.encoder = movl;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, nrets, r2, offsetof(Array, nalloc));
    data.encoder = movl;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, sizeof(Value), r2, offsetof(Array, size));
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    immind(&data.args, 0, r2, offsetof(Array, heap));
    data.encoder = movb;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    retsoff += sizeof(Array);
    indreg(&data.args, localsoff, RSP, r1);
    data.encoder = lea;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    regind(&data.args, r1, r2, offsetof(Array, items));
    data.encoder = movq;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    pushreg(scratch, r2);
    pushreg(scratch, r1);

    return NULL;
}

