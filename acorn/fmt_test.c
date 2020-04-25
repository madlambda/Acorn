/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static u8 test_fmtstring();
static u8 test_fmtint();
static u8 test_fmtchar();
static u8 test_invalidverbs();
static u8 test_customprinter();


int
main()
{
    if (slow(test_fmtstring() != OK)) {
        return 1;
    }

    if (slow(test_fmtint() != OK)) {
        return 1;
    }

    if (slow(test_fmtchar() != OK)) {
        return 1;
    }

    if (slow(test_invalidverbs() != OK)) {
        return 1;
    }

    if (slow(test_customprinter() != OK)) {
        return 1;
    }

    return 0;
}


static u8
test_fmtstring()
{
    String  *res;

    static const String  motivational = str(
        "The man who moves a mountain begins by carrying away small stones."
    );

    static const String  putyournamehere = str("i4k");

    res = cfmt("");
    if (slow(res == NULL)) {
        return ERR;
    }

    free(res);

    res = cfmt("\"%S\" - Confucius", &motivational);
    if (slow(res == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(res,
                         "\"The man who moves a mountain begins by carrying "
                         "away small stones.\" - Confucius")))
    {
        return ERR;
    }

    free(res);

    res = cfmt("The %s is out there, %S. It's looking for you, and it will "
               "find you %s to.", "answer", &putyournamehere, "if you want it");
    if (slow(res == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(res,
                         "The answer is out there, i4k. It's looking for you, "
                         "and it will find you if you want it to.")))
    {
        return ERR;
    }

    free(res);

    return OK;
}


static u8
test_fmtchar()
{
    u32     len;
    char    *p;
    String  *res;
    char    tmp[1024];

    res = cfmt("%c%c%c", 'i', '4', 'k');
    if (slow(res == NULL)) {
        goto error;
    }

    if (slow(!cstringcmp(res, "i4k"))) {
        goto error;
    }

    free(res);

    return OK;

error:

    if (res != NULL) {
        p = copy(tmp, res->start, res->len);
        *p = '\0';
        len = res->len;
        free(res);
        p = tmp;
    } else {
        len = 0;
        p = "res is NULL";
    }

    printf("failed: %s (len: %d)\n", p, len);

    return ERR;
}


static u8
test_fmtint()
{
    int     len;
    char    tmp[1024];
    char    *p;
    String  *res;

    res = cfmt("%d", 0);
    if (slow(res == NULL)) {
        goto error;
    }

    if (slow(!cstringcmp(res, "0"))) {
        goto error;
    }

    free(res);

    res = cfmt("%d %d %d %d", 4, 20, 69, 1337);
    if (slow(res == NULL)) {
        goto error;
    }

    if (slow(!cstringcmp(res, "4 20 69 1337"))) {
        goto error;
    }

    free(res);

    return OK;

error:

    if (res != NULL) {
        p = copy(tmp, res->start, res->len);
        *p = '\0';
        len = res->len;
        free(res);
        p = tmp;
    } else {
        len = 0;
        p = "res is NULL";
    }

    printf("failed: %s (len: %d)\n", p, len);

    return ERR;
}


static u8
test_invalidverbs()
{
    String  *res;

    res = cfmt("test %d %z", 10, 1);
    if (slow(res == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(res, "test 10 (invalid verb %z)"))) {
        return ERR;
    }

    free(res);

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
