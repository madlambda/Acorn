/*
 * Copyright (C) Madlambda Authors.
 */


#ifndef _ACORN_ARRAY_H_
#define _ACORN_ARRAY_H_


#define arraylast(array)                                                      \
    offset((array)->items, (array)->size * ((array)->len - 1))


#define arraysize(array)                                                      \
    (array->size * array->nalloc)


#define arraytotalsize(n, size)                                               \
    (sizeof(Array) + (n * size))


typedef struct {
    u32     len;
    u32     nalloc;
    size_t  size;
    void    *items;
    u8      heap;
} Array;


Array   *newarray(u32 nitems, size_t size);
void    freearray(Array *);
u8      arrayadd(Array *, void *val);
void    *arrayget(Array *, u32 index);
u8      arrayset(Array *, u32 index, void *val);
void    arraydel(Array *, u32 index);
Array   *shrinkarray(Array *old);

/* stack api */
u8      arraypush(Array *array, void *val);
void    *arraypop(Array *array);


inline static void
initarrayfrom(void *data, u32 n, size_t size, u8 heap)
{
    Array  *array;

    expect(data != NULL);

    array = data;
    array->heap = heap;
    array->items = offset(array, sizeof(Array));
    array->size = size;
    array->nalloc = n;
    array->len = 0;
}


/* memset */
#include <string.h>


inline static void
arrayzerorange(Array *array, u32 from, u32 to)
{
    void  *begin, *end;

    if (from == to) {
        return;
    }

    expect(from < to);

    begin = offset(array->items, from * array->size);
    end = offset(array->items, (to * array->size) + array->size);

    memset(begin, 0, end - begin);
}


#endif /* _ACORN_ARRAY_H_ */
