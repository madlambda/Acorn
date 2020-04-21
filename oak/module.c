/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "array.h"
#include "module.h"
#include "bin.h"


u8 parsesects(Module *m, u8 *begin, const u8 *end, Error *err);
u8 *parsesect(u8 *begin, const u8 *end, Section *s, Error *err);


Module *
loadmodule(const char *filename, Error *err)
{
    u8      *begin, *end;
    File    *file;
    Module  *mod;

    err->msg = NULL;

    file = openfile(filename, err);
    if (slow(file == NULL)) {
        return NULL;
    }

    if (slow(file->size < 8)) {
        errset(err, "WASM must have at least 8 bytes");
        goto fail;
    }

    begin = file->data;
    end = file->data + file->size;

    if (slow(memcmp(begin, "\0asm", 4) != 0)) {
        errset(err, "file is not a WASM module");
        goto fail;
    }

    begin += 4;

    mod = malloc(sizeof(Module));
    if (slow(mod == NULL)) {
        goto fail;
    }

    mod->file = file;

    mod->sect = newarray(16, sizeof(Section));
    if (slow(mod->sect == NULL)) {
        goto free;
    }

    u32decode(begin, &mod->version);
    begin += 4;

    if (slow(parsesects(mod, begin, end, err) != OK)) {
        goto free;
    }

    return mod;

free:

    free(mod);

fail:

    closefile(file);
    return NULL;
}


u8
parsesects(Module *m, u8 *begin, const u8 *end, Error *err)
{
    Section  sect;

    while (begin < end) {
        begin = parsesect(begin, end, &sect, err);
        if (slow(begin == NULL)) {
            return ERR;
        }

        if (slow(arrayadd(m->sect, &sect) != OK)) {
            errset(err, "failed to add section");
            return ERR;
        }
    }

    return OK;
}


u8 *
parsesect(u8 *begin, const u8 *end, Section *s, Error *err)
{
    u64      val;
    ssize_t  read;

    if (slow(udecode(begin, end, &val) != 1)) {
        errset(err, "malformed section code");
        return NULL;
    }

    begin++;

    s->id = (u8) val;

    read = udecode(begin, end, &val);
    if (slow(read < 0 || read > 4)) {
        errset(err, "malformed section len");
        return NULL;
    }

    s->len = (u32) val;
    begin += read;

    s->data = begin;

    return begin + s->len;
}


void
closemodule(Module *m)
{
    closefile(m->file);
    freearray(m->sect);
    free(m);
}
