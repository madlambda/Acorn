/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdlib.h>
#include <string.h>


void *
copy(void *dst, const void *src, size_t n)
{
    ptr  p;

    p = (ptr) memcpy(dst, src, n);

    return (void *) (p + n);
}


void
copyptr(ptr dst, const ptr src)
{
    /* GCC erroneusly forbids cast of data to func pointers with -pedantic. */
    memcpy((void *) dst, (void *) src, sizeof(ptr));
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
