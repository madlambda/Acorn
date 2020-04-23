/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "test.h"


/*
 * These functions must be used in tests.
 */


void *
mustalloc(size_t size)
{
    void  *ptr;

    ptr = malloc(size);
    if (slow(ptr == NULL)) {
        error("malloc(%ld): %s", size, strerror(errno));
        exit(1);
    }

    return ptr;
}


u8
error(const char *fmt, ...)
{
    String   *err, *res;
    va_list  args;

    va_start(args, fmt);
    err = cvfmt(fmt, args);
    va_end(args);

    if (slow(err == NULL)) {
        return ERR;
    }

    res = newcstring("[error] ");
    if (slow(res == NULL)) {
        goto fail;
    }

    res = append(res, err);
    if (slow(res == NULL)) {
        goto fail;
    }

    res = appendc(res, 1, '\n');
    if (slow(res == NULL)) {
        goto fail;
    }

    print(res);

    free(err);
    free(res);

    return ERR;

fail:

    free(err);
    return ERR;
}
