#include <acorn.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "array.h"
#include "module.h"
#include "test.h"
#include "testdata/ok/empty.h"
#include "testdata/ok/call1.h"


typedef struct {
    const char  *filename;
    const char  *err;
    Module      *module;
} Testcase;


typedef u8 (*Assertion)(const void *got, const void *want);


static u8 test_module(const Testcase *tc);
static u8 assertmodule(const Module *got, const Module *want);
static u8 assertarray(Array *got, Array *want, const char *name, Assertion a);

static u8 assertsect(const void *got, const void *want);
static u8 asserttypedecl(const void *got, const void *want);
static u8 asserttype(const void *got, const void *want);
static u8 assertimportdecl(const void *got, const void *want);
static u8 asserttabledecl(const void *got, const void *want);
static u8 assertresizablelimit(const void *got, const void *want);
static u8 assertmemorydecl(const void *got, const void *want);
static u8 assertglobaldecl(const void *got, const void *want);
static u8 assertexportdecl(const void *got, const void *want);
static u8 assertcodedecl(const void *got, const void *want);
static u8 assertlocaldecl(const void *gotv, const void *wantv);


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
    }
};


int main()
{
    u32  i;

    for (i = 0; i < nitems(invalid_cases); i++) {
        if (slow(test_module(&invalid_cases[i]) != OK)) {
            return 1;
        }
    }

    return 0;
}


static u8
test_module(const Testcase *tc)
{
    u8      ret;
    Error   err;
    Module  *m;

    m = loadmodule(tc->filename, &err);
    if (m == NULL) {
        if (tc->err == NULL) {
            return error("tc[%s] must not fail: %s", tc->filename, err.msg);
        }

        if (strcmp(err.msg, tc->err) != 0) {
            printf("error mismatch: \"%s\" != \"%s\"\n", err.msg, tc->err);
            return ERR;
        }

        return OK;
    }

    ret = assertmodule(m, tc->module);
    closemodule(m);
    return ret;
}


static u8
assertmodule(const Module *m1, const Module *m2)
{
    u8          ret;

    if (m1 == m2) {
        return OK;
    }

    if (slow(m1 == NULL || m2 == NULL)) {
        return error("module mismatch (%p != %p)", m1, m2);
    }

    if (slow(m1->version != m2->version)) {
        return error("version mismatch (%d != %d)", m1->version, m2->version);
    }

    ret = assertarray(m1->sects, m2->sects, "sect", assertsect);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->types, m2->types, "types", asserttypedecl);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->imports, m2->imports, "imports", assertimportdecl);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->funcs, m2->funcs, "funcs", asserttypedecl);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->tables, m2->tables, "tables", asserttabledecl);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->memories, m2->memories, "memories", assertmemorydecl);
    if (slow(ret != OK)) {
        return ERR;
    }

    ret = assertarray(m1->globals, m2->globals, "globals", assertglobaldecl);
    if (slow(ret != OK)) {
        return ret;
    }

    ret = assertarray(m1->exports, m2->exports, "exports", assertexportdecl);
    if (slow(ret != OK)) {
        return ret;
    }

    ret = assertarray(m1->codes, m2->codes, "codes", assertcodedecl);
    if (slow(ret != OK)) {
        return ret;
    }

    return OK;
}


static u8
assertarray(Array *got, Array *want, const char *name, Assertion assertion)
{
    u32   i;
    void  *p1, *p2;

    if (got != want) {
        if (slow(got == NULL || want == NULL)) {
            return error("%s mismatch (%p != %p)", name, got, want);
        }

        if (slow(len(got) != len(want))) {
            return error("%s len mismatch (%d != %d)", name,
                         len(got), len(want));
        }

        for (i = 0; i < len(want); i++) {
            p1 = arrayget(got, i);
            p2 = arrayget(want, i);
            if (slow(assertion(p1, p2) != OK)) {
                return ERR;
            }
        }
    }

    return OK;
}


static u8
assertsect(const void *got, const void *want) {
    const Section  *s1, *s2;

    s1 = got;
    s2 = want;

    if (slow(s1->id != s2->id)) {
        return error("section id mismatch (%x != %x)", s1->id, s2->id);
    }

    if (slow(s1->len != s2->len)) {
        return error("section len mismatch (%d != %d)", s1->len, s2->len);
    }

    if (slow(memcmp(s1->data, s2->data, s1->len) != 0)) {
        return error("section data mismatch");
    }

    return OK;
}


static u8
asserttypedecl(const void *gotv, const void *wantv)
{
    u8              ret;
    const FuncDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->form != want->form)) {
        return error("funcdecl form mismatch (%x != %x)",
                     got->form, want->form);
    }

    ret = assertarray(got->params, want->params, "type params", asserttype);
    if (slow(ret != OK)) {
        return ERR;
    }

    return assertarray(got->rets, want->rets, "rets", asserttype);
}


static u8
asserttype(const void *got, const void *want)
{
    const Type  *p1, *p2;

    p1 = got;
    p2 = want;

    if (p1 == p2) {
        return OK;
    }

    if (slow((p1 == NULL || p2 == NULL) || (*p1 != *p2))) {
        return error("return mismatch (%d != %d)",
                     (p1 == NULL) ? 0 : *p1,
                     (p2 == NULL ? 0 : *p2));
    }

    return OK;
}


static u8
assertimportdecl(const void *gotv, const void *wantv)
{
    const ImportDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(!stringcmp(got->module, want->module))) {
        return error("import module string mismatch: (%S) != (%S)",
                     got->module, want->module);
    }

    if (slow(!stringcmp(got->field, want->field))) {
        return error("import field string mismatch: (%S) != (%S)",
                     got->field, want->field);
    }

    if (slow(got->kind != want->kind)) {
        return error("import kind mismatch (%d != %d)",
                     got->kind, want->kind);
    }

    switch (want->kind) {
    case Function:
        return asserttypedecl(&got->u.function, &want->u.function);
        break;
    default:
        return error("import kind data not implemented for %d", want->kind);
    }

    return OK;
}


static u8
asserttabledecl(const void *gotv, const void *wantv)
{
    const TableDecl *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->type != want->type)) {
        return error("table type mismatch (%d != %d)", got->type, want->type);
    }

    return assertresizablelimit(&got->limit, &want->limit);
}


static u8
assertresizablelimit(const void *gotv, const void *wantv)
{
    const ResizableLimit  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->flags != want->flags)) {
        return error("resizable limit flags mismatch (%d != %d)",
                     got->flags, want->flags);
    }

    if (slow(got->initial != want->initial)) {
        return error("resizable limit initial mismatch (%d != %d)",
                     got->initial, want->initial);
    }

    if (slow(got->maximum != want->maximum)) {
        return error("resizable limit maximum mismatch (%d != %d)",
                     got->maximum, want->maximum);
    }

    return OK;
}


static u8
assertmemorydecl(const void *gotv, const void *wantv)
{
    const MemoryDecl  *got, *want;

    got = gotv;
    want = wantv;

    return assertresizablelimit(&got->limit, &want->limit);
}


static u8
assertglobaldecl(const void *gotv, const void *wantv)
{
    const GlobalDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->type.type != want->type.type)) {
        return error("global type mismatch (%d != %d)",
                     got->type.type, want->type.type);
    }

    if (slow(got->type.mut != want->type.mut)) {
        return error("global mutability mismatch (%d != %d)",
                     got->type.mut, want->type.mut);
    }

    if (slow(got->u.f64val != want->u.f64val)) {
        return error("global init data mismatch (%u != %u)",
                     got->u.f64val != want->u.f64val);
    }

    return OK;
}

static u8
assertexportdecl(const void *gotv, const void *wantv)
{
    const ExportDecl  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(!stringcmp(got->field, want->field))) {
        return error("import field string mismatch: (%S) != (%S)",
                     got->field, want->field);
    }

    if (slow(got->kind != want->kind)) {
        return error("import kind mismatch (%d != %d)",
                     got->kind, want->kind);
    }

    if (slow(got->index != want->index)) {
        return error("import index mismatch (%d != %d)",
                     got->index, want->index);
    }

    return OK;
}


static u8
assertcodedecl(const void *gotv, const void *wantv)
{
    u8              ret;
    u32             gotsize, wantsize;
    const CodeDecl  *got, *want;

    got = gotv;
    want = wantv;

    ret = assertarray(got->locals, want->locals, "code locals", assertlocaldecl);
    if (slow(ret != OK)) {
        return ERR;
    }

    if (slow(got->start > got->end)) {
        return error("invalid starts and end pointers in code data");
    }

    wantsize = (want->end - want->start);
    gotsize  = (got->end - got->start);

    if (slow(wantsize != gotsize)) {
        return error("code section size mismatch (%d != %d)", gotsize, wantsize);
    }

    if (slow(memcmp(got->start, want->start, wantsize != 0))) {
        return error("code data mismatch");
    }

    return OK;
}


static u8
assertlocaldecl(const void *gotv, const void *wantv)
{
    const LocalEntry  *got, *want;

    got = gotv;
    want = wantv;

    if (slow(got->count != want->count)) {
        return error("code locals count mismatch (%d != %d)",
                     got->count, want->count);
    }

    return asserttype((const void *) got->type, (const void *) want->type);
}
