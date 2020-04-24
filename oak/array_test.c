/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdio.h>
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
        printf("array of zero sized elements must fail\n");
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
            printf("failed newarray(%u, %lu)\n", tc->nalloc, tc->size);
            return ERR;
        }

        for (j = 0; j < (u8) tc->nitems; j++) {
            if (slow(arrayadd(arr, (u8 *) &tc->items[j]) != OK)) {
                printf("failed to arrayadd(*, %u)\n", j);
                return ERR;
            }

            p = arrayget(arr, j);
            if (slow(p == NULL)) {
                printf("failed to arrayget(*, %u)\n", j);
                return ERR;
            }

            if (slow(*p != tc->items[j])) {
                printf("set and get mismatch\n");
                return ERR;
            }
        }

        if (slow(len(arr) != tc->nitems)) {
            printf("wrong number of elements\n");
            return ERR;
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
        printf("newarray(10, %lu)", sizeof(Struct));
        return ERR;
    }

    s.a = 1;
    s.b = 1;
    s.c = 1;

    if (slow(arrayadd(arr, &s) != OK)) {
        printf("arrayadd() fail");
        return ERR;
    }

    s.a = 2;
    s.b = 2;
    s.c = 2;

    if (slow(arrayadd(arr, &s) != OK)) {
        printf("arrayadd() fail");
        return ERR;
    }

    sum = 0;
    for (i = 0; i < len(arr); i++) {
        p = arrayget(arr, i);
        sum += p->a + p->b + p->c;
    }

    if (slow(sum != 9)) {
        printf("arrayadd() not zeroing");
        return ERR;
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
        printf("newarray(20, 8)");
        return ERR;
    }

    u = 10;
    if (slow(arrayadd(arr, &u) != OK)) {
        printf("arrayadd() fail");
        return ERR;
    }

    arraydel(arr, 0);

    if (slow(len(arr) != 0)) {
        printf("arraydel() doesn't remove element");
        return ERR;
    }

    for (i = 0; i < 20; i++) {
        if (slow(arrayadd(arr, &i) != OK)) {
            printf("arrayadd() failed");
            return ERR;
        }
    }

    for (i = 0; i < len(arr); i++) {
        p = arrayget(arr, i);

        if (slow(i != *p)) {
            printf("value mismatch");
            return ERR;
        }
    }

    arraydel(arr, 5);

    for (i = 0; i < len(arr); i++) {
        p = arrayget(arr, i);

        if (i < 5 && *p != i) {
            printf("elements < 5 must be unchanged");
            return ERR;
        }

        if (slow(*p == 5)) {
            printf("value 5 still there");
            return ERR;
        }

        if (i >= 5 && *p != (i+1)) {
            printf("elements >= 5 must be shifted");
            return ERR;
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
        printf("creating array");
        return ERR;
    }

    if (slow(arraysize(arr) != (sizeof(u64) * 100))) {
        printf("alloc size is wrong");
        return ERR;
    }

    new = shrinkarray(arr);
    if (slow(arraysize(new) != 0)) {
        printf("shrink alloc size is wrong");
        return ERR;
    }

    val = 10;
    arrayadd(new, &val);
    arrayadd(new, &val);
    arrayadd(new, &val);

    new = shrinkarray(new);
    if (slow(arraysize(new) != (sizeof(u64) * 3))) {
        printf("shrink alloc size is wrong");
        return ERR;
    }

    freearray(new);

    return OK;
}
