/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/file.h>
#include <oak/module.h>
#include <oak/instance.h>
#include "jit.h"
#include "test.h"


typedef struct {
    const char  *input;
    const char  *err;
    const char  *want;
} Testcase;


static u8 fmtjit(String **buf, u8 ** format, void *val);
static Error *test_jit();


static const Testcase  testcases[] = {
    {
        "testdata/ok/emptyfunc/input.wasm",
        NULL,
        "testdata/ok/emptyfunc/output.jit"
    },
    /*
    {
        "testdata/ok/call1/input.wasm",
        NULL,
        "testdata/ok/call1/output.jit"
    },
    */
};


int
main()
{
    u32             i;
    Error           *err;
    const Testcase  *tc;

    fmtadd('e', errorfmt);
    fmtadd('o', oakfmt);
    fmtadd('J', fmtjit);

    for (i = 0; i < nitems(testcases); i++) {
        tc = &testcases[i];

        err = test_jit(tc);
        if (slow(err != NULL)) {
            cprint("error: %e\n", err);
            return 1;
        }
    }

    return 0;
}


static Error *
test_jit(Testcase *tc)
{
    u32         i;
    File        file;
    Error       *err;
    Jitfn       *jit;
    Module      m;
    Binbuf      got, want;
    Function    *fn;

    err = loadmodule(&m, tc->input);
    if (err != NULL) {
        return err;
    }

    err = compile(&m);
    if (err != NULL) {
        if (tc->err == NULL) {
            return error(err, "tc[%s] must not fail", tc->input);
        }

        if (!iserror(err, tc->err)) {
            return error(err, "error mismatch: expected: \"%s\"\n", tc->err);
        }

        errorfree(err);

        return NULL;
    }

    err = openfile(&file, tc->want);
    if (slow(err != NULL)) {
        return err;
    }

    err = readbinary(&file, &want);
    if (slow(err != NULL)) {
        return err;
    }

    fn = arrayget(m.funcs, 0);
    jit = &fn->jitfn;
    got.code = jit->data;
    got.size = (jit->begin - jit->data);

    if (slow(got.size != want.size)) {
        return newerror("expected jitted code to have %d bytes but got %d (%s)"
                        ":\n%J !=\n%J",
                        want.size, got.size, tc->want, &want, &got);
    }

    if (slow(memcmp(got.code, want.code, got.size) != 0)) {
        return newerror("jitted code mismatch:\n%J !=\n%J\n for (%s)",
                        &want, &got, tc->want);
    }

    closefile(&file);
    closemodule(&m);

    return NULL;
}
