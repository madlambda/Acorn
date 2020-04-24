/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "array.h"
#include "error.h"
#include "file.h"


u8 *mmapfile(int fd, size_t size, int prot, int flags);


File *
openfile(const char *filename, Error *err)
{
    int          fd;
    File         *file;
    struct stat  st;

    fd = open(filename, O_RDONLY, 0);
    if (slow(fd < 0)) {
        error(err, "failed to open \"%s\": %E", filename, errno);
        return NULL;
    }

    if (slow(fstat(fd, &st) < 0)) {
        goto fail;
    }

    file = zmalloc(sizeof(File));
    if (slow(file == NULL)) {
        goto fail;
    }

    file->fd = fd;
    file->size = st.st_size;

    file->data = mmapfile(fd, file->size, PROT_READ, MAP_PRIVATE);
    if (slow(file->data == NULL)) {
        goto fail;
    }

    return file;

fail:

    error(err, "failed to open file: %E", errno);
    close(fd);

    return NULL;
}


void
closefile(File *f)
{
    expect(f->fd != -1);
    expect(f->data != NULL);

    munmap(f->data, f->size);
    close(f->fd);
    free(f);
}


u8 *
mmapfile(int fd, size_t size, int prot, int flags)
{
    return mmap(NULL, size, prot, flags, fd, 0);
}
