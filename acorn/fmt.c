/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <acorn/string.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


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
    u8      *format, *end;
    char    *c;
    String  *buf, *s;

    buf = allocstring(1 + sfmt->len * 2);
    if (slow(buf == NULL)) {
        return NULL;
    }

    format = sfmt->start;
    end = offset(format, sfmt->len);

    while (format < end) {
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
                    return NULL;
                }
            }

            format++;
            continue;

        case 's':
            c = va_arg(args, char *);

            if (fast(c != NULL)) {
                buf = appendcstr(buf, c);
                if (slow(buf == NULL)) {
                    return NULL;
                }
            }

            format++;
            continue;

        default:
            c = "(invalid verb %";
            buf = appendcstr(buf, c);
            if (slow(buf == NULL)) {
                return NULL;
            }
            buf = appendc(buf, 2, (u8) *format, ')');
            if (slow(buf == NULL)) {
                return NULL;
            }

            format++;
            continue;
        }
    }

    return buf;
}