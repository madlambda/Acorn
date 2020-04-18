/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _ACORN_H_
#define _ACORN_H_

#include <stddef.h>
#include <stdarg.h>
#include <assert.h>


#define DEBUG 1

#define OK 0
#define ERR 1


#define slow(expr)                                                            \
    __builtin_expect((long) (expr), 0)


#define fast(expr)                                                            \
    __builtin_expect((long) (expr), 1)


#define slength(val)                                                          \
    (sizeof(val) - 1)


#if (DEBUG)
#define expect(expr)                                                          \
    assert(expr)
#else
#define expect(expr)
#endif


void *copy(void *dst, const void *src, size_t n);
char *vsprint(char *start, char *end, const char *fmt, va_list vl);
char *sprint(char *start, char *end, const char *fmt, ...);


#endif /* _ACORN_H_ */