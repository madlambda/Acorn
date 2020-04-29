/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <oak/file.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


u8 *mmapfile(int fd, size_t size, int prot, int flags);


Error *
openfile(File *file, const char *filename)
{
    Error        *err;
    struct stat  st;

    file->fd = open(filename, O_RDONLY, 0);
    if (slow(file->fd < 0)) {
        return newerror("failed to open \"%s\": %s", filename, strerror(errno));
    }

    if (slow(fstat(file->fd, &st) < 0)) {
        err = newerror("failed to stat file: %s", strerror(errno));
        goto fail;
    }

    file->size = st.st_size;

    file->data = mmapfile(file->fd, file->size, PROT_READ, MAP_PRIVATE);
    if (slow(file->data == NULL)) {
        err = newerror("failed to mmap file: %s", strerror(errno));
        goto fail;
    }

    return NULL;

fail:

    close(file->fd);

    return err;
}


void
closefile(File *f)
{
    expect(f->fd != -1);
    expect(f->data != NULL);

    munmap(f->data, f->size);
    close(f->fd);
}


u8 *
mmapfile(int fd, size_t size, int prot, int flags)
{
    return mmap(NULL, size, prot, flags, fd, 0);
}
