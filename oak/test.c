/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#else
#include <elf.h>
#include <link.h>
#endif

#include <oak/file.h>
#include "test.h"
#include "bin.h"


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


#if defined(__APPLE__)


Error *
readbinary(File *file, Binbuf *data)
{
    u8                         *begin, *end;
    u32                        i;
    struct load_command        *cmd;
    struct mach_header_64      *hdr;
    struct segment_command_64  *segment;

    begin = file->data;
    end = (begin + file->size);

    hdr = (struct mach_header_64*) begin;

    if (slow(hdr->magic != MH_MAGIC_64)) {
        return newerror("unexpected magic number: %x", hdr->magic);
    }

    if (slow((u32)(end - begin) < sizeof(struct mach_header_64))) {
        return newerror("corrupted MACH-O file");
    }

    begin += sizeof(struct mach_header_64);

    for (i = 0; i < hdr->ncmds; i++) {
        cmd = (struct load_command *) begin;

        if (cmd->cmd == LC_SEGMENT_64 || cmd->cmd == LC_SEGMENT) {
            segment = (struct segment_command_64 *) begin;

            if (strcmp(segment->segname, "__TEXT")) {
                data->code = (file->data + segment->fileoff);
                data->size = segment->filesize;
                return NULL;
            }
        }

        begin += cmd->cmdsize;
    }

    return newerror("code segment not found");
}


#else


Error *
readbinary(File *file, Binbuf *data)
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


#endif