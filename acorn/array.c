/*
 * Copyright (C) Madlambda Authors
 */

#include <stdlib.h>
#include <string.h>

#include <acorn.h>
#include <acorn/array.h>



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

    array = malloc(arraytotalsize(n, size));
    if (slow(array == NULL)) {
        return NULL;
    }

    initarrayfrom(array, n, size, 1);

    return array;
}


void
freearray(Array *array)
{
    void  *p;

    if(!array->heap) {
        return;
    }

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

    if (array->len == array->nalloc) {
        if (slow(grow(array) != OK)) {
            return ERR;
        }
    }

    p = offset(array->items, array->size * array->len);
    memcpy(p, val, array->size);

    array->len++;

    return OK;
}


u8
arraypush(Array *array, void *val)
{
    return arrayadd(array, val);
}


void *
arraypop(Array *array)
{
    void  *val;

    expect(array->len > 0);

    val = arrayget(array, array->len - 1);
    array->len--;
    return val;
}


static u8
grow(Array *array)
{
    u32   nalloc, newalloc;
    void  *p, *p2;

    if (!array->heap) {
        return ERR;
    }

    nalloc = array->nalloc;

    if (array->len < 16) {
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
    memcpy(array->items, p, array->size * array->len);

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

    if (slow(i > array->len)) {
        return;
    }

    if (i == (array->len - 1)) {
        array->len--;
        return;
    }

    target = offset(array->items, i * array->size);
    next = offset(array->items, (i + 1) * array->size);
    memmove(target, next, (array->len - i) * array->size);

    array->len--;
}


void *
arrayget(Array *array, u32 i)
{
    if (slow(i >= array->len)) {
        return NULL;
    }

    return offset(array->items, i * array->size);
}


u8
arrayset(Array *array, u32 i, void *val)
{
    void  *p;

    if (slow(i >= (array->len - 1))) {
        return ERR;
    }

    p = offset(array->items, i * array->size);
    memcpy(p, val, array->size);

    return OK;
}


Array *
shrinkarray(Array *old)
{
    void   *p;
    Array  *new;

    p = offset(old, sizeof(Array));

    if (old->len < old->nalloc || p != old->items) {
        new = newarray(old->len, old->size);
        if (new == NULL) {
            return old;
        }

        memcpy(new->items, old->items, old->len * old->size);
        new->len = old->len;

        freearray(old);

        return new;
    }

    return old;
}
