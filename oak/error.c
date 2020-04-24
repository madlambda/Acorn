/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "error.h"


Error *
newerror(const char *format, ...)
{
    Error    *err;
    String   *msg;
    va_list  args;

    va_start(args, format);
    msg = cvfmt(format, args);
    va_end(args);

    if (slow(msg == NULL)) {
        return NULL;
    }

    err = zmalloc(sizeof(Error));
    if (slow(err == NULL)) {
        return NULL;
    }

    err->heap = 1;

    err->stack = newarray(5, sizeof(ptr));
    if (slow(err == NULL)) {
        return NULL;
    }

    if (slow(arrayadd(err->stack, &msg) != OK)) {
        free(msg);
        return NULL;
    }

    return err;
}


void
errorinit(Error *err)
{
    err->stack = NULL;
    err->heap = 0;
}

void
errorfree(Error *err)
{
    u32  i;

    if (err->stack) {
        for (i = 0; i < len(err->stack); i++) {
            free(arrayget(err->stack, i));
        }

        freearray(err->stack);
    }

    if (err->heap) {
        free(err);
    }
}


u8
error(Error *err, const char *format, ...)
{
    String   *msg;
    va_list  args;

    if (err->stack == NULL) {
        err->stack = newarray(5, sizeof(ptr));
        if (slow(err->stack == NULL)) {
            cprint("failed to allocate error: %E", errno);
            return ERR;
        }
    }

    va_start(args, format);
    msg = cvfmt(format, args);
    va_end(args);

    if (slow(msg == NULL)) {
        cprint("failed to format error: %E", errno);
        return ERR;
    }

    arrayadd(err->stack, msg);
    return ERR;
}


u8
iserror(Error *err, const char *errmsg)
{
    u32     i;
    String  *m;

    for (i = 0; i < len(err->stack); i++) {
        m = arrayget(err->stack, i);
        if (fast(m != NULL)) {
            if (cstringcmp(m, errmsg)) {
                return 1;
            }
        }
    }

    return 0;
}

void
eprint(Error *err)
{
    i64     i;
    String  *s;

    for (i = len(err->stack) - 1; i != 0; i--) {
        s = arrayget(err->stack, i);
        if (fast(s != NULL)) {
            print(s);

            free(s);

            if (i > 0) {
                cprint(": ");
            }
        }
    }

    freearray(err->stack);

    err->stack = NULL;

    cprint("\n");
}
