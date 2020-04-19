/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "module.h"


Module *
loadmodule(const char *filename, Error *err)
{
    File    *file;
    Module  *mod;

    file = openfile(filename, err);
    if (slow(file == NULL)) {
        return NULL;
    }

    if (slow(file->size < 8)) {
        errset(err, "WASM must have at least 8 bytes");
        closefile(file);
        return NULL;
    }

    if (slow(memcmp(file->data, "\0asm", 4) != 0)) {
        errset(err, "file is not a WASM module");
        closefile(file);
        return NULL;
    }

    mod = malloc(sizeof(Module));
    if (slow(mod == NULL)) {
        errset(err, strerror(errno));
        return NULL;
    }

    mod->file = file;

    mod->version = (u32) file->data[0];
    mod->version |= ((u32) file->data[1]) << 8;
    mod->version |= ((u32) file->data[2]) << 16;
    mod->version |= ((u32) file->data[3]) << 24;

    return mod;
}


void
closemodule(Module *m)
{
    closefile(m->file);

    free(m);
}
