/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>

#include <oak/module.h>
#include <oak/instance.h>
#include "jit.h"

#include <string.h>
#include <alloca.h>

int test1();
int test2();


/*
// calls other
static Error *
imported_func(Function *thisfn, Local * unused(local))
{
    Value     value;
    Local     l;
    Function  *fn;
    u8        localsbuf[arraytotalsize(1, sizeof(Value))];
    u8        retsbuf[arraytotalsize(0, sizeof(Value))];

    fn = arrayget(thisfn->instance->funcs, 0);

    l.locals = (Array *) localsbuf;
    l.returns = (Array *) retsbuf;

    initarrayfrom(l.locals, 1, sizeof(Value), 0);
    initarrayfrom(l.returns, 0, sizeof(Value), 0);

    value.type = I32;
    value.u.ival = 0x69;
    arraypush(l.locals, &value);

    return fn->fn(fn, &l);
}
*/


static Error *
imported_func(Function * unused(thisfn), Local *local)
{
    Value  *v;

    if (slow(len(local->locals) != 1)) {
        return newerror("invalid number of locals: %d != %d",
                        len(local->locals), 1);
    }

    v = arraypop(local->locals);
    cprint("imported_func called: %d(u32)\n", v->u.ival);

    return NULL;
}


int main() {
    fmtadd('e', errorfmt);
    fmtadd('o', oakfmt);

    if (test1() != 0) {
        return 1;
    }

    return test2();
}


int
test1()
{
    u32       i;
    Error     *err;
    Array     *imports;
    Module    mod;
    Import    import;
    Instance  ins;

    err = loadmodule(&mod, "testdata/ok/call1/input.wasm");
    if (slow(err != NULL)) {
        cprint("error compiling module: %e\n", err);
        errorfree(err);
        return 1;
    }

    err = compile(&mod);
    if (slow(err != NULL)) {
        cprint("error: %e\n", err);
        return 1;
    }

    ins.module = &mod;

    imports = newarray(1, sizeof(Import));
    if (slow(imports == NULL)) {
        return 1;
    }

    import.kind = FunctionKind;
    cstr(&import.field, "imported_func");
    cstr(&import.module, "imports");
    import.fn = imported_func;

    if (slow(arrayadd(imports, &import) != OK)) {
        return 1;
    }

    err = instantiate(&ins, imports);
    if (slow(err != NULL)) {
        cprint("error instantiating module: %e\n", err);
        return 1;
    }

    Array  *params = newarray(10, sizeof(Value));

    err = vminvoke(&ins, "exported_func", params);
    if (slow(err != NULL)) {
        cprint("error running: %e\n", err);
        return 1;
    }

    for (i = 0; i < len(params); i++) {
        Value *v = arrayget(params, i);
        cprint("function returned %o(value)\n", v);
    }

    freearray(params);
    freearray(imports);
    closemodule(&mod);
    closeinstance(&ins);

    return 0;
}


int
test2()
{
    Error   *err;
    Module  m;

    err = loadmodule(&m, "testdata/ok/emptyfunc/input.wasm");
    if (slow(err != NULL)) {
        cprint("error: %e\n", err);
        errorfree(err);
        return 1;
    }

    err = compile(&m);
    if (slow(err != NULL)) {
        cprint("error: %e\n", err);
        errorfree(err);
        return 1;
    }

    closemodule(&m);

    return 0;
}
