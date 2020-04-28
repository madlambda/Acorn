/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _ACORN_STRING_H_
#define _ACORN_STRING_H_


#define offset(p, offset)                                                     \
    ((void *) ((u8 *) (p) + (offset)))


#define stringcmp(s1, s2)                                                     \
    (s1->len == s2->len ? memcmp(s1->start, s2->start, s1->len) == 0 : 0)


/* slow :: 3*O(n) */
#define cstringcmp(s, cstr)                                                   \
    (s->len == strlen(cstr) ? memcmp(s->start, cstr, strlen(cstr)) == 0 : 0)


#define newcstring(s)                                                         \
    (newstring((const u8 *) (s), strlen(s)))


#define str(s)                                                                \
    {.start = (u8 *) s, .len = slength(s)}


#define cstr(s, str)                                                          \
    do {                                                                      \
        s->start = str;                                                       \
        s->len = strlen(str);                                                 \
    } while (0)


#define len(s)                                                                \
    ((s) != NULL ? (s)->len : (0))


#define strnalloc(s)                                                          \
    (s->nalloc)


typedef struct {
    u8      *start;
    u32     len;

    size_t  nalloc;
} String;


String  *newstring(const u8 *from, size_t size);
String  *allocstring(size_t size);
String  *append(String *, const String *s);
String  *appendc(String *, u32 nargs, ...);
String  *appendcstr(String *, const char *str);
String  *strset(String *s, const u8 *data, size_t size);


#endif /* _ACORN_STRING_H_ */
