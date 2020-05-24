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

static Error *test_call1();
static Error *test_emptyfunc();
static Error *test_add();
static Error *test_addcall();


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
    Error  *err;

    fmtadd('e', errorfmt);
    fmtadd('o', oakfmt);

    err = test_call1();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_add();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_addcall();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_emptyfunc();
    if (slow(err != NULL)) {
        goto fail;
    }

    return 0;

fail:

    cprint("error: %e\n", err);
    errorfree(err);

    return 1;
}


static Error *
test_call1()
{
    Error     *err;
    Array     *imports, *returns;
    Module    mod;
    Import    import;
    Instance  ins;

    imports = NULL;
    returns = NULL;

    err = loadmodule(&mod, "testdata/ok/call1/input.wasm");
    if (slow(err != NULL)) {
        return error(err, "loading module");
    }

    err = compile(&mod);
    if (slow(err != NULL)) {
        closemodule(&mod);
        return error(err, "compiling");
    }

    ins.module = &mod;

    imports = newarray(1, sizeof(Import));

    import.kind = FunctionKind;
    cstr(&import.field, "imported_func");
    cstr(&import.module, "imports");
    import.fn = imported_func;

    if (slow(arrayadd(imports, &import) != OK)) {
        err = newerror("failed to add import");
        goto free;
    }

    err = instantiate(&ins, imports);
    if (slow(err != NULL)) {
        goto free;
    }

    returns = newarray(0, sizeof(Value));

    err = vminvoke(&ins, "exported_func", NULL, returns);
    if (slow(err != NULL)) {
        err = error(err, "running");
        goto free;
    }

    if (slow(len(returns) != 0)) {
        err = newerror("expected no return");
        goto free;
    }

free:

    if (returns != NULL) {
        freearray(returns);
    }

    if (imports != NULL) {
        freearray(imports);
    }

    closemodule(&mod);
    closeinstance(&ins);

    return err;
}


static Error *
test_emptyfunc()
{
    Error   *err;
    Module  m;

    err = loadmodule(&m, "testdata/ok/emptyfunc/input.wasm");
    if (slow(err != NULL)) {
        return err;
    }

    err = compile(&m);
    closemodule(&m);

    return err;
}


static Error *
test_add()
{
    u32       i;
    Error     *err;
    Array     *imports, *params, *returns;
    Value     value;
    Module    mod;
    Instance  ins;

    params = NULL;
    returns = NULL;

    err = loadmodule(&mod, "testdata/ok/add/input.wasm");
    if (slow(err != NULL)) {
        return error(err, "loading module");
    }

    err = compile(&mod);
    if (slow(err != NULL)) {
        closemodule(&mod);
        return error(err, "compiling");
    }

    ins.module = &mod;
    imports = newarray(0, sizeof(Import));

    err = instantiate(&ins, imports);
    if (slow(err != NULL)) {
        err = error(err, "instantiating module");
        goto free;
    }

    params = newarray(2, sizeof(Value));
    returns = newarray(1, sizeof(Value));

    value.type = I32;
    value.u.ival = 1300;

    arrayadd(params, &value);

    value.u.ival = 37;

    arrayadd(params, &value);

    err = vminvoke(&ins, "add", params, returns);
    if (slow(err != NULL)) {
        err = error(err, "running");
        goto free;
    }

    for (i = 0; i < len(returns); i++) {
        Value *v = arrayget(returns, i);
        cprint("function returned %o(value)\n", v);
    }

free:

    if (params != NULL) {
        freearray(params);
    }

    if (returns != NULL) {
        freearray(returns);
    }

    freearray(imports);
    closemodule(&mod);
    closeinstance(&ins);

    return err;
}


static Error *
test_addcall()
{
    u32       i;
    Error     *err;
    Array     *imports, *params, *returns;
    Value     value;
    Module    mod;
    Import    import;
    Instance  ins;

    params = NULL;
    returns = NULL;

    err = loadmodule(&mod, "testdata/ok/addcall/input.wasm");
    if (slow(err != NULL)) {
        return error(err, "loading module");
    }

    err = compile(&mod);
    if (slow(err != NULL)) {
        closemodule(&mod);
        return error(err, "compiling");
    }

    ins.module = &mod;

    imports = newarray(1, sizeof(Import));

    import.kind = FunctionKind;
    cstr(&import.field, "printnum");
    cstr(&import.module, "imports");
    import.fn = imported_func;

    arrayadd(imports, &import);

    err = instantiate(&ins, imports);
    if (slow(err != NULL)) {
        err = error(err, "instantiating module");
        goto free;
    }

    params = newarray(2, sizeof(Value));
    returns = newarray(1, sizeof(Value));

    value.type = I32;
    value.u.ival = 1;

    arrayadd(params, &value);

    value.u.ival = 2;

    arrayadd(params, &value);

    err = vminvoke(&ins, "add", params, returns);
    if (slow(err != NULL)) {
        err = error(err, "running");
        goto free;
    }

    for (i = 0; i < len(returns); i++) {
        Value *v = arrayget(returns, i);
        cprint("function returned %o(value)\n", v);
    }

free:

    if (params != NULL) {
        freearray(params);
    }

    if (returns != NULL) {
        freearray(returns);
    }

    freearray(imports);
    closemodule(&mod);
    closeinstance(&ins);

    return err;
}