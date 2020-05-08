/*
 * Copyright (C) Madlambda Authors.
 */
#define _GNU_SOURCE
#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include <oak/instance.h>

#include "jit.h"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>


#if defined(__APPLE__)
#define DEFAULTSIZE 4096
#else
#define DEFAULTSIZE 1024
#endif


static Error *initblock(Block *block, u32 nitems);
static Error *encodeblocks(Function *fn);


Error *
compile(Module *m)
{
    u32       i;
    Error     *err;
    Block     block;
    TypeDecl  *type;
    Function  fn;

    m->funcs = newarray(len(m->codes), sizeof(Function));
    if (slow(m->funcs == NULL)) {
        return newerror("failed to alloc array: %s", strerror(errno));
    }

    for (i = 0; i < len(m->codes); i++) {
        fn.code = arrayget(m->codes, i);
        type = arrayget(m->functypes, i);
        fn.sig = *type;
        fn.instance = NULL;

        fn.jitfn.blocks = newarray(10, sizeof(Block));
        if (slow(fn.jitfn.blocks == NULL)) {
            return newerror("failed to alloc blocks array");
        }

        initblock(&block, 10);
        arrayadd(fn.jitfn.blocks, &block);

        err = allocrw(&fn.jitfn, DEFAULTSIZE);
        if (slow(err != NULL)) {
            return newerror("compiling: %e", err);
        }

        fn.jitfn.begin = fn.jitfn.data;
        fn.jitfn.end = (fn.jitfn.data + fn.jitfn.size);

        err = x64compile(m, &fn);
        if (fast(err == NULL)) {
            err = encodeblocks(&fn);
            if (slow(err != NULL)) {
                cprint("failed to encode instructions: %e", err);
                fn.fn = interp;
            }

        } else {
            cprint("failed to jit function (%o(typedecl)): %e\n", type, err);
            fn.fn = interp;
        }

        if (slow(arrayadd(m->funcs, &fn) != OK)) {
            return newerror("failed to add function");
        }
    }

    return NULL;
}


static Error *
initblock(Block *block, u32 nitems)
{
    block->insdata = newarray(nitems, sizeof(Insdata));
    if (slow(block->insdata == NULL)) {
        return newerror("failed to alloc insdata array");
    }

    block->parent = NULL;
    return NULL;
}


static Error *
encodeblocks(Function *fn)
{
    u32    i;
    Jitfn  *jit;
    Error  *err;
    Block  *block;

    jit = &fn->jitfn;

    for (i = 0; i < len(jit->blocks); i++) {
        block = arrayget(jit->blocks, i);
        err = encodeblock(jit, block);
        if (slow(err != NULL)) {
            return error(err, "encoding block %d", i);
        }
    }

    return NULL;
}


Error *
encodeblock(Jitfn *jit, Block *block)
{
    u32      i;
    Error    *err;
    Insdata  *data;

    for (i = 0; i < len(block->insdata); i++) {
        data = arrayget(block->insdata, i);

        err = data->encoder(jit, &data->args);
        if (slow(err != NULL)) {
            return error(err, "encoding insn %d", i);
        }
    }

    return NULL;
}


Error *
allocrw(Jitfn *j, size_t size)
{
    j->data = mmap(0, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (slow(j->data == MAP_FAILED)) {
        return newerror("mmap(0, %d): %s", DEFAULTSIZE, strerror(errno));
    }

    j->size = DEFAULTSIZE;

    return NULL;
}


void
freejit(Jitfn *j)
{
    munmap(j->data, j->size);
}


Error *
reallocrw(Jitfn *j)
{
    u32  offset;

    offset = (j->begin - j->data);

#if defined(__APPLE__)

    u8      *oldptr;
    size_t  oldsize;
    Error   *err;

    oldptr = j->data;
    oldsize = j->size;

    err = allocrw(j, j->size + DEFAULTSIZE);
    if (slow(err != NULL)) {
        return error(err, "failed to realloc(%d)", j->size + DEFAULTSIZE);
    }

    memcpy(j->data, oldptr, oldsize);

    munmap(oldptr, oldsize);

#else

    j->data = mremap(j->data, j->size, j->size + DEFAULTSIZE, MREMAP_MAYMOVE);
    if (slow(j->data == MAP_FAILED)) {
        return newerror("mremap(%d): %s", j->size + DEFAULTSIZE, strerror(errno));
    }

    j->size = j->size + DEFAULTSIZE;

#endif

    j->begin = (j->data + offset);
    j->end = (j->data + j->size);

    return NULL;
}


Error *
mkexec(Jitfn *fn)
{
    if (slow(mprotect(fn->data, fn->size, PROT_READ | PROT_EXEC) == -1)) {
        return newerror("mprotect(%x, %d): %s", fn->data, fn->size,
                        strerror(errno));
    }

    return NULL;
}
