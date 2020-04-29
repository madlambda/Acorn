
/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"


void
print(String *s, ...)
{
    va_list  args;

    va_start(args, s);
    vprint(s, args);
    va_end(args);
}


void
cprint(const char *format, ...)
{
    String   *s;
    va_list  args;

    va_start(args, format);
    s = cvfmt(format, args);
    va_end(args);

    if (fast(s != NULL)) {
        print(s);
        free(s);
    }
}


void
vprint(String *format, va_list args)
{
    String  *res;

    res = vfmt(format, args);
    if (slow(res == NULL)) {
        return;
    }

    /*
     * We don't need to check the result here but there's no portable way
     * of doing that and still having all the warnings on.
     * The ugly thing below is the last resort.
     */
    (void)(write(1, res->start, res->len) + 1);

    free(res);
}
