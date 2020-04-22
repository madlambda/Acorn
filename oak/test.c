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
    String   *err;
    va_list  args;

    va_start(args, fmt);
    err = cvfmt("[error] %s", args);
    va_end(args);

    if (slow(err == NULL)) {
        return ERR;
    }

    print(err);
    free(err);
    return ERR;
}
