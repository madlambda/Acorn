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


static u8 test_module(const Testcase *tc);
static u8 assertmodule(const Module *m1, const Module *m2);
static u8 assertsect(Section *s1, Section *s2);
static u8 asserttypedecl(FuncDecl *f1, FuncDecl *f2);


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
    u16       i;
    Section   *sect1, *sect2;
    FuncDecl  *type1, *type2;

    if (m1 == m2) {
        return OK;
    }

    if (slow(m1 == NULL || m2 == NULL)) {
        return error("module mismatch (%p != %p)", m1, m2);
    }

    if (slow(m1->version != m2->version)) {
        return error("version mismatch (%d != %d)", m1->version, m2->version);
    }

    if (m1->sects != m2->sects) {
        if (slow(m1->sects == NULL || m2->sects == NULL)) {
            return error("sect mismatch (%p != %p)", m1->sects, m2->sects);
        }

        if (slow(len(m1->sects) != len(m2->sects))) {
            return error("len(sects) mismatch (%d != %d)",
                        len(m1->sects), len(m2->sects));
        }

        for (i = 0; i < len(m1->sects); i++) {
            sect1 = arrayget(m1->sects, i);
            sect2 = arrayget(m2->sects, i);
            if (slow(assertsect(sect1, sect2) != OK)) {
                return ERR;
            }
        }
    }

    if (m1->types != m2->types) {
        if (slow(m1->types == NULL || m2->types == NULL)) {
            return error("types mismatch (%p != %p)", m1->types, m2->types);
        }

        if (slow(len(m1->types) != len(m2->types))) {
            return error("types len mismatch (%d != %d)",
                         len(m1->types), len(m2->types));
        }

        for (i = 0; i < len(m1->types); i++) {
            type1 = arrayget(m1->types, i);
            type2 = arrayget(m2->types, i);
            if (slow(asserttypedecl(type1, type2) != OK)) {
                return ERR;
            }
        }
    }

    return OK;
}


static u8
assertsect(Section *s1, Section *s2) {
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
asserttypedecl(FuncDecl *f1, FuncDecl *f2)
{
    u32   i;
    Type  *p1, *p2;

    if (slow(f1->form != f2->form)) {
        return error("funcdecl form mismatch (%x != %x)", f1->form, f2->form);
    }

    if (f1->params != f2->params) {
        if (slow(f1->params == NULL || f2->params == NULL)) {
            return error("funcdecl params mismatch (%p != %p)",
                         f1->params, f2->params);
        }

        if (slow(len(f1->params) != len(f2->params))) {
            return error("funcdecl param count mismatch (%u != %u)",
                         len(f1->params), len(f2->params));
        }

        for (i = 0; i < len(f1->params); i++) {
            p1 = arrayget(f1->params, i);
            p2 = arrayget(f2->params, i);

            if (slow((p1 == NULL || p2 == NULL) || (*p1 != *p2))) {
                return error("param mismatch (%x != %x)",
                             (p1 == NULL) ? 0 : *p1,
                             (p2 == NULL ? 0 : *p2));
            }
        }
    }

    if (f1->rets != f2->rets) {
        if (slow(f1->rets == NULL || f2->rets == NULL)) {
            return error("funcdecl rets mismatch (%p != %p)",
                         f1->rets, f2->rets);
        }

        if (slow(len(f1->rets) != len(f2->rets))) {
            return error("funcdecl return count mismatch (%u != %u)",
                         len(f1->rets), len(f2->rets));
        }

        for (i = 0; i < len(f1->rets); i++) {
            p1 = arrayget(f1->rets, i);
            p2 = arrayget(f2->rets, i);

            if (slow((p1 == NULL || p2 == NULL) || (*p1 != *p2))) {
                return error("return mismatch (%d != %d)",
                             (p1 == NULL) ? 0 : *p1,
                             (p2 == NULL ? 0 : *p2));
            }
        }
    }

    return OK;
}
