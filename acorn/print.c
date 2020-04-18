
/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdarg.h>
#include <stdio.h>


char *
sprint(char *start, char *end, const char *fmt, ...)
{
    int      n;
    va_list  vl;

    va_start(vl, fmt);
    n = vsnprintf(start, end - start, fmt, vl);
    va_end(vl);

    return start + n;
}


char *
vsprint(char *start, char *end, const char *fmt, va_list vl)
{
    return start + vsnprintf(start, end - start, fmt, vl);
}
