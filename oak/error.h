/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _OAK_ERROR_H_
#define _OAK_ERROR_H_


typedef struct {
    u8     heap;    /* tell if heap allocated */
    Array  *stack;
} Error;


Error   *newerror(const char *format, ...);
void    errorinit(Error *err);
void    errorfree(Error *err)
u8      error(Error *err, const char *format, ...);
u8      iserror(Error *err, const char *errmsg);
void    eprint(Error *err);


#endif /* _OAK_ERROR_H_ */
