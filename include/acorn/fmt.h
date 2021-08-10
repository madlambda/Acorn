/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _ACORN_FMT_H_
#define _ACORN_FMT_H_


typedef u8 (*Formatter)(String **buf, u8 **fmt, void *val);


typedef struct {
    Formatter  *formatters;
} Printer;


Printer     *newprinter();
String      *pfmt(Printer *, String *format, ...);
String      *pcfmt(Printer *, const char *format, ...);
String      *vpfmt(Printer *, String *format, va_list args);
String      *vpfmtbuf(Printer *, String *buf, String *format, va_list args);

void        pfmtadd(Printer *, u8 flag, Formatter f);
void        fmtadd(u8 flag, Formatter f);

String      *fmt(String *format, ...);
String      *vfmt(String *format, va_list args);
String      *cfmt(const char *format, ...);
String      *cvfmt(const char *format, va_list args);
String      *fmtbuf(String *buf, String *format, ...);
String      *vfmtbuf(String *buf, String *format, va_list args);
String      *cfmtbuf(String *buf, const char *format, ...);

u8 hexfmt(String **buf, u8 **format, void *val);

String      *fmtint(i64 ival, int base);


#endif /* _ACORN_FMT_H_ */
