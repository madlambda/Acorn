/*
 * Copyright (C) Madlambda Authors
 */

/* required for test *only* to use printf PRIu64, PRId64, and so on */
#define __STDC_FORMAT_MACROS

#include <acorn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    const char  *format;
    const char  *sprintf;

    union {
        int     ival;
        i8      i8val;
        i16     i16val;
        i32     i32val;
        i64     i64val;
        u8      u8val;
        u16     u16val;
        u32     u32val;
        u64     u64val;
    } u;
} Testcaseint;


typedef u8 (*testfn)();


static u8 test_fmtstring();
static u8 test_fmtint();
static u8 test_fmti8();
static u8 test_fmti16();
static u8 test_fmti32();
static u8 test_fmti64();
static u8 test_fmtu8();
static u8 test_fmtu16();
static u8 test_fmtu32();
static u8 test_fmtu64();
static u8 test_fmtchar();
static u8 test_invalidverbs();
static u8 test_customprinter();

static u8 assertstr(String *got, char *want);


static const Testcaseint  inttests[] = {
    {
        .format = "%d",
        /* uses the same format for sprintf */
        .u.ival = 0,
    },
    {
        .format = "%d",
        .u.ival    = -1,
    },
    {
        .format = "%da",
        .u.ival    = 10000000,
    },
    {
        .format = "%d",
        .u.ival    = INT32_MIN,
    },
    {
        .format = "AI(Jijaijaankan%dnkjsdisahd9d",
        .u.ival    = INT32_MIN,
    },
};


/* signed integers */

static const Testcaseint  i8tests[] = {
    {
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = 0,
    },
    {
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = 100,
    },
    {
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = INT8_MAX,
    },
    {
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = INT8_MIN,
    },
};


static const Testcaseint  i16tests[] = {
    {
        .format     = "%d(i16)",
        .sprintf    = "%"PRIi16,
        .u.u16val   = 0,
    },
    {
        .format     = "%d(i16)",
        .sprintf    = "%"PRIi16,
        .u.i16val   = INT16_MAX,
    },
    {
        .format     = "%d(i16)",
        .sprintf    = "%"PRIu16,
        .u.i16val   = INT16_MIN,
    },
    {
        .format     = "%d(i16)test",
        .sprintf    = "%"PRIi16"test",
        .u.i16val   = 255,
    },
};


static const Testcaseint  i32tests[] = {
    {
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = 0,
    },
    {
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = INT32_MAX,
    },
    {
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = INT32_MIN,
    },
    {
        .format     = "%d(i32)test",
        .sprintf    = "%"PRIi32"test",
        .u.i32val   = 255,
    },
    {
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = 0xffff,
    },
};


static const Testcaseint  i64tests[] = {
    {
        .format     = "%d(i64)",
        .sprintf    = "%"PRIi64,
        .u.i64val   = 0,
    },
    {
        .format     = "%d(i64)",
        .sprintf    = "%"PRIi64,
        .u.i64val   = INT64_MAX,
    },
    {
        .format     = "%d(i64)test",
        .sprintf    = "%"PRIi64"test",
        .u.i64val   = INT64_MAX,
    },
    {
        .format     = "%d(i64)test",
        .sprintf    = "%"PRIi64"test",
        .u.i64val   = INT64_MIN,
    },
    {
        .format     = "%d(i64)",
        .sprintf    = "%"PRIi64,
        .u.i64val   = 0xff,
    },
};


/* unsigned integers */

static const Testcaseint  u8tests[] = {
    {
        .format     = "%d(u8)",
        .sprintf    = "%"PRIu8,
        .u.u8val    = 0,
    },
    {
        .format     = "%d(u8)",
        .sprintf    = "%"PRIu8,
        .u.u8val    = INT8_MAX,
    },
    {
        .format     = "%d(u8)test",
        .sprintf    = "%"PRIu8"test",
        .u.u8val    = UINT8_MAX,
    },
    {
        .format     = "%d(u8)",
        .sprintf    = "%"PRIu8,
        .u.u8val    = 0xff,
    },
};


static const Testcaseint  u16tests[] = {
    {
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu16,
        .u.u16val   = 0,
    },
    {
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu16,
        .u.u16val   = INT16_MAX,
    },
    {
        .format     = "%d(u16)test",
        .sprintf    = "%"PRIu16"test",
        .u.u16val   = UINT16_MAX,
    },
    {
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu16,
        .u.u16val   = 0xffff,
    },
};


static const Testcaseint  u32tests[] = {
    {
        .format     = "%d(u32)",
        .sprintf    = "%"PRIu32,
        .u.u32val   = 0,
    },
    {
        .format     = "%d(u32)",
        .sprintf    = "%"PRIu32,
        .u.u32val   = INT32_MAX,
    },
    {
        .format     = "%d(u32)test",
        .sprintf    = "%"PRIu32"test",
        .u.u32val   = UINT32_MAX,
    },
    {
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu32,
        .u.u32val   = 0xffff,
    },
};


static const Testcaseint  u64tests[] = {
    {
        .format     = "%d(u64)",
        .sprintf    = "%"PRIu64,
        .u.u64val   = 0,
    },
    {
        .format     = "%d(u64)",
        .sprintf    = "%"PRIu64,
        .u.u64val   = INT64_MAX,
    },
    {
        .format     = "%d(u64)test",
        .sprintf    = "%"PRIu64"test",
        .u.u64val   = UINT64_MAX,
    },
    {
        .format     = "%d(u64)",
        .sprintf    = "%"PRIu64,
        .u.u64val   = 0xff,
    },
};


int
main()
{
    u32     i;
    String  *res;

    static const testfn  tests[] = {
        test_invalidverbs,
        test_fmtchar,
        test_fmtstring,

        test_fmtint,
        test_fmtu8,
        test_fmtu16,
        test_fmtu32,
        test_fmtu64,
        test_fmti8,
        test_fmti16,
        test_fmti32,
        test_fmti64,

        test_customprinter,
    };

    res = cfmt("");
    if (slow(res == NULL)) {
        return 1;
    }

    free(res);

    for (i = 0; i < 9; i++) {
        if (slow(tests[i]() != OK)) {
            return 1;
        }
    }

    return 0;
}


static u8
test_fmtstring()
{
    static const String  motivational = str(
        "The man who moves a mountain begins by carrying away small stones."
    );

    static const String  putyournamehere = str("i4k");

    if (assertstr(cfmt("\"%S\" - Confucius", &motivational),
                  "\"The man who moves a mountain begins by carrying "
                  "away small stones.\" - Confucius") != OK)
    {
        return ERR;
    }

    if (assertstr(
            cfmt("The %s is out there, %S. It's looking for you, and it will "
                 "find you %s to.", "answer", &putyournamehere,
                 "if you want it"),
            "The answer is out there, i4k. It's looking for you, "
            "and it will find you if you want it to.") != OK)
    {
        return ERR;
    }

    return OK;
}


static u8
test_fmtchar()
{
    if (assertstr(cfmt("%c%c%c", 'i', '4', 'k'), "i4k") != OK) {
        return ERR;
    }

    return OK;
}


static u8
test_fmtint()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(inttests); i++) {
        tc = &inttests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->format, tc->u.ival);

        ret = assertstr(cfmt(tc->format, tc->u.ival), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmtu8()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(u8tests); i++) {
        tc = &u8tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.u8val);

        ret = assertstr(cfmt(tc->format, tc->u.u8val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmtu16()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(u16tests); i++) {
        tc = &u16tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.u16val);

        ret = assertstr(cfmt(tc->format, tc->u.u16val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmtu32()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(u32tests); i++) {
        tc = &u32tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.u32val);

        ret = assertstr(cfmt(tc->format, tc->u.u32val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmtu64()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(u64tests); i++) {
        tc = &u64tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.u64val);

        ret = assertstr(cfmt(tc->format, tc->u.u64val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmti8()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(i8tests); i++) {
        tc = &i8tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.i8val);

        ret = assertstr(cfmt(tc->format, tc->u.i8val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmti16()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(i16tests); i++) {
        tc = &i16tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.i16val);

        ret = assertstr(cfmt(tc->format, tc->u.i16val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmti32()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(i32tests); i++) {
        tc = &i32tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.i32val);

        ret = assertstr(cfmt(tc->format, tc->u.i32val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_fmti64()
{
    u8                 ret;
    u32                i;
    const Testcaseint  *tc;
    char               want[256];

    for (i = 0; i < nitems(i64tests); i++) {
        tc = &i64tests[i];

        memset(want, 0, sizeof(want));
        sprintf(want, tc->sprintf, tc->u.i64val);

        ret = assertstr(cfmt(tc->format, tc->u.i64val), want);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
test_invalidverbs()
{
    if (assertstr(cfmt("test %d %z", 10, 1), "test 10 (invalid verb %z)")
        != OK)
    {
        return ERR;
    }

    if (assertstr(cfmt("test %d(v32)", 10), "test (invalid verb %d(v32))")
        != OK)
    {
        return ERR;
    }

    return OK;
}


static u8
mydumbfmt1(String **buf, u8 ** unused(format), void * unused(val))
{
    *buf = appendcstr(*buf, "dumb formatter");
    return OK;
}


typedef struct {
    char  a;
    char  b;
} Custom;


static u8
mydumbfmt2(String **buf, u8 ** unused(format), void *val)
{
    Custom  *c;

    c = (Custom *) val;

    *buf = appendc(*buf, 10,
                   'a', ':', ' ', c->a, ',', ' ', 'b', ':', ' ', c->b);
    return OK;
}


static u8
test_customprinter()
{
    String   *s;
    Custom   custom;
    Printer  *printer;

    printer = newprinter();
    if (slow(printer == NULL)) {
        return ERR;
    }

    s = pcfmt(printer, "a%sb", "test");
    if (slow(s == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(s, "atestb"))) {
        return ERR;
    }

    free(s);

    pfmtadd(printer, 'D', mydumbfmt1);

    s = pcfmt(printer, "My %D.", "ignored");
    if (slow(s == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(s, "My dumb formatter."))) {
        return ERR;
    }

    free(s);

    pfmtadd(printer, 'C', mydumbfmt2);

    custom.a = 'Z';
    custom.b = 'Y';

    s = pcfmt(printer, "Values: %C", &custom);
    if (slow(s == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(s, "Values: a: Z, b: Y"))) {
        return ERR;
    }

    free(s);
    free(printer);

    return OK;
}


static u8
assertstr(String *got, char *want)
{
    u8    ret;
    char  *p;
    char  cgot[256];

    ret = OK;

    if (slow(got == NULL)) {
        printf("[error] got NULL but want \"%s\"\n", want);
        return ERR;
    }

    if (slow(!cstringcmp(got, want))) {
        p = copy(cgot, got->start, got->len);
        *p = '\0';

        printf("[error] got \"%s\" but want \"%s\"\n", cgot, want);

        ret = ERR;
    }

    free(got);
    return ret;
}
