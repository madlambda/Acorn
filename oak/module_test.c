/*
 * Copyright (C) Madlambda Authors.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include "test.h"
#include "testdata/ok/empty.h"
#include "testdata/ok/call1.h"
#include "testdata/ok/globalconstant.h"


typedef struct {
    const char  *filename;
    const char  *err;
    Module      *module;
} Testcase;


typedef Error *(*Assert)(const void *got, const void *want);


static Error *test_module(const Testcase *tc);
static Error *assertmodule(const Module *got, const Module *want);
static Error *assertarray(Array *got, Array *want, const char *name, Assert a);

static Error *assertsect(const void *got, const void *want);
static Error *asserttypedecl(const void *got, const void *want);
static Error *asserttype(const void *got, const void *want);
static Error *assertimportdecl(const void *got, const void *want);
static Error *asserttabledecl(const void *got, const void *want);
static Error *assertresizablelimit(const void *got, const void *want);
static Error *assertmemorydecl(const void *got, const void *want);
static Error *assertglobaldecl(const void *got, const void *want);
static Error *assertexportdecl(const void *got, const void *want);
static Error *assertcodedecl(const void *got, const void *want);
static Error *assertlocaldecl(const void *gotv, const void *wantv);


static const Testcase  invalid_cases[] = {
    {
        "testdata/invalid/notwasm",
        "WASM must have at least 8 bytes",
        NULL,
    },
    {
        "testdata/ok/empty.wasm",
        NULL,
        &emptymod,
    },
    {
        "testdata/ok/call1.wasm",
        NULL,
        &call1mod,
    },
    {
        "testdata/ok/globalconstant.wasm",
        NULL,
        &globalconstmod,
    },
};


int main()
{
    u32    i;
    Error  *err;

    fmtadd('e', errorfmt);

    for (i = 0; i < nitems(invalid_cases); i++) {
        err = test_module(&invalid_cases[i]);
        if (slow(err != NULL)) {
            cprint("[error] %e\n", err);
            errorfree(err);
            return 1;
        }
    }

    return 0;
}


static Error *
test_module(const Testcase *tc)
{
    Error   *err;
    Module  m;

    err = loadmodule(&m, tc->filename);
    if (err != NULL) {
        if (tc->err == NULL) {
            return error(err, "tc[%s] must not fail", tc->filename);
        }

        if (!iserror(err, tc->err)) {
            return error(err, "error mismatch: expected: \"%s\"\n", tc->err);
        }

        errorfree(err);

        return NULL;
    }

    err = assertmodule(&m, tc->module);
    closemodule(&m);
    return err;
}


static Error *
assertmodule(const Module *m1, const Module *m2)
{
    Error  *err;

    if (m1 == m2) {
        return NULL;
    }

    if (slow(m1 == NULL || m2 == NULL)) {
        return newerror("module mismatch (%p != %p)", m1, m2);
    }

    if (slow(m1->version != m2->version)) {
        return newerror("version mismatch (%d != %d)", m1->version, m2->version);
    }

    err = assertarray(m1->sects, m2->sects, "sect", assertsect);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->types, m2->types, "types", asserttypedecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->imports, m2->imports, "imports", assertimportdecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->funcs, m2->funcs, "funcs", asserttypedecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->tables, m2->tables, "tables", asserttabledecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->memories, m2->memories, "memories", assertmemorydecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->globals, m2->globals, "globals", assertglobaldecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->exports, m2->exports, "exports", assertexportdecl);
    if (slow(err != NULL)) {
        return err;
    }

    err = assertarray(m1->codes, m2->codes, "codes", assertcodedecl);
    if (slow(err != NULL)) {
        return err;
    }

    return NULL;
}


static Error *
assertarray(Array *got, Array *want, const char *name, Assert assertion)
{
    u32    i;
    void   *p1, *p2;
    Error  *err;

    if (got != want) {
        if (slow(got == NULL || want == NULL)) {
            return newerror("%s mismatch (%p != %p)", name, got, want);
        }

        if (slow(len(got) != len(want))) {
            return newerror("%s len mismatch (%d != %d)", name,
                            len(got), len(want));
        }

        for (i = 0; i < len(want); i++) {
            p1 = arrayget(got, i);
            p2 = arrayget(want, i);
            err = assertion(p1, p2);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    return NULL;
}


static Error *
assertsect(const void *gotv, const void *wantv) {
    u32            i;
    const Section  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->id != want->id)) {
        return newerror("section id mismatch (%d != %d) (%d %d)",
                        got->id, want->id, got->len, want->len);
    }

    if (slow(got->len != want->len)) {
        return newerror("section len mismatch (%d != %d)",
                         got->len, want->len);
    }

    if (slow(memcmp(got->data, want->data, got->len) != 0)) {
        printf("got: ");
        for (i = 0; i < got->len; i++) {
            printf("%02x ", got->data[i]);
        }

        printf("\nwant: ");
        for (i = 0; i < want->len; i++) {
            printf("%02x ", want->data[i]);
        }

        puts("");

        return newerror("section data mismatch");
    }

    return NULL;
}


static Error *
asserttypedecl(const void *gotv, const void *wantv)
{
    Error           *err;
    const FuncDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->form != want->form)) {
        return newerror("funcdecl form mismatch (%x != %x)",
                        got->form, want->form);
    }

    err = assertarray(got->params, want->params, "params", asserttype);
    if (slow(err != NULL)) {
        return err;
    }

    return assertarray(got->rets, want->rets, "rets", asserttype);
}


static Error *
asserttype(const void *got, const void *want)
{
    const Type  *p1, *p2;

    p1 = got;
    p2 = want;

    if (p1 == p2) {
        return NULL;
    }

    if (slow((p1 == NULL || p2 == NULL) || (*p1 != *p2))) {
        return newerror("return mismatch (%d != %d)",
                        (p1 == NULL) ? 0 : *p1,
                        (p2 == NULL ? 0 : *p2));
    }

    return NULL;
}


static Error *
assertimportdecl(const void *gotv, const void *wantv)
{
    const ImportDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(!stringcmp(got->module, want->module))) {
        return newerror("import module string mismatch: (%S) != (%S)",
                        got->module, want->module);
    }

    if (slow(!stringcmp(got->field, want->field))) {
        return newerror("import field string mismatch: (%S) != (%S)",
                        got->field, want->field);
    }

    if (slow(got->kind != want->kind)) {
        return newerror("import kind mismatch (%d != %d)",
                        got->kind, want->kind);
    }

    switch (want->kind) {
    case Function:
        return asserttypedecl(&got->u.function, &want->u.function);
        break;
    default:
        return newerror("import data not implemented for %d", want->kind);
    }

    return NULL;
}


static Error *
asserttabledecl(const void *gotv, const void *wantv)
{
    const TableDecl *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->type != want->type)) {
        return newerror("table type mismatch (%d != %d)",
                        got->type, want->type);
    }

    return assertresizablelimit(&got->limit, &want->limit);
}


static Error *
assertresizablelimit(const void *gotv, const void *wantv)
{
    const ResizableLimit  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->flags != want->flags)) {
        return newerror("resizable limit flags mismatch (%d != %d)",
                        got->flags, want->flags);
    }

    if (slow(got->initial != want->initial)) {
        return newerror("resizable limit initial mismatch (%d != %d)",
                        got->initial, want->initial);
    }

    if (slow(got->maximum != want->maximum)) {
        return newerror("resizable limit maximum mismatch (%d != %d)",
                        got->maximum, want->maximum);
    }

    return NULL;
}


static Error *
assertmemorydecl(const void *gotv, const void *wantv)
{
    const MemoryDecl  *got, *want;

    got = gotv;
    want = wantv;

    return assertresizablelimit(&got->limit, &want->limit);
}


static Error *
assertglobaldecl(const void *gotv, const void *wantv)
{
    const GlobalDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->type.type != want->type.type)) {
        return newerror("global type mismatch (%d != %d)",
                        got->type.type, want->type.type);
    }

    if (slow(got->type.mut != want->type.mut)) {
        return newerror("global mutability mismatch (%d != %d)",
                        got->type.mut, want->type.mut);
    }

    if (slow(got->u.i32val != want->u.i32val)) {
        return newerror("global init data mismatch (%d != %d)",
                        got->u.i32val != want->u.i32val);
    }

    return NULL;
}

static Error *
assertexportdecl(const void *gotv, const void *wantv)
{
    const ExportDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(!stringcmp(got->field, want->field))) {
        return newerror("import field string mismatch: (%S) != (%S)",
                        got->field, want->field);
    }

    if (slow(got->kind != want->kind)) {
        return newerror("import kind mismatch (%d != %d)",
                        got->kind, want->kind);
    }

    if (slow(got->index != want->index)) {
        return newerror("import index mismatch (%d != %d)",
                        got->index, want->index);
    }

    return NULL;
}


static Error *
assertcodedecl(const void *gotv, const void *wantv)
{
    u32             gotsize, wantsize;
    Error           *err;
    const CodeDecl  *got, *want;

    got = gotv;
    want = wantv;

    err = assertarray(got->locals, want->locals, "code locals", assertlocaldecl);
    if (slow(err != NULL)) {
        return err;
    }

    if (slow(got->start > got->end)) {
        return newerror("invalid starts and end pointers in code data");
    }

    wantsize = (want->end - want->start);
    gotsize  = (got->end - got->start);

    if (slow(wantsize != gotsize)) {
        return newerror("code section size mismatch (%d != %d)",
                        gotsize, wantsize);
    }

    if (slow(memcmp(got->start, want->start, wantsize) != 0)) {
        return newerror("code data mismatch");
    }

    return NULL;
}


static Error *
assertlocaldecl(const void *gotv, const void *wantv)
{
    const LocalEntry  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->count != want->count)) {
        return newerror("code locals count mismatch (%d != %d)",
                        got->count, want->count);
    }

    return asserttype((const void *) got->type, (const void *) want->type);
}
