/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _ACORN_ERROR_H_
#define _ACORN_ERROR_H_


typedef struct Error {
    const u8      internal;     /* internal errors are non-freeable */
    String        *msg;
    struct Error  *cause;
} Error;


Error   *newerror(const char *format, ...);
void    errorinit(Error *);
void    errorfree(Error *);
Error   *error(Error *, const char *format, ...);
u8      iserror(Error *, const char *errmsg);

/* custom formatter */
u8      errorfmt(String **buf, u8 **format, void *val);


#endif /* _ACORN_ERROR_H_ */
