/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include "test.h"
#include "array.h"


static u8 test_u8array();
static u8 test_structarray();
static u8 test_arraydel();
static u8 test_shrinkarray();


int
main()
{
    if (slow(newarray(0, 0) != NULL)) {
        error("array of zero sized elements must fail");
        return 1;
    }

    if (slow(test_u8array() != OK)) {
        return 1;
    }

    if (slow(test_structarray() != OK)) {
        return 1;
    }

    if (slow(test_arraydel() != OK)) {
        return 1;
    }

    if (slow(test_shrinkarray() != OK)) {
        return 1;
    }

    return 0;
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


static u8
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
            return error("failed newarray(%u, %u)", tc->nalloc, tc->size);
        }

        for (j = 0; j < (u8) tc->nitems; j++) {
            if (slow(arrayadd(arr, (u8 *) &tc->items[j]) != OK)) {
                return error("failed to arrayadd(*, %u)", j);
            }

            p = arrayget(arr, j);
            if (slow(p == NULL)) {
                return error("failed to arrayget(*, %u)", j);
            }

            if (slow(*p != tc->items[j])) {
                return error("set and get mismatch");
            }
        }

        if (slow(arraylen(arr) != tc->nitems)) {
            return error("wrong number of elements");
        }

        freearray(arr);
    }

    return OK;
}


typedef struct {
    u32  a;
    u32  b;
    u64  c;
} Struct;


static u8
test_structarray()
{
    u32     i, sum;
    Array   *arr;
    Struct  s, *p;

    arr = newarray(10, sizeof(Struct));
    if (slow(arr == NULL)) {
        return error("newarray(10, %u)", sizeof(Struct));
    }

    s.a = 1;
    s.b = 1;
    s.c = 1;

    if (slow(arrayadd(arr, &s) != OK)) {
        return error("arrayadd() fail");
    }

    s.a = 2;
    s.b = 2;
    s.c = 2;

    if (slow(arrayadd(arr, &s) != OK)) {
        return error("arrayadd() fail");
    }

    sum = 0;
    for (i = 0; i < arraylen(arr); i++) {
        p = arrayget(arr, i);
        sum += p->a + p->b + p->c;
    }

    if (slow(sum != 9)) {
        return error("arrayadd() not zeroing");
    }

    freearray(arr);

    return OK;
}


static u8
test_arraydel()
{
    u64    u, i, *p;
    Array  *arr;

    arr = newarray(16, sizeof(u64));
    if (slow(arr == NULL)) {
        return error("newarray(20, 8)");
    }

    u = 10;
    if (slow(arrayadd(arr, &u) != OK)) {
        return error("arrayadd() fail");
    }

    arraydel(arr, 0);

    if (slow(arr->nitems != 0)) {
        return error("arraydel() doesn't remove element");
    }

    for (i = 0; i < 20; i++) {
        if (slow(arrayadd(arr, &i) != OK)) {
            return error("arrayadd() failed");
        }
    }

    for (i = 0; i < arraylen(arr); i++) {
        p = arrayget(arr, i);

        if (slow(i != *p)) {
            return error("value mismatch");
        }
    }

    arraydel(arr, 5);

    for (i = 0; i < arraylen(arr); i++) {
        p = arrayget(arr, i);

        if (i < 5 && *p != i) {
            return error("elements < 5 must be unchanged");
        }

        if (slow(*p == 5)) {
            return error("value 5 still there");
        }

        if (i >= 5 && *p != (i+1)) {
            return error("elements >= 5 must be shifted");
        }
    }

    freearray(arr);

    return OK;
}


static u8
test_shrinkarray()
{
    u64    val;
    Array  *arr, *new;

    arr = newarray(100, sizeof(u64));
    if (slow(arr == NULL)) {
        return error("creating array");
    }

    if (slow(arraysize(arr) != (sizeof(u64) * 100))) {
        return error("alloc size is wrong");
    }

    new = shrinkarray(arr);
    if (slow(arraysize(new) != 0)) {
        return error("shrink alloc size is wrong");
    }

    val = 10;
    arrayadd(new, &val);
    arrayadd(new, &val);
    arrayadd(new, &val);

    new = shrinkarray(new);
    if (slow(arraysize(new) != (sizeof(u64) * 3))) {
        return error("shrink alloc size is wrong");
    }

    freearray(new);

    return OK;
}
