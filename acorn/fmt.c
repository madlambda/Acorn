/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


/*
 * Formats supported:
 *
 *  %s      null-terminated string (C string)
 *  %S      String * (Acorn string)
 *  %d      base 10 integer
 *  %c      char
 */


/*
 * Default formatters
 */
static u8 wrongfmt(String **buf, u8 **format, void *val);
static u8 intfmt(String **buf, u8 **format, void *val);
static u8 charfmt(String **buf, u8 **format, void *val);
static u8 cstrfmt(String **buf, u8 **format, void *val);
static u8 stringfmt(String **buf, u8 **format, void *val);

static String *fmtintbuf(String *buf, i64 ival, int base);


static const char  Einvalidverb[] = "invalid verb %X";


#define MAXFMT ('z' - 'A' + 1)


static Formatter  formatters[MAXFMT] = {
    /* 'A' */
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    stringfmt,  /* S */
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,   /* [ */
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,   /* ` */

    wrongfmt,   /* a */
    wrongfmt,
    charfmt,    /* c */
    intfmt,     /* d */
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    cstrfmt,    /* s */
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,
    wrongfmt,   /* z */
};


static Printer  defaultprinter = {
    .formatters = formatters,
};


/*
 * newprinter creates a custom printer.
 * The function pfmtadd() can be used to extend or update the builtin
 * formatters.
 */
Printer *
newprinter()
{
    Printer  *printer;

    printer = malloc(sizeof(Printer) + sizeof(Formatter) * MAXFMT);
    if (slow(printer == NULL)) {
        return NULL;
    }

    printer->formatters = offset(printer, sizeof(Printer));
    memcpy(printer->formatters, formatters, sizeof(Formatter) * MAXFMT);

    return printer;
}


/*
 * Format using a custom printer.
 */
String *
pfmt(Printer *printer, String *format, ...)
{
    String   *buf;
    va_list  args;

    va_start(args, format);
    buf = vpfmt(printer, format, args);
    va_end(args);

    return buf;
}


String *
vpfmt(Printer *printer, String *format, va_list args)
{
    String  *buf;

    buf = allocstring(1 + format->len * 2);
    if (slow(buf == NULL)) {
        return NULL;
    }

    return vpfmtbuf(printer, buf, format, args);
}


String *
pcfmt(Printer *printer, const char *format, ...)
{
    String   *sfmt, *s;
    va_list  args;

    sfmt = newstring((u8 *) format, strlen(format));
    if (slow(sfmt == NULL)) {
        return NULL;
    }

    va_start(args, format);
    s = vpfmt(printer, sfmt, args);
    va_end(args);

    free(sfmt);

    return s;
}


/*
 * Format onto an allocated buffer.
 */
String *
fmtbuf(String *buf, String *format, ...)
{
    String   *s;
    va_list  args;

    va_start(args, format);
    s = vpfmtbuf(&defaultprinter, buf, format, args);
    va_end(args);

    return s;
}


/*
 * Format the string into a newly allocated buffer.
 * Users are responsible to free the returned buffer.
 */
String *
fmt(String *format, ...)
{
    String   *s;
    va_list  args;

    va_start(args, format);
    s = vfmt(format, args);
    va_end(args);

    return s;
}


String *
cfmtbuf(String *buf, const char *format, ...)
{
    String   *sfmt, *s;
    va_list  args;

    sfmt = newstring((u8 *) format, strlen(format));
    if (slow(sfmt == NULL)) {
        return NULL;
    }

    va_start(args, format);
    s = vpfmtbuf(&defaultprinter, buf, sfmt, args);
    va_end(args);

    free(sfmt);

    return s;
}


/*
 * Format the null-terminated string into a newly allocated buffer.
 * Users are responsible to free the returned buffer.
 */
String *
cfmt(const char *format, ...)
{
    String   *sfmt, *s;
    va_list  args;

    sfmt = newstring((u8 *) format, strlen(format));
    if (slow(sfmt == NULL)) {
        return NULL;
    }

    va_start(args, format);
    s = vfmt(sfmt, args);
    va_end(args);

    free(sfmt);

    return s;
}


String *
cvfmt(const char *format, va_list args)
{
    String  *sfmt, *s;

    sfmt = newstring((u8 *) format, strlen(format));
    if (slow(sfmt == NULL)) {
        return NULL;
    }

    s = vfmt(sfmt, args);
    free(sfmt);
    return s;
}


String *
vfmt(String *sfmt, va_list args)
{
    String  *buf;

    buf = allocstring(1 + sfmt->len * 2);
    if (slow(buf == NULL)) {
        return NULL;
    }

    return vpfmtbuf(&defaultprinter, buf, sfmt, args);
}


String *
vpfmtbuf(Printer *printer, String *buf, String *sfmt, va_list args)
{
    u8      *format, *end, ret;
    u8      index;
    void    *p;

    format = sfmt->start;
    end = offset(format, sfmt->len);

    while (buf != NULL && format < end) {
        if (*format != '%') {
            buf = appendc(buf, 1, *format++);
            continue;
        }

        format++;

        index = *format - 'A';

        if (slow(index >= MAXFMT)) {
            if (slow(wrongfmt(&buf, &format, NULL) != OK)) {
                return NULL;
            }

            format++;
            continue;
        }

        p = va_arg(args, void *);

        ret = printer->formatters[index](&buf, &format, p);
        if (slow(ret != OK)) {
            return NULL;
        }

        format++;
        continue;
    }

    return buf;
}


void
pfmtadd(Printer *printer, u8 flag, Formatter formatter)
{
   printer->formatters[flag - 'A'] = formatter;
}


void
fmtadd(u8 flag, Formatter formatter)
{
    pfmtadd(&defaultprinter, flag, formatter);
}


#define MAXI64 "-9223372036854775808"


String *
fmtint(i64 ival, int base)
{
    String  *buf;

    buf = allocstring(slength(MAXI64));
    if (slow(buf == NULL)) {
        return NULL;
    }

    return fmtintbuf(buf, ival, base);
}


static String *
fmtintbuf(String *buf, i64 ival, int base)
{
    u8      neg, *p, *start, *end;
    u64     uval;
    size_t  maxlen, len;
    u8      tmp[slength(MAXI64)];

    if (slow(base != 10)) {
        /* TODO(i4k) */
        return NULL;
    }

    maxlen = slength(MAXI64);

    if (slow((strnalloc(buf) - len(buf)) < maxlen)) {
        return NULL;
    }

    if (ival < 0) {
        neg = 1;
        uval = -ival;
    } else {
        neg = 0;
        uval = ival;
    }

    p = offset(tmp, maxlen);

    do {
        *(--p) = (u8) (uval % base + '0');
        uval /= base;
    } while (uval != 0);

    if (neg > 0) {
        buf->start[buf->len] = '-';
        buf->len++;
    }

    len = (tmp + maxlen) - p;
    start = (buf->start + buf->len);
    end = start + len;

    while (start < end) {
        *start++ = *p++;
    }

    buf->len += len;
    return buf;
}


static u8
wrongfmt(String **buf, u8 **format, void * unused(val))
{
    char  verberr[sizeof(Einvalidverb)];

    memcpy(verberr, Einvalidverb, sizeof(Einvalidverb));

    verberr[slength(verberr) - 1] = **format;

    *buf = appendc(*buf, 1, '(');
    *buf = appendcstr(*buf, verberr);
    if (slow(*buf == NULL)) {
        return ERR;
    }

    *buf = appendc(*buf, 1, ')');
    if (slow(*buf == NULL)) {
        return ERR;
    }
    return OK;
}


static u8
intfmt(String **buf, u8 ** unused(format), void *val)
{
    String  sintbuf;
    u8      intbuf[slength(MAXI64)];

    sintbuf.len = 0;
    sintbuf.nalloc = sizeof(intbuf);
    sintbuf.start = intbuf;

    if (slow(fmtintbuf(&sintbuf, (i64) val, 10) == NULL)) {
        return ERR;
    }
    *buf = append(*buf, &sintbuf);
    if (slow(buf == NULL)) {
        return ERR;
    }

    return OK;
}


static u8
charfmt(String **buf, u8 ** unused(format), void *val)
{
    ptr   p;
    char  c;

    p = (ptr) val;

    c = (char) p;

    *buf = appendc(*buf, 1, c);
    if (slow(*buf == NULL)) {
        return ERR;
    }

    return OK;
}


static u8
stringfmt(String **buf, u8 ** unused(format), void *val)
{
    *buf = append(*buf, (String *) val);
    if (slow(*buf == NULL)) {
        return ERR;
    }
    return OK;
}


static u8
cstrfmt(String **buf, u8 ** unused(format), void *val)
{
    *buf = appendcstr(*buf, (const char *) val);
    if (slow(*buf == NULL)) {
        return ERR;
    }
    return OK;
}
