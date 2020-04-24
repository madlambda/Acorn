/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _OAK_ERROR_H_
#define _OAK_ERROR_H_


typedef struct Error {
    const u8      internal;     /* internal errors are non-freeable */
    u8            heap;
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


#endif /* _OAK_ERROR_H_ */
