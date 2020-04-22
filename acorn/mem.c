/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdlib.h>
#include <string.h>


void *
copy(void *dst, const void *src, size_t n)
{
    void  *p;

    p = memcpy(dst, src, n);

    return p + n;
}


void *
zmalloc(size_t size)
{
    void  *ptr;

    ptr = malloc(size);
    if (slow(ptr == NULL)) {
        return NULL;
    }

    memset(ptr, 0, size);

    return ptr;
}
