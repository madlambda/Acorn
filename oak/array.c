/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"


static u8 grow(Array *array);


/*
 * Allocates a new array for storing `n` items of size `size`.
 */
Array *
newarray(u32 n, size_t size)
{
    Array  *array;

    if (slow(size == 0)) {
        return NULL;
    }

    array = malloc(sizeof(Array) + n * size);
    if (slow(array == NULL)) {
        return NULL;
    }

    array->nitems = 0;
    array->nalloc = n;
    array->size = size;
    array->items = offset(array, sizeof(Array));

    return array;
}


void
freearray(Array *array)
{
    void  *p;

    p = offset(array, sizeof(Array));
    if (array->items != p) {
        free(array->items);
    }

    free(array);
}


u8
arrayadd(Array *array, void *val)
{
    void  *p;

    if (array->nitems == array->nalloc) {
        if (slow(grow(array) != OK)) {
            return ERR;
        }
    }

    p = offset(array->items, array->size * array->nitems);
    memcpy(p, val, array->size);

    array->nitems++;

    return OK;
}


static u8
grow(Array *array)
{
    u32   nalloc, newalloc;
    void  *p, *p2;

    nalloc = array->nalloc;

    if (array->nitems < 16) {
        newalloc = 1 + 2*nalloc;
    } else {
        newalloc = 1 + nalloc + nalloc/2;
    }

    p = array->items;

    array->items = malloc(array->size * newalloc);
    if (slow(array->items == NULL)) {
        return ERR;
    }

    array->nalloc = newalloc;
    memcpy(array->items, p, array->size * array->nitems);

    p2 = offset(array, sizeof(Array));
    if (p != p2) {
        /* old items are from different bucket than *array */
        free(p);
    }

    return OK;
}


void
arraydel(Array *array, u32 i)
{
    void  *target, *next;

    if (slow(i > array->nitems)) {
        return;
    }

    if (i == (array->nitems - 1)) {
        array->nitems--;
        return;
    }

    target = offset(array->items, i * array->size);
    next = offset(array->items, (i + 1) * array->size);
    memmove(target, next, (array->nitems - i) * array->size);

    array->nitems--;
}


void *
arrayget(Array *array, u32 i)
{
    if (slow(i >= array->nitems)) {
        return NULL;
    }

    return offset(array->items, i * array->size);
}


u8
arrayset(Array *array, u32 i, void *val)
{
    void  *p;

    if (slow(i >= (array->nitems - 1))) {
        return ERR;
    }

    p = offset(array->items, i * array->size);
    memcpy(p, val, array->size);

    return OK;
}


Array *
shrinkarray(Array *old)
{
    void   *items, *p;
    Array  *new;

    p = offset(old, sizeof(Array));
    items = old->items;

    if (old->nitems < old->nalloc || p != items) {
        new = newarray(old->nitems, old->size);
        if (new == NULL) {
            return old;
        }

        memcpy(new->items, old->items, old->nitems * old->size);
        new->nitems = old->nitems;

        freearray(old);

        return new;
    }

    return old;
}
