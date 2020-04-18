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



void *
test_alloc(size_t size)
{
    void  *ptr;

    ptr = malloc(size);
    if (slow(ptr == NULL)) {
        test_error("malloc(%ld): %s", size, strerror(errno));
        exit(1);
    }

    return ptr;
}


void
test_error(const char *fmt, ...)
{
    char     *p, *end;
    va_list  vl;
    char     err[OAK_MAX_ERR_MSG];

    static const char  prefix[] = "[error] ";

    end = err + OAK_MAX_ERR_MSG - 2;

    p = copy(err, prefix, slength(prefix));

    va_start(vl, fmt);
    p = vsprint(p, end, fmt, vl);
    va_end(vl);

    *p++ = '\n';
    *p = '\0';

    printf("%s", err);
}
