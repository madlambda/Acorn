/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <string.h>


void *
copy(void *dst, const void *src, size_t n)
{
    void  *p;

    p = memcpy(dst, src, n);

    return p + n;
}
