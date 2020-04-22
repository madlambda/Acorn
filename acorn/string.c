/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "string.h"


static String *grow(String *s);
static String *vappendc(String *s, u32 nargs, va_list args);


String *
newstring(const u8 *from, size_t size)
{
    size_t  allocsize;
    String  *s;

    if (size < 80) {
        allocsize = 1 + size * 2;
    } else {
        allocsize = 1 + size + size/2;
    }

    s = allocstring(allocsize);
    if (fast(s != NULL)) {
        memcpy(s->start, from, size);
        s->len = size;
    }
    return s;
}


String *
allocstring(size_t size)
{
    String  *s;

    s = malloc(sizeof(String) + size);
    if (slow(s == NULL)) {
        return NULL;
    }

    if (size > 0) {
        s->start = offset(s, sizeof(String));
    }

    s->len = 0;
    s->nalloc = size;

    return s;
}


static String *
vappendc(String *s, u32 nargs, va_list args)
{
    u8   c;
    u32  i;

    for (i = 0; i < nargs; i++) {
        if (s->len == s->nalloc) {
            s = grow(s);
            if (slow(s == NULL)) {
                return NULL;
            }
        }

        c = va_arg(args, int);
        s->start[s->len] = (u8) (c & 0xff);
        s->len++;
    }

    return s;
}


String *
appendc(String *s, u32 nargs, ...)
{
    va_list  args;

    va_start(args, nargs);
    s = vappendc(s, nargs, args);
    va_end(args);

    return s;
}


String *
append(String *s, const String *other)
{
    u32  i;

    for (i = 0; i < other->len; i++) {
        s = appendc(s, 1, other->start[i]);
        if (slow(s == NULL)) {
            return NULL;
        }
    }

    return s;
}


String *
appendcstr(String *s, const char *str)
{
    while (*str != '\0') {
        s = appendc(s, 1, *str++);
        if (slow(s == NULL)) {
            return NULL;
        }
    }

    return s;
}


static String *
grow(String *s)
{
    size_t  newsize;
    String  *new;

    newsize = 1 + s->nalloc + s->nalloc/2;

    new = allocstring(newsize);
    if (slow(new == NULL)) {
        return NULL;
    }

    new->len = s->len;
    new->nalloc = newsize;

    memcpy(new->start, s->start, s->len);
    free(s);
    return new;
}
