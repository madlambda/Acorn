#include <acorn.h>
#include <stdio.h>
#include <string.h>
#include "array.h"
#include "error.h"
#include "file.h"
#include "module.h"
#include "test.h"
#include "testdata/ok/empty.h"
#include "testdata/ok/call1.h"
#include "testdata/ok/globalconstant.h"


typedef struct {
    const char  *filename;
    const char  *err;
    Module      *module;
} Testcase;


typedef u8 (*Assert)(const void *got, const void *want, Error *err);


static u8 test_module(const Testcase *tc, Error *err);
static u8 assertmodule(const Module *got, const Module *want, Error *err);
static u8 assertarray(Array *got, Array *want, const char *name, Assert a,
    Error *err);

static u8 assertsect(const void *got, const void *want, Error *err);
static u8 asserttypedecl(const void *got, const void *want, Error *err);
static u8 asserttype(const void *got, const void *want, Error *err);
static u8 assertimportdecl(const void *got, const void *want, Error *err);
static u8 asserttabledecl(const void *got, const void *want, Error *err);
static u8 assertresizablelimit(const void *got, const void *want, Error *err);
static u8 assertmemorydecl(const void *got, const void *want, Error *err);
static u8 assertglobaldecl(const void *got, const void *want, Error *err);
static u8 assertexportdecl(const void *got, const void *want, Error *err);
static u8 assertcodedecl(const void *got, const void *want, Error *err);
static u8 assertlocaldecl(const void *gotv, const void *wantv, Error *err);


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
    Error  err;

    errorinit(&err);

    for (i = 0; i < nitems(invalid_cases); i++) {
        if (slow(test_module(&invalid_cases[i], &err) != OK)) {
            cprint("[error] %e\n", &err);
            return 1;
        }
    }

    return 0;
}


static u8
test_module(const Testcase *tc, Error *err)
{
    u8      ret;
    Error   goterr;
    Module  *m;

    errorinit(&goterr);

    m = loadmodule(tc->filename, &goterr);
    if (m == NULL) {
        if (tc->err == NULL) {
            return error(err, "tc[%s] must not fail: %e", tc->filename,
                         &goterr);
        }

        if (iserror(&goterr, tc->err) != 0) {
            return error(err, "error mismatch: \"%e\" != \"%s\"\n", &goterr,
                         tc->err);
        }

        return OK;
    }

    ret = assertmodule(m, tc->module, err);
    closemodule(m);
    return ret;
}


static u8
assertmodule(const Module *m1, const Module *m2, Error *err)
{
    u8  ret;

    if (m1 == m2) {
        return OK;
    }

    if (slow(m1 == NULL || m2 == NULL)) {
        return error(err, "module mismatch (%p != %p)", m1, m2);
    }

    if (slow(m1->version != m2->version)) {
        return error(err, "version mismatch (%d != %d)",
                     m1->version, m2->version);
    }

    ret = assertarray(m1->sects, m2->sects, "sect", assertsect, err);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->types, m2->types, "types", asserttypedecl, err);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->imports, m2->imports, "imports", assertimportdecl,
                      err);

    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->funcs, m2->funcs, "funcs", asserttypedecl, err);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->tables, m2->tables, "tables", asserttabledecl, err);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->memories, m2->memories, "memories", assertmemorydecl,
                      err);

    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->globals, m2->globals, "globals", assertglobaldecl,
                      err);

    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->exports, m2->exports, "exports", assertexportdecl,
                      err);

    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->codes, m2->codes, "codes", assertcodedecl, err);
    if (slow(ret != OK)) {
        return ret;
    }

    return OK;
}


static u8
assertarray(Array *got, Array *want, const char *name, Assert assertion,
    Error *err)
{
    u32   i;
    void  *p1, *p2;

    if (got != want) {
        if (slow(got == NULL || want == NULL)) {
            return error(err, "%s mismatch (%p != %p)", name, got, want);
        }

        if (slow(len(got) != len(want))) {
            return error(err, "%s len mismatch (%d != %d)", name,
                         len(got), len(want));
        }

        for (i = 0; i < len(want); i++) {
            p1 = arrayget(got, i);
            p2 = arrayget(want, i);
            if (slow(assertion(p1, p2, err) != OK)) {
                return ERR;
            }
        }
    }

    return OK;
}


static u8
assertsect(const void *gotv, const void *wantv, Error *err) {
    u32            i;
    const Section  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->id != want->id)) {
        return error(err, "section id mismatch (%d != %d) (%d %d)",
                     got->id, want->id, got->len, want->len);
    }

    if (slow(got->len != want->len)) {
        return error(err, "section len mismatch (%d != %d)",
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

        return error(err, "section data mismatch");
    }

    return OK;
}


static u8
asserttypedecl(const void *gotv, const void *wantv, Error *err)
{
    u8              ret;
    const FuncDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->form != want->form)) {
        return error(err, "funcdecl form mismatch (%x != %x)",
                     got->form, want->form);
    }

    ret = assertarray(got->params, want->params, "params", asserttype, err);
    if (slow(ret != OK)) {
        return ERR;
    }

    return assertarray(got->rets, want->rets, "rets", asserttype, err);
}


static u8
asserttype(const void *got, const void *want, Error *err)
{
    const Type  *p1, *p2;

    p1 = got;
    p2 = want;

    if (p1 == p2) {
        return OK;
    }

    if (slow((p1 == NULL || p2 == NULL) || (*p1 != *p2))) {
        return error(err, "return mismatch (%d != %d)",
                     (p1 == NULL) ? 0 : *p1,
                     (p2 == NULL ? 0 : *p2));
    }

    return OK;
}


static u8
assertimportdecl(const void *gotv, const void *wantv, Error *err)
{
    const ImportDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(!stringcmp(got->module, want->module))) {
        return error(err, "import module string mismatch: (%S) != (%S)",
                     got->module, want->module);
    }

    if (slow(!stringcmp(got->field, want->field))) {
        return error(err, "import field string mismatch: (%S) != (%S)",
                     got->field, want->field);
    }

    if (slow(got->kind != want->kind)) {
        return error(err, "import kind mismatch (%d != %d)",
                     got->kind, want->kind);
    }

    switch (want->kind) {
    case Function:
        return asserttypedecl(&got->u.function, &want->u.function, err);
        break;
    default:
        return error(err, "import data not implemented for %d", want->kind);
    }

    return OK;
}


static u8
asserttabledecl(const void *gotv, const void *wantv, Error *err)
{
    const TableDecl *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->type != want->type)) {
        return error(err, "table type mismatch (%d != %d)",
                     got->type, want->type);
    }

    return assertresizablelimit(&got->limit, &want->limit, err);
}


static u8
assertresizablelimit(const void *gotv, const void *wantv, Error *err)
{
    const ResizableLimit  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->flags != want->flags)) {
        return error(err, "resizable limit flags mismatch (%d != %d)",
                     got->flags, want->flags);
    }

    if (slow(got->initial != want->initial)) {
        return error(err, "resizable limit initial mismatch (%d != %d)",
                     got->initial, want->initial);
    }

    if (slow(got->maximum != want->maximum)) {
        return error(err, "resizable limit maximum mismatch (%d != %d)",
                     got->maximum, want->maximum);
    }

    return OK;
}


static u8
assertmemorydecl(const void *gotv, const void *wantv, Error *err)
{
    const MemoryDecl  *got, *want;

    got = gotv;
    want = wantv;

    return assertresizablelimit(&got->limit, &want->limit, err);
}


static u8
assertglobaldecl(const void *gotv, const void *wantv, Error *err)
{
    const GlobalDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->type.type != want->type.type)) {
        return error(err, "global type mismatch (%d != %d)",
                     got->type.type, want->type.type);
    }

    if (slow(got->type.mut != want->type.mut)) {
        return error(err, "global mutability mismatch (%d != %d)",
                     got->type.mut, want->type.mut);
    }

    if (slow(got->u.i32val != want->u.i32val)) {
        return error(err, "global init data mismatch (%d != %d)",
                     got->u.i32val != want->u.i32val);
    }

    return OK;
}

static u8
assertexportdecl(const void *gotv, const void *wantv, Error *err)
{
    const ExportDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(!stringcmp(got->field, want->field))) {
        return error(err, "import field string mismatch: (%S) != (%S)",
                     got->field, want->field);
    }

    if (slow(got->kind != want->kind)) {
        return error(err, "import kind mismatch (%d != %d)",
                     got->kind, want->kind);
    }

    if (slow(got->index != want->index)) {
        return error(err, "import index mismatch (%d != %d)",
                     got->index, want->index);
    }

    return OK;
}


static u8
assertcodedecl(const void *gotv, const void *wantv, Error *err)
{
    u8              ret;
    u32             gotsize, wantsize;
    const CodeDecl  *got, *want;

    got = gotv;
    want = wantv;

    ret = assertarray(got->locals, want->locals, "code locals",
                      assertlocaldecl, err);

    if (slow(ret != OK)) {
        return ERR;
    }

    if (slow(got->start > got->end)) {
        return error(err, "invalid starts and end pointers in code data");
    }

    wantsize = (want->end - want->start);
    gotsize  = (got->end - got->start);

    if (slow(wantsize != gotsize)) {
        return error(err, "code section size mismatch (%d != %d)",
                     gotsize, wantsize);
    }

    if (slow(memcmp(got->start, want->start, wantsize) != 0)) {
        return error(err, "code data mismatch");
    }

    return OK;
}


static u8
assertlocaldecl(const void *gotv, const void *wantv, Error *err)
{
    const LocalEntry  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->count != want->count)) {
        return error(err, "code locals count mismatch (%d != %d)",
                     got->count, want->count);
    }

    return asserttype((const void *) got->type, (const void *) want->type, err);
}
