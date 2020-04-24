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
        cprint("malloc(%d): %e", size, strerror(errno));
        exit(1);
    }

    return ptr;
}
