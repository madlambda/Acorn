/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _OAK_TEST_H_


typedef struct {
    u8      *code;
    u32     size;
} Binbuf;


typedef struct {
    Binbuf  a;
    Binbuf  b;
} Bincmp;


void    *mustalloc(size_t size);
Error   *readbinary(File *file, Binbuf *data);

u8      fmtjit(String **buf, u8 ** format, void *val);
u8      fmtjitcmp(String **buf, u8 ** format, void *val);


#endif /* _OAK_TEST_H_ */
