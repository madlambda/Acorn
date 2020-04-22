/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _OAK_FILE_H_
#define _OAK_FILE_H_


typedef struct {
    int     fd;
    u8      *data;
    size_t  size;
} File;


File *openfile(const char *filename, Error *err);
void closefile(File *file);

#endif