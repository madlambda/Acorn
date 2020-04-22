
/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdarg.h>
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
vprint(String *s, va_list args)
{
    String  *res;

    res = vfmt(s, args);
    if (slow(res == NULL)) {
        return;
    }

    (void) write(1, res->start, res->len);
}
