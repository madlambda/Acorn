/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdio.h>
#include "test.h"
#include "array.h"
#include "error.h"


static Error *test_u8array();
static Error *test_structarray();
static Error *test_arraydel();
static Error *test_shrinkarray();


int
main()
{
    Error  *err;

    fmtadd('e', errorfmt);

    if (slow(newarray(0, 0) != NULL)) {
        printf("array of zero sized elements must fail\n");
        return 1;
    }

    err = test_u8array();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_structarray();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_arraydel();
    if (slow(err != NULL)) {
        goto fail;
    }

    err = test_shrinkarray();
    if (slow(err != NULL)) {
        goto fail;
    }

    return 0;

fail:

    cprint("[error] %e", err);
    return 1;
}


typedef struct {
    u32     nalloc;
    size_t  size;
    u32     nitems;
    u64     items[256];
} Testcase;


static const Testcase  u8testcases[] = {
    {0, 1, 3, {4, 20, 69}},
};


static Error *
test_u8array()
{
    u8              j, *p;
    u32             i;
    Array           *arr;
    const Testcase  *tc;

    for (i = 0; i < nitems(u8testcases); i++) {
        tc = &u8testcases[i];
        arr = newarray(tc->nalloc, tc->size);
        if (slow(arr == NULL)) {
            return newerror("failed newarray(%u, %lu)", tc->nalloc, tc->size);
        }

        for (j = 0; j < (u8) tc->nitems; j++) {
            if (slow(arrayadd(arr, (u8 *) &tc->items[j]) != OK)) {
                return newerror("failed to arrayadd(*, %u)", j);
            }

            p = arrayget(arr, j);
            if (slow(p == NULL)) {
                return newerror("failed to arrayget(*, %u)", j);
            }

            if (slow(*p != tc->items[j])) {
                return newerror("set and get mismatch");
            }
        }

        if (slow(len(arr) != tc->nitems)) {
            return newerror("wrong number of elements");
        }

        freearray(arr);
    }

    return NULL;
}


typedef struct {
    u32  a;
    u32  b;
    u64  c;
} Struct;


static Error *
test_structarray()
{
    u32     i, sum;
    Array   *arr;
    Struct  s, *p;

    arr = newarray(10, sizeof(Struct));
    if (slow(arr == NULL)) {
        return newerror("newarray(10, %lu)", sizeof(Struct));
    }

    s.a = 1;
    s.b = 1;
    s.c = 1;

    if (slow(arrayadd(arr, &s) != OK)) {
        return newerror("arrayadd() fail");
    }

    s.a = 2;
    s.b = 2;
    s.c = 2;

    if (slow(arrayadd(arr, &s) != OK)) {
        return newerror("arrayadd() fail");
    }

    sum = 0;
    for (i = 0; i < len(arr); i++) {
        p = arrayget(arr, i);
        sum += p->a + p->b + p->c;
    }

    if (slow(sum != 9)) {
        return newerror("arrayadd() not zeroing");
    }

    freearray(arr);

    return NULL;
}


static Error *
test_arraydel()
{
    u64    u, i, *p;
    Array  *arr;

    arr = newarray(16, sizeof(u64));
    if (slow(arr == NULL)) {
        return newerror("newarray(20, 8)");
    }

    u = 10;
    if (slow(arrayadd(arr, &u) != OK)) {
        return newerror("arrayadd() fail");
    }

    arraydel(arr, 0);

    if (slow(len(arr) != 0)) {
        return newerror("arraydel() doesn't remove element");
    }

    for (i = 0; i < 20; i++) {
        if (slow(arrayadd(arr, &i) != OK)) {
            return newerror("arrayadd() failed");
        }
    }

    for (i = 0; i < len(arr); i++) {
        p = arrayget(arr, i);

        if (slow(i != *p)) {
            return newerror("value mismatch");
        }
    }

    arraydel(arr, 5);

    for (i = 0; i < len(arr); i++) {
        p = arrayget(arr, i);

        if (i < 5 && *p != i) {
            return newerror("elements < 5 must be unchanged");
        }

        if (slow(*p == 5)) {
            return newerror("value 5 still there");
        }

        if (i >= 5 && *p != (i+1)) {
            return newerror("elements >= 5 must be shifted");
        }
    }

    freearray(arr);

    return NULL;
}


static Error *
test_shrinkarray()
{
    u64    val;
    Array  *arr, *new;

    arr = newarray(100, sizeof(u64));
    if (slow(arr == NULL)) {
        return newerror("creating array");
    }

    if (slow(arraysize(arr) != (sizeof(u64) * 100))) {
        return newerror("alloc size is wrong");
    }

    new = shrinkarray(arr);
    if (slow(arraysize(new) != 0)) {
        return newerror("shrink alloc size is wrong");
    }

    val = 10;
    arrayadd(new, &val);
    arrayadd(new, &val);
    arrayadd(new, &val);

    new = shrinkarray(new);
    if (slow(arraysize(new) != (sizeof(u64) * 3))) {
        return newerror("shrink alloc size is wrong");
    }

    freearray(new);

    return NULL;
}
