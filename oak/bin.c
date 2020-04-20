/*
 * Copyright (C) Madlambda Authors.
 */


#include <acorn.h>
#include <sys/types.h>
#include <inttypes.h>

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

    expect(p <= end);

    if (slow(more)) {
        /* malformed ULEB128 byte stream */
        return -1;
    }

    return (p - begin);
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


void
u32decode(const u8 *data, u32 *val)
{
    *val = (u32) data[0];
    *val |= ((u32) data[1]) << 8;
    *val |= ((u32) data[2]) << 16;
    *val |= ((u32) data[3]) << 24;
}
