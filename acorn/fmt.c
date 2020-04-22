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
 */


#define MAXI64 "-9223372036854775808"


static String *fmtintbuf(String *buf, i64 ival, int base);


static const char  Eappend[] = "failed to append string";
static const char  Einvalidverb[] = "invalid verb %X";


String *
fmtbuf(String *buf, String *format, ...)
{
    String   *s;
    va_list  args;

    va_start(args, format);
    s = vfmtbuf(buf, format, args);
    va_end(args);

    return s;
}


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
    s = vfmtbuf(buf, sfmt, args);
    va_end(args);

    free(sfmt);

    return s;
}


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

    return vfmtbuf(buf, sfmt, args);
}


String *
vfmtbuf(String *buf, String *sfmt, va_list args)
{
    u8      *format, *end;
    i64     ival;
    char    *c, *err;
    String  *s;
    String  sintbuf;
    u8      intbuf[slength(MAXI64)];
    char    verberr[sizeof(Einvalidverb)];

    sintbuf.len = 0;
    sintbuf.nalloc = sizeof(intbuf);
    sintbuf.start = intbuf;

    err = NULL;

    memcpy(verberr, Einvalidverb, sizeof(Einvalidverb));

    format = sfmt->start;
    end = offset(format, sfmt->len);

    while (buf != NULL && format < end) {
        if (*format != '%') {
            buf = appendc(buf, 1, *format++);
            continue;
        }

        format++;

        switch (*format) {
        case 'S':
            s = va_arg(args, String *);

            if (fast(s != NULL)) {
                buf = append(buf, s);
                if (slow(buf == NULL)) {
                    err = (char *) Eappend;
                    goto fmtfail;
                }
            }

            format++;
            continue;

        case 's':
            c = va_arg(args, char *);

            if (fast(c != NULL)) {
                buf = appendcstr(buf, c);
                if (slow(buf == NULL)) {
                    err = (char *) Eappend;
                    goto fmtfail;
                }
            }

            format++;
            continue;

        case 'd':
            ival = va_arg(args, i64);

            if (slow(fmtintbuf(&sintbuf, ival, 10) == NULL)) {
                err = "failed to convert integer to string";
                goto fmtfail;
            }
            buf = append(buf, &sintbuf);
            if (slow(buf == NULL)) {
                err = (char *) Eappend;
                goto fmtfail;
            }

            memset(intbuf, 0, sizeof(intbuf));
            sintbuf.len = 0;

            format++;
            continue;

        default:

            err = verberr;
            err[slength(verberr) - 1] = *format;

fmtfail:

            expect(err != NULL);

            /* if it was a memory error, it will probably fail again */

            buf = appendc(buf, 1, '(');
            buf = appendcstr(buf, err);
            if (slow(buf == NULL)) {
                return NULL;
            }
            buf = appendc(buf, 1, ')');
            if (slow(buf == NULL)) {
                return NULL;
            }

            format++;
            continue;
        }
    }

    return buf;
}


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
