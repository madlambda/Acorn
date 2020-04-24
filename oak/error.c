/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "error.h"


static Error *vnewerror(const char *format, va_list args);


static String  internalerror = {
    .start   = (u8 *) "internal error",
    .len     = 14,
};


static Error  Einternal = {
    .msg        = &internalerror,
    .internal   = 1,
};


Error *
newerror(const char *format, ...)
{
    Error    *err;
    va_list  args;

    va_start(args, format);
    err = vnewerror(format, args);
    va_end(args);

    return err;
}


static Error *
vnewerror(const char *format, va_list args)
{
    Error   *err;
    String  *msg;

    msg = cvfmt(format, args);
    if (slow(msg == NULL)) {
        return NULL;
    }

    err = zmalloc(sizeof(Error));
    if (slow(err == NULL)) {
        return NULL;
    }

    err->heap = 1;
    err->msg = msg;
    return err;
}


/*
 * Initialize an stack-allocated error object.
 */
void
errorinit(Error *err)
{
    err->cause = NULL;
    err->heap = 0;
}


/*
 * Free an error object. In case of stack allocated error objects, it frees its
 * internal data.
 */
void
errorfree(Error *err)
{
    if (err->internal) {
        /* internal errors are statically allocated */
        return;
    }

    if (err->msg) {
        free(err->msg);
    }

    if (err->cause) {
        errorfree(err->cause);
    }

    if (err->heap) {
        free(err);
    }
}


/*
 * Format the error and store it on the error object.
 * It always returns ERR for convenience use.
 *
 *   if (somethingiswrong()) {
 *       return error(err, "failed to do something");
 *   }
 */
Error *
error(Error *err, const char *format, ...)
{
    Error    *new;
    va_list  args;

    va_start(args, format);
    new = vnewerror(format, args);
    va_end(args);

    if (slow(new == NULL)) {
        /* return old error, if any */
        return &Einternal;
    }

    if (err != NULL && err->msg != NULL) {
        new->cause = err;
    }

    return new;
}


/*
 * iserror() checks if errmsg is on the stack of errors.
 */
u8
iserror(Error *err, const char *errmsg)
{
    while (err != NULL) {
        if (err->msg == NULL) {
            break;
        }

        if (cstringcmp(err->msg, errmsg)) {
            return 1;
        }

        err = err->cause;
    }

    return 0;
}


u8
errorfmt(String **buf, u8 ** unused(format), void *val)
{
    Error  *err;

    err = (Error *) val;

    if (err == NULL || err->msg == NULL) {
        *buf = appendcstr(*buf, "(no error)");
        return OK;
    }

    while (err != NULL) {
        *buf = append(*buf, err->msg);
        if (slow(*buf == NULL)) {
            return ERR;
        }

        if (err->cause != NULL) {
            *buf = appendcstr(*buf, ": ");
            if (slow(*buf == NULL)) {
                return ERR;
            }
        }

        err = err->cause;
    }

    return OK;
}
