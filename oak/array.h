/*
 * Copyright (C) Madlambda Authors.
 */


#ifndef _OAK_ARRAY_H_
#define _OAK_ARRAY_H_


#define arraylast(array)                                                      \
    offset((array)->items, (array)->size * ((array)->len - 1))


#define arraysize(array)                                                      \
    (array->size * array->nalloc)


typedef struct {
    u32     len;
    u32     nalloc;
    size_t  size;
    void    *items;
} Array;


Array   *newarray(u32 nitems, size_t size);
void    freearray(Array *);
u8      arrayadd(Array *, void *val);
void    *arrayget(Array *, u32 index);
u8      arrayset(Array *, u32 index, void *val);
void    arraydel(Array *, u32 index);
Array   *shrinkarray(Array *old);

#endif /* _OAK_ARRAY_H_ */
