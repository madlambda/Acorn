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

    u8          bits;
    u8          sign;

    const char  *want;
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
static u8 test_fmtchar();
static u8 test_invalidverbs();
static u8 test_customprinter();

static u8 assertstr(String *got, char *want);


static const Testcaseint  inttests[] = {
    {
        .format     = "%d",
        .sprintf    = "%d",
        .u.ival     = 0,
        .want       = "0",
    },
    {
        .format     = "%d",
        .sprintf    = "%d",
        .u.ival     = -1,
        .want       = "-1",
    },
    {
        .format     = "%da",
        .sprintf    = "%da",
        .u.ival     = 10000000,
        .want       = "10000000a",
    },
    {
        .format     = "%d",
        .sprintf    = "%d",
        .u.ival     = INT32_MIN,
        .want       = "-2147483648",
    },
    {
        .format     = "AI(Jijaijaankan%dnkjsdisahd9d",
        .sprintf    = "AI(Jijaijaankan%dnkjsdisahd9d",
        .u.ival     = INT32_MIN,
        .want       = "AI(Jijaijaankan-2147483648nkjsdisahd9d",
    },
    {
        .bits       = 8,
        .sign       = 1,
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = 0,
        .want       = "0",
    },
    {
        .bits       = 8,
        .sign       = 1,
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = 100,
        .want       = "100",
    },
    {
        .bits       = 8,
        .sign       = 1,
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = INT8_MAX,
        .want       = "127",
    },
    {
        .bits       = 8,
        .sign       = 1,
        .format     = "%d(i8)",
        .sprintf    = "%"PRIi8,
        .u.i8val    = INT8_MIN,
        .want       = "-128",
    },
    {
        .bits       = 16,
        .sign       = 1,
        .format     = "%d(i16)",
        .sprintf    = "%"PRIi16,
        .u.i16val   = 0,
        .want       = "0",
    },
    {
        .bits       = 16,
        .sign       = 1,
        .format     = "%d(i16)",
        .sprintf    = "%"PRIi16,
        .u.i16val   = INT16_MAX,
        .want       = "32767",
    },
    {
        .bits       = 16,
        .sign       = 1,
        .format     = "%d(i16)",
        .sprintf    = "%"PRIi16,
        .u.i16val   = INT16_MIN,
        .want       = "-32768",
    },
    {
        .bits       = 16,
        .sign       = 1,
        .format     = "%d(i16)test",
        .sprintf    = "%"PRIi16"test",
        .u.i16val   = 255,
        .want       = "255test",
    },
    {
        .bits       = 32,
        .sign       = 1,
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = 0,
        .want       = "0",
    },
    {
        .bits       = 32,
        .sign       = 1,
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = INT32_MAX,
        .want       = "2147483647",
    },
    {
        .bits       = 32,
        .sign       = 1,
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = INT32_MIN,
        .want       = "-2147483648",
    },
    {
        .bits       = 32,
        .sign       = 1,
        .format     = "%d(i32)test",
        .sprintf    = "%"PRIi32"test",
        .u.i32val   = 255,
        .want       = "255test",
    },
    {
        .bits       = 32,
        .sign       = 1,
        .format     = "%d(i32)",
        .sprintf    = "%"PRIi32,
        .u.i32val   = 0xffff,
        .want       = "65535",
    },
    {
        .bits       = 64,
        .sign       = 1,
        .format     = "%d(i64)",
        .sprintf    = "%"PRIi64,
        .u.i64val   = 0,
        .want       = "0",
    },
    {
        .bits       = 64,
        .sign       = 1,
        .format     = "%d(i64)",
        .sprintf    = "%"PRIi64,
        .u.i64val   = INT64_MAX,
        .want       = "9223372036854775807",
    },
    {
        .bits       = 64,
        .sign       = 1,
        .format     = "%d(i64)test",
        .sprintf    = "%"PRIi64"test",
        .u.i64val   = INT64_MAX,
        .want       = "9223372036854775807test",
    },
    {
        .bits       = 64,
        .sign       = 1,
        .format     = "%d(i64)test",
        .sprintf    = "%"PRIi64"test",
        .u.i64val   = INT64_MIN,
        .want       = "-9223372036854775808test",
    },
    {
        .bits       = 64,
        .sign       = 1,
        .format     = "%d(i64)",
        .sprintf    = "%"PRIi64,
        .u.i64val   = 0xff,
        .want       = "255",
    },
    {
        .bits       = 8,
        .format     = "%d(u8)",
        .sprintf    = "%"PRIu8,
        .u.u8val    = 0,
        .want       = "0",
    },
    {
        .bits       = 8,
        .format     = "%d(u8)",
        .sprintf    = "%"PRIu8,
        .u.u8val    = INT8_MAX,
        .want       = "127",
    },
    {
        .bits       = 8,
        .format     = "%d(u8)test",
        .sprintf    = "%"PRIu8"test",
        .u.u8val    = UINT8_MAX,
        .want       = "255test",
    },
    {
        .bits       = 8,
        .format     = "%d(u8)",
        .sprintf    = "%"PRIu8,
        .u.u8val    = 0xff,
        .want       = "255",
    },
    {
        .bits       = 16,
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu16,
        .u.u16val   = 0,
        .want       = "0",
    },
    {
        .bits       = 16,
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu16,
        .u.u16val   = INT16_MAX,
        .want       = "32767",
    },
    {
        .bits       = 16,
        .format     = "%d(u16)test",
        .sprintf    = "%"PRIu16"test",
        .u.u16val   = UINT16_MAX,
        .want       = "65535test",
    },
    {
        .bits       = 16,
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu16,
        .u.u16val   = 0xffff,
        .want       = "65535",
    },
    {
        .bits       = 32,
        .format     = "%d(u32)",
        .sprintf    = "%"PRIu32,
        .u.u32val   = 0,
        .want       = "0",
    },
    {
        .bits       = 32,
        .format     = "%d(u32)",
        .sprintf    = "%"PRIu32,
        .u.u32val   = INT32_MAX,
        .want       = "2147483647",
    },
    {
        .bits       = 32,
        .format     = "%d(u32)test",
        .sprintf    = "%"PRIu32"test",
        .u.u32val   = UINT32_MAX,
        .want       = "4294967295test",
    },
    {
        .bits       = 32,
        .format     = "%d(u16)",
        .sprintf    = "%"PRIu32,
        .u.u32val   = 0xffff,
        .want       = "65535",
    },
    {
        .bits       = 64,
        .format     = "%d(u64)",
        .sprintf    = "%"PRIu64,
        .u.u64val   = 0,
        .want       = "0",
    },
    {
        .bits       = 64,
        .format     = "%d(u64)",
        .sprintf    = "%"PRIu64,
        .u.u64val   = INT64_MAX,
        .want       = "9223372036854775807",
    },
    {
        .bits       = 64,
        .format     = "%d(u64)test",
        .sprintf    = "%"PRIu64"test",
        .u.u64val   = UINT64_MAX,
        .want       = "18446744073709551615test",
    },
    {
        .bits       = 64,
        .format     = "%d(u64)",
        .sprintf    = "%"PRIu64,
        .u.u64val   = 0xff,
        .want       = "255",
    },
    {
        .bits       = 32,
        .format     = "%x",
        .sprintf    = "%x",
        .u.ival     = 255,
        .want       = "ff",
    },
    {
        .bits       = 32,
        .format     = "%x",
        .sprintf    = "%x",
        .u.ival     = 0xa,
        .want       = "a",
    },
    {
        .bits       = 32,
        .format     = "%x",
        .sprintf    = "%x",
        .u.i32val   = 0xa11a,
        .want       = "a11a",
    },
    {
        .bits       = 64,
        .format     = "%x(u64)",
        .sprintf    = "%"PRIx64,
        .u.u64val   = UINT64_MAX,
        .want       = "ffffffffffffffff",
    },
    {
        .bits       = 64,
        .format     = "%x",
        .sprintf    = "%"PRIx64,
        .u.i64val   = 0xa,
        .want       = "a",
    },
    {
        .bits       = 64,
        .format     = "%x",
        .sprintf    = "%"PRIx64,
        .u.i64val   = 0xa11a,
        .want       = "a11a",
    },
    {
        .bits       = 64,
        .format     = "0x%x",
        .sprintf    = "0x%"PRIx64,
        .u.i64val   = 0xa11a,
        .want       = "0xa11a",
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
        test_customprinter,
    };

    res = cfmt("");
    if (slow(res == NULL)) {
        return 1;
    }

    free(res);

    for (i = 0; i < nitems(tests); i++) {
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

        switch (tc->bits) {
        case 0:
            sprintf(want, tc->sprintf, tc->u.ival);
            ret = assertstr(cfmt(tc->format, tc->u.ival), want);
            break;

        case 8:
            if (tc->sign) {
                sprintf(want, tc->sprintf, tc->u.i8val);
                ret = assertstr(cfmt(tc->format, tc->u.i8val), want);
            } else {
                sprintf(want, tc->sprintf, tc->u.u8val);
                ret = assertstr(cfmt(tc->format, tc->u.u8val), want);
            }
            break;

        case 16:
            if (tc->sign) {
                sprintf(want, tc->sprintf, tc->u.i16val);
                ret = assertstr(cfmt(tc->format, tc->u.i16val), want);
            } else {
                sprintf(want, tc->sprintf, tc->u.u16val);
                ret = assertstr(cfmt(tc->format, tc->u.u16val), want);
            }
            break;

        case 32:
            if (tc->sign) {
                sprintf(want, tc->sprintf, tc->u.i32val);
                ret = assertstr(cfmt(tc->format, tc->u.i32val), want);
            } else {
                sprintf(want, tc->sprintf, tc->u.u32val);
                ret = assertstr(cfmt(tc->format, tc->u.u32val), want);
            }
            break;

        case 64:
            if (tc->sign) {
                sprintf(want, tc->sprintf, tc->u.i64val);
                ret = assertstr(cfmt(tc->format, tc->u.i64val), want);
            } else {
                sprintf(want, tc->sprintf, tc->u.u64val);
                ret = assertstr(cfmt(tc->format, tc->u.u64val), want);
            }
            break;

        default:
            printf("invalid size: %d", tc->bits);
            return ERR;
        }

        if (slow(strcmp(want, tc->want) != 0)) {
            printf("unexpected libc sprintf \"%s\", want \"%s\"\n", want,
                   tc->want);

            return ERR;
        }

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
