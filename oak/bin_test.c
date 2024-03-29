/*
 * Copyright (C) Madlambda Authors.
 */

#include <sys/types.h>
#include <stdlib.h>

#include <acorn.h>
#include "bin.h"
#include "test.h"


typedef struct {
    u64  val;
    u8   size;
    u8   want[256];
} UTestcase;


typedef struct {
    i64  val;
    u8   size;
    u8   want[256];
} STestcase;


Error *test_uvencode(u64 v, const u8 *want, u8 size);
Error *test_uvdecode(const u8 *encoded, u8 size, u64 want);
Error *test_svencode(i64 v, const u8 *want, u8 size);
Error *test_svdecode(const u8 *encoded, u8 size, i64 want);
Error *test_encodeoverflow();
Error *test_decodemalformed();


static const UTestcase  utestcases[] = {
    {0, 1, {0x0}},
    {1, 1, {0x1}},
    {2, 1, {0x2}},
    {50, 1, {0x32}},
    {100, 1, {0x64}},
    {0x7f, 1, {0x7f}},
    {666, 2, {0x9a, 0x5}},
    {16256, 2, {0x80, 0x7f}},
    {0x3b4, 2, {0xb4, 0x07}},
    {0x40c, 2, {0x8c, 0x08}},
    {624485, 3, {0xe5, 0x8e, 0x26}},
    {0, 1, {0x0}},
    {0xff, 2, {0xff, 0x1}},
    {0xffff, 3, {0xff, 0xff, 0x3}},
    {0xffffff, 4, {0xff, 0xff, 0xff, 0x7}},
    {0xffffffff, 5, {0xff, 0xff, 0xff, 0xff, 0xf}},
    {0xffffffffff, 6, {0xff, 0xff, 0xff, 0xff, 0xff, 0x1f}},
    {0xffffffffffff, 7, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f}},
    {0xffffffffffffff, 8, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}},
    {UINT64_MAX, 10, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0x1}},
};


static const STestcase  stestcases[] = {
    {0, 1, {0x0}},
    {1, 1, {0x1}},
    {1, 1, {0x1}},
    {2, 1, {0x2}},
    {50, 1, {0x32}},
    {0x3f, 1, {0x3f}},  /* 111111 */
    {0xff, 2, {0xff, 0x1}},
    {0xffff, 3, {0xff, 0xff, 0x3}},
    {0xffffff, 4, {0xff, 0xff, 0xff, 0x7}},
    {0xffffffff, 5, {0xff, 0xff, 0xff, 0xff, 0xf}},
    {0xffffffffff, 6, {0xff, 0xff, 0xff, 0xff, 0xff, 0x1f}},
    {0xffffffffffff, 7, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f}},
    {0xffffffffffffff, 9, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                           0x00}},
    {INT64_MAX, 10, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}},
    {0xffffffffffffffff, 1, {0x7f}}, /* == -1 (overflow) */
    {-1, 1, {0x7f}},
    {-2, 1, {0x7e}},
    {-3, 1, {0x7d}},
    {-32, 1, {0x60}},
    {-50, 1, {0x4e}},
    {-0x3f, 1, {0x41}},
    {-128, 2, {0x80, 0x7f}},
    {-254, 2, {0x82, 0x7e}},
    {-255, 2, {0x81, 0x7e}},
    {-256, 2, {0x80, 0x7e}},
    {-123456, 3, {0xc0, 0xbb, 0x78}},
    {-65535, 3, {0x81, 0x80, 0x7c}},
    {-16777215, 4, {0x81, 0x80, 0x80, 0x78}},
    {-33554430, 4, {0x82, 0x80, 0x80, 0x70}},
    {-67108860, 4, {0x84, 0x80, 0x80, 0x60}},
    {-134217720, 4, {0x88, 0x80, 0x80, 0x40}},
    {-268435440, 5, {0x90, 0x80, 0x80, 0x80, 0x7f}},
    {-536870880, 5, {0xa0, 0x80, 0x80, 0x80, 0x7e}},
    {-1073741760, 5, {0xc0, 0x80, 0x80, 0x80, 0x7c}},
    {-2147483520, 5, {0x80, 0x81, 0x80, 0x80, 0x78}},
    {-4294967040, 5, {0x80, 0x82, 0x80, 0x80, 0x70}},
    {-8589934080, 5, {0x80, 0x84, 0x80, 0x80, 0x60}},
    {-17179868160, 5, {0x80, 0x88, 0x80, 0x80, 0x40}},
    {-34359736320, 6, {0x80, 0x90, 0x80, 0x80, 0x80, 0x7f}},
    {-68719472640, 6, {0x80, 0xa0, 0x80, 0x80, 0x80, 0x7e}},
    {-137438945280, 6, {0x80, 0xc0, 0x80, 0x80, 0x80, 0x7c}},
    {-274877890560, 6, {0x80, 0x80, 0x81, 0x80, 0x80, 0x78}},
    {-549755781120, 6, {0x80, 0x80, 0x82, 0x80, 0x80, 0x70}},
    {-1099511562240, 6, {0x80, 0x80, 0x84, 0x80, 0x80, 0x60}},
    {-2199023124480, 6, {0x80, 0x80, 0x88, 0x80, 0x80, 0x40}},
    {-4398046248960, 7, {0x80, 0x80, 0x90, 0x80, 0x80, 0x80, 0x7f}},
    {-8796092497920, 7, {0x80, 0x80, 0xa0, 0x80, 0x80, 0x80, 0x7e}},
    {-17592184995840, 7, {0x80, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x7c}},
    {-35184369991680, 7, {0x80, 0x80, 0x80, 0x81, 0x80, 0x80, 0x78}},
    {-70368739983360, 7, {0x80, 0x80, 0x80, 0x82, 0x80, 0x80, 0x70}},
    {-140737479966720, 7, {0x80, 0x80, 0x80, 0x84, 0x80, 0x80, 0x60}},
    {-281474959933440, 7, {0x80, 0x80, 0x80, 0x88, 0x80, 0x80, 0x40}},
    {-562949919866880, 8, {0x80, 0x80, 0x80, 0x90, 0x80, 0x80, 0x80, 0x7f}},
    {-1125899839733760, 8, {0x80, 0x80, 0x80, 0xa0, 0x80, 0x80, 0x80, 0x7e}},
    {-2251799679467520, 8, {0x80, 0x80, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x7c}},
    {-4503599358935040, 8, {0x80, 0x80, 0x80, 0x80, 0x81, 0x80, 0x80, 0x78}},
    {-9007198717870080, 8, {0x80, 0x80, 0x80, 0x80, 0x82, 0x80, 0x80, 0x70}},
    {-18014397435740160, 8, {0x80, 0x80, 0x80, 0x80, 0x84, 0x80, 0x80, 0x60}},
    {-36028794871480320, 8, {0x80, 0x80, 0x80, 0x80, 0x88, 0x80, 0x80, 0x40}},
    {-72057589742960640, 9, {0x80, 0x80, 0x80, 0x80, 0x90, 0x80, 0x80, 0x80,
                             0x7f}},
    {-144115179485921280, 9, {0x80, 0x80, 0x80, 0x80, 0xa0, 0x80, 0x80, 0x80,
                              0x7e}},
    {-288230358971842560, 9, {0x80, 0x80, 0x80, 0x80, 0xc0, 0x80, 0x80, 0x80,
                              0x7c}},
    {-576460717943685120, 9, {0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 0x80, 0x80,
                              0x78}},
    {-1152921435887370240, 9, {0x80, 0x80, 0x80, 0x80, 0x80, 0x82, 0x80, 0x80,
                               0x70}},
    {-2305842871774740480, 9, {0x80, 0x80, 0x80, 0x80, 0x80, 0x84, 0x80, 0x80,
                               0x60}},
    {-4611685743549480960, 9, {0x80, 0x80, 0x80, 0x80, 0x80, 0x88, 0x80, 0x80,
                               0x40}},
    {-9223371487098961920, 10, {0x80, 0x80, 0x80, 0x80, 0x80, 0x90, 0x80, 0x80,
                               0x80, 0x7f}},
    {INT64_MIN, 10, {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                               0x80, 0x7f}},
};


#define OAK_ULEB128_NTESTS                                                    \
    (sizeof(utestcases)/sizeof(utestcases[0]))

#define OAK_SLEB128_NTESTS                                                    \
    (sizeof(stestcases)/sizeof(stestcases[0]))


int main() {
    size_t           i;
    Error            *err;
    const UTestcase  *utc;
    const STestcase  *stc;

    fmtadd('e', errorfmt);

    err = test_encodeoverflow();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_decodemalformed();
    if (slow(err != NULL)) {
        goto fail;
    }

    for (i = 0; i < OAK_ULEB128_NTESTS; i++) {
        utc = &utestcases[i];
        err = test_uvencode(utc->val, utc->want, utc->size);
        if (slow(err != NULL)) {
            goto fail;
        }

        err = test_uvdecode(utc->want, utc->size, utc->val);
        if (slow(err != NULL)) {
            goto fail;
        }
    }

    for (i = 0; i < OAK_SLEB128_NTESTS; i++) {
        stc = &stestcases[i];
        err = test_svencode(stc->val, stc->want, stc->size);
        if (slow(err != NULL)) {
            goto fail;
        }

        err = test_svdecode(stc->want, stc->size, stc->val);
        if (slow(err != NULL)) {
            goto fail;
        }
    }

    return 0;

fail:

    cprint("[error] %e", err);

    errorfree(err);

    return 1;
}


Error *
test_encodeoverflow()
{
    u8       *end;
    ssize_t  n;
    u8       buf[10];

    /* tests for writing past the buffer */
    end = buf;
    n = uvencode(100, buf, end);
    if (slow(n != -1)) {
        return newerror("should have failed");
    }

    end = buf + 1;
    n = uvencode(0xfffffff, buf, end);
    if (slow(n != -1)) {
        return newerror("should have failed");
    }

    end = buf;
    n = svencode(100, buf, end);
    if (slow(n != -1)) {
        return newerror("should have failed");
    }

    end = buf + 1;
    n = svencode(0xfffffff, buf, end);
    if (slow(n != -1)) {
        return newerror("should have failed");
    }

    return NULL;
}


Error *
test_decodemalformed()
{
    u64       uval;
    ssize_t   n;
    const u8  *end;

    const u8  continuationbit[]  = {0x80};
    const u8  continuationbit2[] = {0xff, 0xff};

    end = continuationbit + 1;

    n = uvdecode(continuationbit, end, &uval);
    if (slow(n != -1)) {
        return newerror("want -1 but got %ld (%lu)", n, uval);
    }

    end = continuationbit2 + 1;

    n = uvdecode(continuationbit2, end, &uval);
    if (slow(n != -1)) {
        return newerror("want -1 but got %ld (%lu)", n, uval);
    }

    return NULL;
}


Error *
test_uvencode(u64 v, const u8 *want, u8 size)
{
    u8     *got, *end;
    int    i, n;
    Error  *err;

    got = mustalloc(size);
    end = got + size;

    n = uvencode(v, got, end);

    err = NULL;

    if (slow(n != size)) {
        err = newerror("uleb enc: val %ld, encoded %d bytes but expected %d",
                       v, n, size);

        goto fail;
    }

    for (i = 0; i < size; i++) {
        if (slow(want[i] != got[i])) {
            err = newerror("uleb enc: val %ld, byte %d, 0x%x != 0x%x",
                           v, i, want[i], got[i]);

            goto fail;
        }
    }

fail:

    free(got);

    return err;
}


Error *
test_uvdecode(const u8 *encoded, u8 size, u64 want)
{
    u64       got;
    ssize_t   got_size;
    const u8  *end;

    end = encoded + size;

    got_size = uvdecode(encoded, end, &got);
    if (slow((ssize_t) size != got_size)) {
        return newerror("decoding for %lu: read %d bytes but want %d", want,
                        got_size, size);
    }

    if (slow(got != want)) {
        return newerror("decoding for %lu: got %llu", want, got);
    }

    return NULL;
}


Error *
test_svencode(i64 v, const u8 *want, u8 size)
{
    u8     *got, *end;
    int    i, n;
    Error  *err;

    got = mustalloc(size);
    end = got + size;

    err = NULL;

    n = svencode(v, got, end);

    if (slow(n != size)) {
        err = newerror("sleb enc: val %ld, encoded %d bytes but expected %d",
                       v, n, size);

        goto fail;
    }

    for (i = 0; i < size; i++) {
        if (slow(want[i] != got[i])) {
            err = newerror("sleb enc: val %ld, byte %d, 0x%x != 0x%x",
                           v, i, want[i], got[i]);

            goto fail;
        }
    }

fail:

    free(got);

    return err;
}


Error *
test_svdecode(const u8 *encoded, u8 size, i64 want) {
    i64       got;
    ssize_t   got_size;
    const u8  *end;

    end = encoded + size;

    got_size = svdecode(encoded, end, &got);
    if (slow((ssize_t) size != got_size)) {
        return newerror("decoding for %lld: read %d bytes but want %d", want,
                        got_size, size);
    }

    if (slow(got != want)) {
        return newerror("decoding for %ld: got %ld", want, got);
    }

    return NULL;
}
