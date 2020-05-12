/*
 * Copyright (C) Madlambda Authors.
 */

#include <sys/types.h>

#include <acorn.h>
#include "bin.h"


/*
 * Encodes the unsigned integer `v` into LEB128 bytes in `begin` until `end`.
 * It returns the number of bytes written or -1 in case there's no enough space
 * in the buffer.
 */
ssize_t
uleb128encode(u64 v, u8 *begin, u8 *end)
{
    u8  *p;

    p = begin;

    if (slow(p >= end)) {
        return -1;
    }

    do {
        *p = (v & 0x7f);
        v >>= 7;

        if (v != 0) {
            *p |= 0x80;
        }

        p++;
    } while (v != 0 && (p < end));

    if (slow(v != 0 && p >= end)) {
        /* not enough space */
        return -1;
    }

    return (p - begin);
}


/*
 * Encodes the signed integer `v` into LEB128 bytes in `begin` until `end`.
 * It returns the number of bytes written or -1 in case there's no enough space
 * in the buffer.
 */
ssize_t
sleb128encode(i64 v, u8 *begin, u8 *end)
{
    u8  more, *p;

    more = 1;
    p = begin;

    if (slow(p >= end)) {
        return -1;
    }

    do {
        *p = (v & 0x7f);
        v >>= 7;

        if ((v == 0 && (*p & 0x40) == 0)
            || (v == -1 && (*p & 0x40) != 0))
        {
            more = 0;

        } else {
            *p |= 0x80;
        }

        p++;
    } while (more && p < end);

    if (slow(more)) {
        /* not enough space */
        return -1;
    }

    return (p - begin);
}


/*
 * Decode an unsigned integer from the LEB128 byte stream starting at begin and
 * ending at end.  The result is written at res and returns the number of bytes
 * read or -1 if the bytes are malformed.
 */
ssize_t
uleb128decode(const u8 *begin, const u8 *end, u64 *res)
{
    u8  shift, more, *p;

    *res = 0;
    shift = 0;

    p = (u8 *) begin;

    do {
        *res |= ((u64)((*p) & 0x7f) << shift);

        shift += 7;
        more = *p & 0x80;
        p++;
    } while(more && p < end && shift <= 64);

    if (slow(more)) {
        /* malformed ULEB128 byte stream */
        return -1;
    }

    return (p - begin);
}


u8
u8vdecode(u8 **begin, const u8 *end, u8 *val)
{
    u64      uval;
    ssize_t  read;

    read = uvdecode(*begin, end, &uval);
    if (slow(read != 1)) {
        return ERR;
    }

    *begin += 1;
    *val = (u8) (uval & 0xff);
    return OK;
}

u8
u32vdecode(u8 **begin, const u8 *end, u32 *val)
{
    u64      uval;
    ssize_t  read;

    read = uvdecode(*begin, end, &uval);
    if (slow(read <= 0 || read > 4)) {
        return ERR;
    }

    *begin += read;
    *val = (u32) uval;
    return OK;
}


/*
 * Decode a signed integer from the LEB128 byte stream starting at begin and
 * ending at end.  The result is written at res and returns the number of bytes
 * read or -1 if the bytes are malformed.
 */
ssize_t
sleb128decode(const u8 *begin, const u8 *end, i64 *res)
{
    u8  shift, *p;

    *res = 0;
    shift = 0;

    p = (u8 *) begin;

    do {
        *res |= ((u64)((*p) & 0x7f) << shift);
        shift += 7;
    } while ((*p++ & 0x80) != 0 && p <= end);

    --p;

    if ((shift < 64) && (*p & 0x40)) {
        /* shift an u64 bit pattern to avoid -Werror=shift-negative-value */
        *res |= (i64)(((u64)(-1) << shift));
    }

    return (p - begin) + 1;
}


u8
s8vdecode(u8 **begin, const u8 *end, i8 *val)
{
    i64      ival;
    ssize_t  read;

    read = svdecode(*begin, end, &ival);
    if (slow(read != 1)) {
        return ERR;
    }

    *begin += 1;
    *val = (i8) ival;
    return OK;
}


u8
s32vdecode(u8 **begin, const u8 *end, i32 *val)
{
    i64      ival;
    ssize_t  read;

    read = svdecode(*begin, end, &ival);
    if (slow(read <= 0 || read > 4)) {
        return ERR;
    }

    *begin += read;
    *val = (i32) ival;
    return OK;
}


u8
s64vdecode(u8 **begin, const u8 *end, i64 *val)
{
    ssize_t  read;

    read = svdecode(*begin, end, val);
    if (slow(read <= 0 || read > 8)) {
        return ERR;
    }

    *begin += read;
    return OK;
}


u8
u32decode(u8 **begin, const u8 *end, u32 *val)
{
    const u8  *data;
    if (slow((end - *begin) < 4)) {
        return ERR;
    }
    data = *begin;
    *val = (u32) data[0];
    *val |= ((u32) data[1]) << 8;
    *val |= ((u32) data[2]) << 16;
    *val |= ((u32) data[3]) << 24;

    *begin += 4;
    return OK;
}


u8
u8encode(u8 val, u8 **begin, const u8 *end)
{
    u8  *data;

    data = *begin;

    if (slow((end - data) < 1)) {
        return ERR;
    }

    *data++ = (u8) val;
    *begin = data;
    return OK;
}


u8
u16encode(u16 val, u8 **begin, const u8 *end)
{
    u8  *data;

    if (slow((end - *begin) < 2)) {
        return ERR;
    }

    data = *begin;

    data[0] = (val & 0xff);
    data[1] = (val >> 8) & 0xff;

    *begin += 2;
    return OK;
}


u8
u32encode(u32 val, u8 **begin, const u8 *end)
{
    u8  *data;

    if (slow((end - *begin) < 4)) {
        return ERR;
    }

    data = *begin;

    data[0] = (val & 0xff);
    data[1] = (val >> 8) & 0xff;
    data[2] = (val >> 16) & 0xff;
    data[3] = (val >> 24) & 0xff;

    *begin += 4;
    return OK;
}


u8
u64encode(u64 val, u8 **begin, const u8 *end)
{
    u8  *data;

    if (slow((end - *begin) < 8)) {
        return ERR;
    }

    data = *begin;

    data[0] = (val & 0xff);
    data[1] = (val >> 8) & 0xff;
    data[2] = (val >> 16) & 0xff;
    data[3] = (val >> 24) & 0xff;
    data[4] = (val >> 32) & 0xff;
    data[5] = (val >> 40) & 0xff;
    data[6] = (val >> 48) & 0xff;
    data[7] = (val >> 56) & 0xff;

    *begin += 8;
    return OK;
}


u8
uencode(u64 val, u8 bits, u8 **begin, const u8 *end)
{
    switch (bits) {
    case 8:
        return u8encode(val, begin, end);

    case 16:
        return u16encode(val, begin, end);

    case 32:
        return u32encode(val, begin, end);

    case 64:
        return u64encode(val, begin, end);
    }

    return ERR;
}
