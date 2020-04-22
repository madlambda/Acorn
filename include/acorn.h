/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _ACORN_H_
#define _ACORN_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>

#include <acorn/types.h>


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


void    *copy(void *dst, const void *src, size_t n);


#include <acorn/string.h>
#include <acorn/fmt.h>


void    print(String *s, ...);
void    vprint(String *s, va_list args);

#endif /* _ACORN_H_ */