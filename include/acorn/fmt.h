/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _OAK_FMT_H_
#define _OAK_FMT_H_


String  *fmt(String *format, ...);
String  *vfmt(String *format, va_list args);
String  *cfmt(const char *format, ...);
String  *cvfmt(const char *format, va_list args);


#endif /* _OAK_FMT_H_ */