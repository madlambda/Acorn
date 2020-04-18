/*
 * Copyright (C) Madlambda Authors.
 */


#include <acorn.h>
#include <oak_bin.h>


/*
 * Encodes the unsigned integer `v` into LEB128 bytes in `begin` until `end`.
 * It returns the number of bytes written or -1 in case there's no enough space
 * in the buffer.
 */
ssize_t
oak_uleb128_encode(uint64_t v, uint8_t *begin, uint8_t *end) {
    uint8_t  *p;

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
oak_sleb128_encode(int64_t v, uint8_t *begin, uint8_t *end)
{
    uint8_t  more, *p;

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
oak_bin_uleb128_decode(const uint8_t *begin, const uint8_t *end, uint64_t *res)
{
    uint8_t  shift, more, *p;

    *res = 0;
    shift = 0;

    p = (uint8_t *) begin;

    do {
        *res |= ((uint64_t)((*p) & 0x7f) << shift);

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
oak_bin_sleb128_decode(const uint8_t *begin, const uint8_t *end, int64_t *res)
{
    uint8_t  shift, *p;

    *res = 0;
    shift = 0;

    p = (uint8_t *) begin;

    do {
        *res |= ((uint64_t)((*p) & 0x7f) << shift);
        shift += 7;
    } while ((*p++ & 0x80) != 0 && p <= end);

    --p;

    if ((shift < 64) && (*p & 0x40)) {
        /* shift an uint64 bit pattern to avoid -Werror=shift-negative-value */
        *res |= (int64_t)(((uint64_t)(-1) << shift));
    }

    return (p - begin) + 1;
}
