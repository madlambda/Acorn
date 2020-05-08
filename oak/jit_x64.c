/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include <oak/instance.h>

#include <stddef.h>     /* offsetof(s, m) */
#include <sys/types.h>
#include <errno.h>
#include "opcodes.h"
#include "jit.h"
#include "bin.h"
#include "x64.h"


static Error *prologue(Jitfn *j, Jitvalue *args);
static Error *epilogue(Jitfn *j, Jitvalue *args);
static Error *xorregreg(Jitfn *j, Jitvalue *args);
static Error *funcall(Jitfn *j, Jitvalue *args);
static Error *add(Jitfn *j, Jitvalue *args);
static Error *sub(Jitfn *j, Jitvalue *args);
static Error *movregrsp(Jitfn *j, Jitvalue *args);
static Error *movimm32regdisp(Jitfn *j, Jitvalue *args);
static Error *setlocalreg(Jitfn *j, Jitvalue *args);

/* emitters push instructions onto blocks */
static Error *emitprologue(Block *b, u32 *allocsize);
static Error *emitepilogue(Block *b, u32 *restoresize);
static Error *emitmovimmreg(Block *b, i32 src, Reg dst);
static Error *emitfuncall(Block *block);
static Error *emitlookupfn(Block *b, u32 fn);
static Error *emitpreparelocals(Block *b, Array *scratch, u32 nlocals,
    u32 nrets, u32 *spsize);
static Error *emitsetlocalreg(Block *block, i32 i, Reg reg);


static const Insarg  functionAcessor = {
    .reg    = RSP,
    .disp   = 0,
};


static const Insarg  unused(localsAcessor) = {
    .reg    = RSP,
    .disp   = 8,
};


static const Insarg  unused(funcallLocalsAcessor) = {
    .reg    = RSP,
    .disp   = 16,
};


void
pushreg(Array *a, Reg r)
{
    arraypush(a, &r);
}


Reg
popreg(Array *a)
{
    Reg  *r;

    r = arraypop(a);
    return *r;
}


Error *
x64compile(Module *m, Function *fn)
{
    u8        *begin;
    i32       i32val;
    u32       i, u32val, spsize;
    Reg       reg;
    Error     *err;
    Array     *calleeregs;
    Array     *callerregs;
    Array     *usedregs;
    Jitfn     *j;
    Block     *current;
    TypeDecl  *fndecl;
    const u8  *end;

    j = &fn->jitfn;

    calleeregs = newarray(6, sizeof(Reg));
    expect(calleeregs != NULL);

    callerregs = newarray(10, sizeof(Reg));
    expect(callerregs != NULL);

    usedregs = newarray(10, sizeof(Reg));
    expect(usedregs != NULL);

    pushreg(callerregs, R15);
    pushreg(callerregs, R14);
    pushreg(callerregs, R13);
    pushreg(callerregs, R12);
    pushreg(callerregs, RBX);

    pushreg(calleeregs, R11);
    pushreg(calleeregs, R10);
    pushreg(calleeregs, RBP);
    pushreg(calleeregs, R9);
    pushreg(calleeregs, R8);
    pushreg(calleeregs, RCX);
    pushreg(calleeregs, RDX);

    begin = (u8 *) fn->code->start;
    end = (u8 *) fn->code->end;

    current = arrayget(j->blocks, 0);

    spsize = 0x10; /* fn(Function, Locals) */

    emitprologue(current, &spsize);

    while (begin < end) {
        switch (*begin) {
        case Opreturn:
            emitepilogue(current, &spsize);
            break;

        case Opi32const:
            begin++;

            if (slow(s32vdecode(&begin, end, &i32val) != OK)) {
                return newerror("failed to decode i32.const");
            }

            reg = popreg(calleeregs);
            pushreg(usedregs, reg);

            emitmovimmreg(current, i32val, reg);
            break;
        case Opcall:
            begin++;
            if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
                return newerror("failed to decode");
            }

            fndecl = arrayget(m->functypes, u32val);
            if (slow(fn == NULL)) {
                return newerror("function %d not found", u32val);
            }

            if (slow(len(usedregs) < len(fndecl->params))) {
                return newerror("function expects %d args but given %d",
                                len(fndecl->params), len(usedregs));
            }

            err = emitpreparelocals(current, calleeregs, 10, 10, &spsize);
            if (slow(err != NULL)) {
                return error(err, "preparing locals for funcall");
            }

            err = emitlookupfn(current, u32val);
            if (slow(err != NULL)) {
                return error(err, "preparing funcall");
            }

            for (i = 0; i < len(fndecl->params); i++) {
                reg = popreg(usedregs);
                pushreg(calleeregs, reg);
                emitsetlocalreg(current, i, reg);
            }

            /*
            if (nlocalitems(fn) > len(fn->sig.params)) {
                arrayzerorange(l.locals, len(fn->sig.params), nlocalitems(fn) - 1);
            }
            */

            emitfuncall(current);
            break;

        default:
            return newerror("unrecognized opcode %x", *begin);
        }
    }

    err = emitepilogue(current, &spsize);
    if (slow(err != NULL)) {
        return err;
    }

    /* GCC erroneusly forbids cast of data to func pointers with -pedantic. */
    memcpy((void *)((ptr) fn->fn), j->data, sizeof(ptr));
    return NULL;
}


static Error *
emitprologue(Block *block, u32 *allocsize)
{
    Insdata  data;

    data.encoder = prologue;
    data.args.stacksize = allocsize;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
prologue(Jitfn *j, Jitvalue *args)
{
    i32       *allocsize;
    Error     *err;
    Jitvalue  args2;

    allocsize = (i32 *) args->stacksize;

    args->src.i64val = *allocsize;

    if ((args->src.i64val % 16) != 0) {
        args->src.i64val = ((args->src.i64val / 16)+1)*16;
    }

    args->dst.reg = RSP;
    err = sub(j, args);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    memset(&args2, 0, sizeof(Jitvalue));
    args2.src.reg = RDI;
    args2.dst.disp = 0;
    err = movregrsp(j, &args2);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    args2.src.reg = RSI;
    args2.dst.disp = 8;
    err = movregrsp(j, &args2);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    args2.src.reg = RAX;
    args2.dst.reg = RAX;
    err = xorregreg(j, &args2);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    return NULL;
}


static Error *
sub(Jitfn *j, Jitvalue *args)
{
    u8     size;
    Error  *err;

    if (args->src.i64val < -128 || args->src.i64val > 127) {
        size = 7;
    } else {
        size = 4;
    }

    cprint("size: %d reg: %d\n", size, args->dst.reg);

    if (slow((j->end - j->begin) < size)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    if (size == 4) {
        memcpy(j->begin, "\x48\x83", 2);
        j->begin += 2;
        *j->begin++ = 0xe8 + args->dst.reg;
        *j->begin++ = (u8) args->src.i64val;

        return NULL;
    }

    memcpy(j->begin, "\x48\x81", 2);
    j->begin += 2;
    *j->begin++ = 0xe8 + args->dst.reg;

    if (slow(u32encode(args->src.i64val, &j->begin, j->end))) {
        return newerror("failed to encode u32");
    }

    return NULL;
}


static Error *
add(Jitfn *j, Jitvalue *args)
{
    u8     size;
    Error  *err;

    if (args->src.i64val < -128 || args->src.i64val > 127) {
        size = 7;
    } else {
        size = 4;
    }

    if (slow((j->end - j->begin) < size)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    if (size == 4) {
        memcpy(j->begin, "\x48\x83", 2);
        j->begin += 2;
        *j->begin++ = 0xc0 + (u8) args->dst.reg;
        *j->begin++ = (u8) args->src.i64val;

        return NULL;
    }

    memcpy(j->begin, "\x48\x81", 2);
    j->begin += 2;
    *j->begin++ = 0xc0 + (u8) args->dst.reg;

    if (slow(u32encode(args->src.i64val, &j->begin, j->end))) {
        return newerror("failed to encode u32");
    }

    return NULL;
}


static Error *
emitepilogue(Block *block, u32 *restoresize)
{
    Insdata  data;

    data.encoder = epilogue;
    data.args.stacksize = restoresize;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
epilogue(Jitfn *j, Jitvalue *args)
{
    u32    restorestack;
    Error  *err;

    restorestack = *args->stacksize;

    if (restorestack > 0) {
        args->dst.reg = RSP;
        args->src.i64val = restorestack;
        err = add(j, args);
        if (slow(err != NULL)) {
            return error(err, "while epilogue");
        }
    }

    if (slow((j->end - j->begin) < 1)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    memcpy(j->begin, "\xc3", 1); /* ret */
    j->begin += 1;

    return NULL;
}


static Error *
movregrsp(Jitfn *j, Jitvalue *args)
{
    u8     size;
    Error  *err;

    if (args->dst.disp != 0) {
        if (args->dst.disp < -128 || args->dst.disp > 127) {
            size = 8;
        } else {
            size = 5;
        }
    } else {
        size = 4;
    }

    if (slow((j->end - j->begin) < size)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    switch (size) {
    case 4:
        memcpy(j->begin, "\x48\x89", 2);
        j->begin += 2;
        *j->begin++ = (8 * args->src.reg) + RSP;
        *j->begin++ = 0x24;
        break;

    case 5:
        memcpy(j->begin, "\x48\x89", 2);
        j->begin += 2;
        *j->begin++ = 0x44 + (8 * args->src.reg);
        *j->begin++ = 0x24;
        *j->begin++ = (i8) args->dst.disp;
        break;
    default:
        expect(0);
    }

    return NULL;
}


static Error *
xorregreg(Jitfn *j, Jitvalue *args)
{
    Error  *err;

    if (slow((j->end - j->begin) < 3)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    *j->begin++ = 0x48;
    *j->begin++ = 0x31;
    *j->begin++ = (u8) ((args->src.reg * 8) + args->dst.reg) + 0xc0;
    return NULL;
}


static Error *
emitmovimmreg(Block *block, i32 src, Reg dst)
{
    Insdata  data;

    data.encoder = mov;
    data.args.mode = ImmReg;
    data.args.src.i64val = src;
    data.args.dst.reg = dst;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
movimm32regdisp(Jitfn *j, Jitvalue *args)
{
    *j->begin++ = 0xc7;
    *j->begin++ = 0x40 + args->dst.reg;
    *j->begin++ = (i8) args->dst.disp;
    u32encode(args->src.i64val, &j->begin, j->end);
    return NULL;
}


static Error *
movregdispreg(Jitfn *j, Jitvalue *args)
{
    u8     size;
    Error  *err;

    if (args->src.disp == 0) {
        if (args->src.reg != RSP && args->src.reg != RBP) {
            size = 3;
        } else {
            size = 4;
        }
    } else {
        if (args->src.disp < -128 || args->src.disp > 127) {
            size = 7;
        } else {
            if (args->src.reg != RSP && args->src.reg != RBP) {
                size = 4;
            } else {
                size = 5;
            }
        }
    }

    if (slow((j->end - j->begin) < size)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    *j->begin++ = 0x48;
    *j->begin++ = 0x8b;

    switch (size) {
    case 3:
        *j->begin++ = args->src.reg + (8 * args->dst.reg);
        break;
    case 4:
        if (args->src.disp == 0) {
            *j->begin++ = args->src.reg + (8 * args->dst.reg);
            *j->begin++ = 0x24;
        } else {
            *j->begin++ = 0x40 + args->src.reg + (8 * args->dst.reg);
            *j->begin++ = (i8) args->src.disp;
        }
        break;
    case 5:
        *j->begin++ = 0x40 + args->src.reg + (8 * args->dst.reg);
        *j->begin++ = 0x24;
        *j->begin++ = (i8) args->src.disp;
        break;
    case 7:
        *j->begin++ = 0x80 + args->src.reg + (8 * args->dst.reg);
        if (slow(u32encode(args->src.disp, &j->begin, j->end) != OK)) {
            return newerror("failed to encode u32");
        }
        break;
    default:
        expect(0);
    }

    return NULL;
}


static Error *
movregregdisp(Jitfn *j, Jitvalue *args)
{
    u8     size;
    Error  *err;

    if (args->dst.disp == 0) {
        if (args->dst.reg != RSP && args->dst.reg != RBP) {
            size = 3;
        } else {
            size = 4;
        }
    } else {
        if ((args->dst.disp < -128 || args->dst.disp > 127)
            && args->dst.reg != RSP && args->dst.reg != RBP) {
            size = 4;
        } else {
            size = 7;
        }
    }

    if (slow((j->end - j->begin) < size)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    *j->begin++ = 0x48;
    *j->begin++ = 0x89;

    switch (size) {
    case 3:
        *j->begin++ = args->dst.reg + (8 * args->src.reg);
        break;
    case 4:
        if (args->dst.disp == 0) {
            *j->begin++ = args->dst.reg + (8 * args->src.reg);
            *j->begin++ = 0x24;
        } else {
            *j->begin++ = 0x40 + args->dst.reg + (8 * args->src.reg);
            *j->begin++ = (i8) args->dst.disp;
        }
        break;
    case 5:
        *j->begin++ = 0x40 + args->src.reg + (8 * args->dst.reg);
        *j->begin++ = 0x24;
        *j->begin++ = (i8) args->dst.disp;
        break;
    case 7:
        *j->begin++ = 0x80 + args->src.reg + (8 * args->dst.reg);
        if (slow(u32encode(args->dst.disp, &j->begin, j->end) != OK)) {
            return newerror("failed to encode u32");
        }
        break;
    default:
        expect(0);
    }

    return NULL;
}



static Error *
emitlookupfn(Block *block, u32 fnindex)
{
    u32       disp;
    Insdata   data;

    data.args.dst.reg = RAX;
    data.args.src = functionAcessor;
    data.encoder = movregdispreg;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = RAX;
    data.args.src.reg = RAX;
    data.args.src.disp = offsetof(struct sFunction, instance);
    data.encoder = movregdispreg;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = RAX;
    data.args.src.reg = RAX;
    data.args.src.disp = offsetof(Instance, funcs);
    data.encoder = movregdispreg;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = RAX;
    data.args.src.reg = RAX;
    data.args.src.disp = offsetof(Array, items);
    data.encoder = movregdispreg;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    disp = sizeof(Function) * fnindex;

    if (disp != 0) {
        data.args.dst.reg = RAX;
        data.args.src.reg = RAX;
        data.args.src.disp = disp;
        data.encoder = movregdispreg;

        if (slow(arrayadd(block->insdata, &data) != OK)) {
            return newerror("failed to add instruction to block");
        }
    }

    data.args.mode = RegReg;
    data.args.src.reg = RAX;
    data.args.dst.reg = RDI;
    data.encoder = mov;
    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = RAX;
    data.args.src.reg = RAX;
    data.args.src.disp = offsetof(Function, fn);
    data.encoder = movregdispreg;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
emitpreparelocals(Block *block, Array * scratch, u32 nlocals, u32 nrets,
    u32 *spsize)
{
    u32      disp;
    Reg      r2;
    Insdata  data;

    *spsize += sizeof(Local) + arraytotalsize(nlocals, sizeof(Value))
                + arraytotalsize(nrets, sizeof(Value));

    r2 = popreg(scratch);

    data.args.mode = RegReg;
    data.args.dst.reg = RSI;
    data.args.src.reg = RSP;
    data.encoder = mov;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = RSI;
    data.args.src.i64val = 0x10;
    data.encoder = add;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    disp = offsetof(Local, locals);
    if (disp != 0) {
        data.args.dst.reg = r2;
        data.args.src.reg = RSI;
        data.args.src.disp = disp;
        data.encoder = movregdispreg;

        if (slow(arrayadd(block->insdata, &data) != OK)) {
            return newerror("failed to add instruction to block");
        }
    }

    data.args.mode = RegReg;
    data.args.dst.reg = r2;
    data.args.src.reg = RSI;
    data.encoder = mov;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = r2;
    data.args.src.i64val = sizeof(Local);
    data.encoder = add;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.src.reg = r2;
    data.args.dst.reg = RSI;
    data.args.dst.disp = 0;
    data.encoder = movregregdisp;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.src.i64val = 0;
    data.args.dst.reg = r2;
    data.args.dst.disp = offsetof(Array, len);
    data.encoder = movimm32regdisp;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    data.args.dst.reg = RSI;
    data.args.src.i64val = 0x10;
    data.encoder = sub;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    pushreg(scratch, r2);

    return NULL;
}


static Error *
emitsetlocalreg(Block *block, i32 i, Reg reg)
{
    Insdata  data;

    data.encoder = setlocalreg;
    data.args.src.reg = reg;
    data.args.dst.i64val = i;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
setlocalreg(Jitfn * unused(j), Jitvalue * unused(args))
{
    return NULL;
}


static Error *
emitfuncall(Block *block)
{
    Insdata  data;

    data.args.src.reg = RAX;
    data.encoder = funcall;

    if (slow(arrayadd(block->insdata, &data) != OK)) {
        return newerror("failed to add instruction to block");
    }

    return NULL;
}


static Error *
funcall(Jitfn * unused(j), Jitvalue * args)
{
    Error  *err;

    if (slow((j->end - j->begin) < 2)) {
        err = reallocrw(j);
        if (slow(err != NULL)) {
            return err;
        }
    }

    *j->begin++ = 0xff;
    *j->begin++ = 0xd0 + args->src.reg;
    return NULL;
}
