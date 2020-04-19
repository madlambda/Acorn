#include <acorn.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "module.h"


typedef struct {
    const char  *filename;
    Error       err;
    Module      module;
} Testcase;


u8 testmodule(const Testcase *tc);


static const Testcase  invalid_cases[] = {
    {
        "testdata/invalid/notwasm",
        {"WASM must have at least 8 bytes", NULL},
        {NULL, 0, NULL, 0},
    },
    {
        "testdata/ok/empty",
        {NULL, NULL},
        {NULL, 1, NULL, 0},
    }
};


#define nitems(a) (sizeof(a)/sizeof(a[0]))


int main()
{
    u32  i;

    for (i = 0; i < nitems(invalid_cases); i++) {
        if (slow(testmodule(&invalid_cases[i]) != OK)) {
            return 1;
        }
    }

    return 0;
}


u8
testmodule(const Testcase *tc)
{
    Error           err;
    Module          *m;

    err.msg = NULL;

    m = loadmodule(tc->filename, &err);
    if (m == NULL) {
        if (strcmp(err.msg, tc->err.msg) != 0) {
            printf("error mismatch: \"%s\" != \"%s\"\n", err.msg, tc->err.msg);
            return ERR;
        }
    } else {
        closemodule(m);
    }

    return OK;
}
