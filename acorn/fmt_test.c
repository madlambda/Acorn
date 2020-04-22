/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdlib.h>
#include <string.h>


static u8 test_stringfmt();
static u8 test_invalidverbs();


int
main()
{
    if (slow(test_stringfmt() != OK)) {
        return 1;
    }

    if (slow(test_invalidverbs() != OK)) {
        return 1;
    }

    return 0;
}


static u8
test_stringfmt()
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
test_invalidverbs()
{
    String  *res;

    res = cfmt("%z");
    if (slow(res == NULL)) {
        return ERR;
    }

    if (slow(!cstringcmp(res, "(invalid verb %z)"))) {
        return ERR;
    }

    free(res);

    return OK;
}
