/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
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

    mod = zmalloc(sizeof(Module));
    if (slow(mod == NULL)) {
        goto fail;
    }

    mod->file = file;

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
    Section s;

    while (begin < end) {
        begin = parsesect(begin, end, &s, err);
        if (slow(begin == NULL)) {
            return ERR;
        }

        m->nsect++;
        m->sect = realloc(m->sect, m->nsect * sizeof(Section));
        if (slow(m->sect == NULL)) {
            errset(err, "failed to realloc");
            return ERR;
        }

        m->sect[m->nsect - 1] = s;
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

    if (m->nsect > 0) {
        free(m->sect);
    }

    free(m);
}
