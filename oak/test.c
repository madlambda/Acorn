/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <elf.h>
#include <link.h>

#include <oak/file.h>
#include "test.h"


/*
 * These functions must be used in tests.
 */


void *
mustalloc(size_t size)
{
    void  *ptr;

    ptr = malloc(size);
    if (slow(ptr == NULL)) {
        cprint("malloc(%d): %e", size, strerror(errno));
        exit(1);
    }

    return ptr;
}


u8
fmtjit(String **buf, u8 ** format, void *val)
{
    u32      i;
    ptr      p;
    Binbuf  *code;

    code = (Binbuf *) val;

    for (i = 0; i < code->size; i++) {
        p = code->code[i];
        if (hexfmt(buf, format, (void *) p) != OK) {
            return ERR;
        }
        *buf = appendc(*buf, 1, ' ');
    }

    return OK;
}


u8
fmtjitcmp(String **buf, u8 ** format, void *ptrval)
{
    u8      val;
    ptr     p;
    u32     i;
    Bincmp  *data;

    data = (Bincmp *) ptrval;

    if (slow(data->a.size != data->b.size)) {
        *buf = appendcstr(*buf, "size mismatch");
        return *buf != NULL ? OK : ERR;
    }

    for (i = 0; i < data->a.size; i++) {
        val = data->a.code[i];
        p = val;
        if (hexfmt(buf, format, (void *) p) != OK) {
            return ERR;
        }
        *buf = appendc(*buf, 1, ' ');

        if (val != data->b.code[i]) {
            *buf = appendcstr(*buf, "<- differ here (want ");
            if (fast(*buf != NULL)) {
                p = data->b.code[i];
                if (hexfmt(buf, format, (void *) p) != OK) {
                    return ERR;
                }

                *buf = appendc(*buf, 1, ')');
            }
            break;
        }
    }

    return *buf != NULL ? OK : ERR;
}


Error *
readelfcode(File *file, Binbuf *data)
{
    u32         i;
    ElfW(Ehdr)  *ehdr;
    ElfW(Shdr)  *shdr;

    ehdr = (ElfW(Ehdr) *) file->data;

    if (slow(ehdr->e_shnum == 0)) {
        return newerror("no section headers found in jit output");
    }

    shdr = (ElfW(Shdr) *) (file->data + ehdr->e_shoff);

    data->code = NULL;
    data->size = 0;

    for (i = 0; i < ehdr->e_shnum; i++) {
        shdr = (shdr + i);

        if (shdr->sh_type == SHT_PROGBITS) {
            data->code = (file->data + shdr->sh_offset);
            data->size = shdr->sh_size;
            break;
        }
    }

    if (slow(data->code == NULL)) {
        return newerror("no code section found in jit output");
    }

    return NULL;
}
