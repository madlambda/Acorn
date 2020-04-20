#include <acorn.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "module.h"
#include "test.h"


typedef struct {
    const char  *filename;
    const char  *err;
    Module      module;
} Testcase;


u8 test_module(const Testcase *tc);
u8 assertmodule(const Module *m1, const Module *m2);
u8 assertsect(Section *s1, Section *s2);


static const u8 call1typedata[] = {
    0x2, 0x60, 0x1, 0x7f, 0x0, 0x60, 0x0, 0x0
};

static const u8 call1impodata[] = {
    0x01, 0x07, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74,
    0x73, 0x0d, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74,
    0x65, 0x64, 0x5f, 0x66, 0x75, 0x6e, 0x63, 0x00,
    0x00
};

static const u8 call1funcdata[] = {0x01, 0x01};

static const u8 call1expodata[] = {
    0x01, 0x0d, 0x65, 0x78, 0x70, 0x6f, 0x72, 0x74,
    0x65, 0x64, 0x5f, 0x66, 0x75, 0x6e, 0x63, 0x00,
    0x01,
};

static const u8 call1codedata[] = {
    0x01, 0x06, 0x00, 0x41, 0x2a, 0x10, 0x00, 0x0b,
};

static Section call1sects[] = {
    {1, 0x8, call1typedata},
    {2, 0x19, call1impodata},
    {3, 2, call1funcdata},
    {7, 0x11, call1expodata},
    {10, 8, call1codedata},
};


static const Testcase  invalid_cases[] = {
    {
        "testdata/invalid/notwasm",
        "WASM must have at least 8 bytes",
        {NULL, 0, NULL, 0},
    },
    {
        "testdata/ok/empty.wasm",
        NULL,
        {NULL, 11, NULL, 0},
    },
    {
        "testdata/ok/call1.wasm",
        NULL,
        {NULL, 1, call1sects, 5},
    }
};


#define nitems(a) (sizeof(a)/sizeof(a[0]))


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


u8
test_module(const Testcase *tc)
{
    u8      ret;
    Error   err;
    Module  *m;

    err.msg = NULL;

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

    ret = assertmodule(m, &tc->module);
    closemodule(m);
    return ret;
}


u8
assertmodule(const Module *m1, const Module *m2)
{
    u16  i;

    if (m1 == m2) {
        return OK;
    }

    if (slow(m1 == NULL || m2 == NULL)) {
        return error("module mismatch (%p != %p)", m1, m2);
    }

    if (slow(m1->version != m2->version)) {
        return error("version mismatch (%d != %d)", m1->version, m2->version);
    }

    if (slow(m1->nsect != m2->nsect)) {
        return error("nsections mismatch (%d != %d)", m1->nsect, m2->nsect);
    }

    if (m1->sect == m2->sect) {
        return OK;
    }

    for (i = 0; i < m1->nsect; i++) {
        if (slow(assertsect(&m1->sect[i], &m2->sect[i]) != OK)) {
            return ERR;
        }
    }

    return OK;
}


u8
assertsect(Section *s1, Section *s2) {
    if (slow(s1->id != s2->id)) {
        return error("section id mismatch (%x != %x)", s1->id, s2->id);
    }

    if (slow(s1->len != s2->len)) {
        return error("section len mismatch (%x != %x)", s1->len, s2->len);
    }

    if (slow(memcmp(s1->data, s2->data, s1->len) != 0)) {
        return error("section data mismatch");
    }

    return OK;
}
