/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include <oak/instance.h>
#include "jit.h"
#include "x64.h"
#include <unistd.h>
#include "bin.h"


typedef struct {
    i64     min;
    u64     max;
} Limit;


typedef enum {
    Mov = 0,
    Lea = 4,
} Kindmove;


static Error *addregreg(Jitfn *j, Jitvalue *args);
static Error *addimmreg(Jitfn *j, Jitvalue *args);
static Error *addmemreg(Jitfn *j, Jitvalue *args);

static Error *mov(Jitfn *j, Jitvalue *operands);
static Error *movregreg(Jitfn *j, Jitvalue *args);
static Error *movimmreg(Jitfn *j, Jitvalue *args);
static Error *movmemreg(Jitfn *j, Jitvalue *args);
static Error *movindreg(Jitfn *j, Jitvalue *args, Kindmove kind);
static Error *movimmind(Jitfn *j, Jitvalue *args);
static Error *xorregreg(Jitfn *j, Jitvalue *args);
static Error *callrelreg(Jitfn *j, Jitvalue *args);


static inline u8 rexfor(Reg src, Reg dst, u8 srcsize, u8 dstsize,
    u8 defsrc, u8 defdst);
static inline void prefixfor(u8 **begin, u8 size);


static const u8  regsizes[LASTREG] = {
    8,  8,  8,  8,  8,  8,  8,  8,  /* AL...BH */
    16, 16, 16, 16, 16, 16, 16, 16, /* AX...DI */
    32, 32, 32, 32, 32, 32, 32, 32, /* EAX...EDI */
    64, 64, 64, 64, 64, 64, 64, 64, /* RAX...RAX */
    8,  8,  8,  8,  8,  8,  8,  8,  /* R8B...R15B */
    16, 16, 16, 16, 16, 16, 16, 16, /* R8W...R15W */
    32, 32, 32, 32, 32, 32, 32, 32, /* R8D...R15D */
    64, 64, 64, 64, 64, 64, 64, 64, /* R8...R15 */
};


static const Limit  limits[9] = {
    {.min = 0, .max = 0},
    {.min = INT8_MIN,  .max = INT8_MAX},
    {.min = INT16_MIN, .max = INT16_MAX},
    {.min = 0, .max = 0},
    {.min = INT32_MIN, .max = INT32_MAX},
    {.min = 0, .max = 0},
    {.min = 0, .max = 0},
    {.min = 0, .max = 0},
    {.min = INT64_MIN, .max = INT64_MAX},
};


static const u8  rexr[LASTREG] = {
    0, 0, 0, 0, 0, 0, 0, 0, /* AL...BH */
    0, 0, 0, 0, 0, 0, 0, 0, /* AX...DI */
    0, 0, 0, 0, 0, 0, 0, 0, /* EAX...EDI */
    0, 0, 0, 0, 0, 0, 0, 0, /* RAX...RAX */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8B...R15B */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8W...R15W */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8D...R15D */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8...R15 */
};


Error *
add(Jitfn *j, Jitvalue *operands)
{
    switch (operands->mode) {
    case RegReg:
        return addregreg(j, operands);
    case MemReg:
        return addmemreg(j, operands);
    case ImmReg:
        return addimmreg(j, operands);
    default:
        return newerror("addressing mode not supported: %d", operands->mode);
    }
}


static Error *
mov(Jitfn *j, Jitvalue *operands)
{
    if (slow(operands->size % 8 != 0 && operands->size > 64)) {
        return newerror("invalid instruction size: %d", operands->size);
    }

    switch(operands->mode) {
    case RegReg:
        return movregreg(j, operands);
    case ImmReg:
        return movimmreg(j, operands);
    case MemReg:
    case RegMem:
        return movmemreg(j, operands);
    case IndReg:
    case RegInd:
        return movindreg(j, operands, Mov);
    case ImmInd:
        return movimmind(j, operands);
    default:
        return newerror("addressing mode not supported: %d", operands->mode);
    }
}


Error *
lea(Jitfn *j, Jitvalue *operands)
{
    switch (operands->mode) {
    case IndReg:
        return movindreg(j, operands, Lea);
    default:
        return newerror("addressing mode not supported: %d", operands->mode);
    }
}


Error *
callq(Jitfn *j, Jitvalue *operands)
{
    operands->size = 64;

    switch (operands->mode) {
    case RelReg:
        return callrelreg(j, operands);
    default:
        return newerror("addressing mode not supported: %d", operands->mode);
    }
}


static Error *
callrelreg(Jitfn *j, Jitvalue *args)
{
    Error  *err;

    growmem(j);

    *j->begin++ = 0xff;
    *j->begin++ = modrm(3, 2, args->dst.reg);
    return NULL;
}


Error *
movb(Jitfn *j, Jitvalue *args)
{
    args->size = 8;
    return mov(j, args);
}


Error *
movw(Jitfn *j, Jitvalue *args)
{
    args->size = 16;
    return mov(j, args);
}


Error *
movl(Jitfn *j, Jitvalue *args)
{
    args->size = 32;
    return mov(j, args);
}


Error *
movq(Jitfn *j, Jitvalue *args)
{
    args->size = 64;
    return mov(j, args);
}


Error *
ret(Jitfn *j, Jitvalue *operands)
{
    i64  size;

    size = 0;
    operands->stacksize = &size;
    return epilogue(j, operands);
}


Error *
prologue(Jitfn *j, Jitvalue *args)
{
    i64       *allocsize;
    Error     *err;

    allocsize = (i64 *) args->stacksize;

    args->src.i64val = *allocsize;

    if ((args->src.i64val % 16) != 0) {
        *allocsize = ((args->src.i64val / 16)+1)*16;
    }

    immreg(args, -(*allocsize), RSP);
    err = add(j, args);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    regind(args, RDI, RSP, 0);
    err = movq(j, args);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    regind(args, RSI, RSP, 8);
    err = movq(j, args);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    regreg(args, RAX, RAX);
    err = xorregreg(j, args);
    if (slow(err != NULL)) {
        return error(err, "encoding prologue");
    }

    return NULL;
}


Error *
epilogue(Jitfn *j, Jitvalue *args)
{
    u32    restorestack;
    Error  *err;

    restorestack = *args->stacksize;

    if (restorestack > 0) {
        immreg(args, restorestack, RSP);
        err = add(j, args);
        if (slow(err != NULL)) {
            return error(err, "while encoding epilogue");
        }
    }

    growmem(j);

    *j->begin++ = 0xc3;

    return NULL;
}


static Error *
addregreg(Jitfn *j, Jitvalue *args)
{
    u8      srcsize, dstsize, srcrexr, dstrexr, opcode;
    Error   *err;
    Insarg  *src, *dst;

    growmem(j);

    src = &args->src;
    dst = &args->dst;

    srcsize = regsizes[src->reg];
    dstsize = regsizes[dst->reg];
    srcrexr = rexr[src->reg];
    dstrexr = rexr[dst->reg];

    if (slow(srcsize != dstsize)) {
        return newerror("add: register sizes mismatch (%d: %d) (%d: %d)",
                        src->reg, srcsize, dst->reg, dstsize);
    }

    if (srcsize == 16) {
        *j->begin++ = 0x66;
    }

    opcode = (srcsize != 8);

    if (srcrexr || dstrexr || srcsize == 64) {
        *j->begin++ = rex(srcsize == 64, srcrexr, 0, dstrexr);
    }

    *j->begin++ = opcode;
    *j->begin++ = modrm(3, args->src.reg, args->dst.reg);

    return NULL;
}


static Error *
addimmreg(Jitfn *j, Jitvalue *args)
{
    u8           nbits;
    i64          imm;
    Reg          regn;
    Error        *err;
    Insarg       *dst;
    const Limit  *lim;

    growmem(j);

    dst = &args->dst;
    regn = dst->reg % 8;

    nbits = regsizes[dst->reg];
    imm = args->src.i64val;
    lim = &limits[nbits / 8];

    checkimm(imm, lim->min, lim->max);

    if (nbits == 8) {
        *j->begin++ = 0x04 + regn;
        if (slow(uencode(imm, nbits, &j->begin, j->end) != OK)) {
            return newerror("failed to encode");
        }

        return NULL;
    }

    if (nbits == 16) {
        *j->begin++ = 0x66;
    }

    if (nbits == 64 || rexr[dst->reg]) {
        *j->begin++ = rex(nbits == 64, 0, 0, rexr[dst->reg]);
    }

    if (imm >= INT8_MIN && imm <= INT8_MAX) {
        *j->begin++ = 0x83;
        nbits = 8;

    } else {
        *j->begin++ = 0x81;
        nbits = 32;
    }

    *j->begin++ = modrm(3, 0, regn);

    if (slow(uencode(imm, nbits, &j->begin, j->end) != OK)) {
        return newerror("failed to encode");
    }

    return NULL;
}


static Error *
addmemreg(Jitfn *j, Jitvalue *args)
{
    u8   dstsize, regn, opcode;
    u64  imm;

    imm = args->src.u64val;
    opcode = 2;
    dstsize = regsizes[args->dst.reg];

    checkaddr(imm, INT32_MAX);

    if (dstsize == 16) {
        *j->begin++ = 0x66;
    }

    if (rexr[args->dst.reg] || dstsize == 64) {
        *j->begin++ = rex(dstsize == 64, rexr[args->dst.reg], 0, 0);
    }

    regn = args->dst.reg % 8;

    opcode |= (dstsize != 8);

    *j->begin++ = opcode;
    *j->begin++ = modrm(0, regn, 4);
    *j->begin++ = sib(0, 4, 5);

    if (slow(u32encode(imm, &j->begin, j->end) != OK)) {
        return newerror("failed to encode 32bits");
    }

    return NULL;
}


static Error *
movregreg(Jitfn *j, Jitvalue *args)
{
    u8      srcsize;
    Reg     srcn, dstn;
    Error   *err;
    Insarg  *src, *dst;

    growmem(j);

    checkregsizes(args->src.reg, args->dst.reg);

    if (slow(regsizes[args->src.reg] != args->size)) {
        return newerror("instruction size mismatch: used %dbit instruction "
                        "with %dbit register", args->size,
                        regsizes[args->src.reg]);
    }

    src = &args->src;
    dst = &args->dst;
    srcn = src->reg % 8;
    dstn = dst->reg % 8;

    srcsize = regsizes[src->reg];

    if (srcsize == 16) {
        *j->begin++ = 0x66;
    }

    if (srcsize == 64 || rexr[src->reg] || rexr[dst->reg]) {
        *j->begin++ = rex(srcsize == 64, rexr[src->reg], 0, rexr[dst->reg]);
    }

    *j->begin++ = 0x88 | (srcsize != 8);
    *j->begin++ = modrm(3, srcn, dstn);

    return NULL;
}


static Error *
movimmreg(Jitfn *j, Jitvalue *args)
{
    u8           dstsize;
    Reg          regn;
    Error        *err;
    const Limit  *lim;

    expect(args->mode == ImmReg);

    growmem(j);

    regn = args->dst.reg % 8;
    dstsize = regsizes[args->dst.reg];

    lim = &limits[dstsize / 8];
    checkimm(args->src.i64val, lim->min, lim->max);

    if (dstsize == 16) {
        *j->begin++ = 0x66;
    }

    if (dstsize == 64 || rexr[args->dst.reg]) {
        *j->begin++ = rex(dstsize == 64, 0, 0, rexr[args->dst.reg]);
    }

    *j->begin++ = (0xb0 + regn) | ((dstsize != 8) << 3);

    if (slow(uencode(args->src.i64val, dstsize, &j->begin, j->end) != OK)) {
        return newerror("failed to encode");
    }

    return NULL;
}


static Error *
movmemreg(Jitfn *j, Jitvalue *args)
{
    u8     mode, dstsize;
    Reg    regn;
    Error  *err;
    Insarg  *r1, *r2;

    mode = args->mode;

    expect(mode == MemReg || mode == RegMem);

    growmem(j);

    if (mode == MemReg) {
        r1 = &args->src;
        r2 = &args->dst;

        checkaddr((u64) r1->u64val, UINT32_MAX);

    } else {
        r1 = &args->dst;
        r2 = &args->src;
    }

    regn = r2->reg % 8;
    dstsize = regsizes[r2->reg];

    if (dstsize == 16) {
        *j->begin++ = 0x66;
    }

    if (dstsize == 64 || rexr[r2->reg]) {
        *j->begin++ = rex(dstsize == 64, rexr[r2->reg], 0, 0);
    }

    *j->begin++ = 0x88 | (mode == MemReg) << 1 | (dstsize != 8);
    *j->begin++ = modrm(0, regn, 4);
    *j->begin++ = sib(0, 4, 5);

    if (slow(u32encode(r1->u64val, &j->begin, j->end) != OK)) {
        return newerror("failed to encode mov immediate");
    }

    return NULL;
}


static Error *
movindreg(Jitfn *j, Jitvalue *args, Kindmove kind)
{
    u8      srcsize, dstsize, rexval, sibval, mod, hassib, nodisp;
    u8      defopsize, defaddrsize;
    Reg     srcn, dstn;
    Error   *err;
    Insarg  *src, *dst;

    expect(args->mode == IndReg || args->mode == RegInd);

    if (args->mode == IndReg) {
        src = &args->src;
        dst = &args->dst;

    } else {
        src = &args->dst;
        dst = &args->src;

        if (slow(kind == Lea)) {
            return newerror("mode not supported for lea");
        }
    }

    if (slow(regsizes[src->reg] < 16)) {
        return newerror("reg %d is not a valid base/index expression",
                        args->src.reg);
    }

    growmem(j);

    srcsize = regsizes[src->reg];
    dstsize = regsizes[dst->reg];

    expect(srcsize <= 64 && dstsize <= 64);

    defopsize = 32;
    defaddrsize = 64;

    if (srcsize < defaddrsize) {
        prefixfor(&j->begin, srcsize);
    }

    if (dstsize != defopsize) {
        prefixfor(&j->begin, dstsize);
    }

    rexval = rexfor(src->reg, dst->reg, srcsize, dstsize, defaddrsize, defopsize);
    if (rexval != 0) {
        *j->begin++ = rexval;
    }

    srcn = src->reg % 8;
    dstn = dst->reg % 8;

    mod = 0;
    hassib = 0;
    nodisp = 0;

    if (src->disp != 0) {
        if (absolute(src->disp) > 127) {
            mod = 2;

        } else {
            mod = 1;
        }
    }

    switch (src->reg) {
    case ESP:
    case RSP:
    case R12:
    case R12D:
        hassib = 1;
        sibval = sib(0, srcn, srcn);

        break;

    case EBP:
    case RBP:
    case R13:
        if (mod == 1) {
            hassib = 1;
            sibval = sib(0, src->disp / 8, src->disp % 8);
            nodisp = 1;
        } else if (args->mode == IndReg && mod != 2) {
            sibval = sib(0, 0, 0);
            hassib = 1;
        }
        if (!mod) {
            mod = 1;
        }

        break;
    }

    if (kind == Mov) {
        *j->begin++ = 0x88 | ((args->mode == IndReg) << 1) | (dstsize != 8);

    } else {
        *j->begin++ = (kind + 0x88) | (dstsize != 8);
    }

    *j->begin++ = modrm(mod, dstn, srcn);

    if (hassib) {
        *j->begin++ = sibval;
    }

    if (!nodisp && src->disp != 0) {
        if (mod == 1) {
            *j->begin++ = src->disp;
        } else {
            if (slow(u32encode(src->disp, &j->begin, j->end) != OK)) {
                return newerror("failed to encode displacement");
            }
        }
    }

    return NULL;
}


static Error *
movimmind(Jitfn *j, Jitvalue *args)
{
    u8            regn, dstsize, defaddrsize, rexval, mod, sibval, hassib;
    u8            nodisp;
    Error         *err;
    Insarg        *src, *dst;
    const Limit   *lim;

    expect(args->mode == ImmInd);

    if (slow(regsizes[args->dst.reg] < 32)) {
        return newerror("reg %d (%dbit) is not a valid base/index expression",
                        args->dst.reg, regsizes[args->dst.reg]);
    }

    growmem(j);

    src = &args->src;
    dst = &args->dst;
    dstsize = regsizes[dst->reg];

    expect(dstsize <= 64);

    lim = &limits[args->size / 8];

    checkimm(src->i64val, lim->min, lim->max);

    if (slow(src->i64val > INT32_MAX)) {
        return newerror("use movabs");
    }

    defaddrsize = 64;

    if (dstsize != defaddrsize) {
        prefixfor(&j->begin, dstsize);
    }

    rexval = rexfor(args->dst.reg, 0, args->size, 64, 32, 64);
    if (rexval != 0) {
        *j->begin++ = rexval;
    } else if (args->size == 64) {
        *j->begin++ = rex(1, 0, 0, 0);
    }

    regn = dst->reg % 8;

    if (dstsize == 16) {
        *j->begin++ = 0x66;
    }

    nodisp = 0;
    hassib = 0;
    mod = 0;

    if (dst->disp != 0) {
        if (absolute(dst->disp) > 127) {
            mod = 2;

        } else {
            mod = 1;
        }
    }

    switch (dst->reg) {
    case ESP:
    case RSP:
    case R12:
    case R12B:
    case R12W:
    case R12D:
        sibval = sib(0, regn, regn);
        hassib = 1;
        break;
    case EBP:
    case RBP:
    case R13:
    case R13B:
    case R13W:
    case R13D:
        if (mod == 1) {
            hassib = 1;
            sibval = sib(0, dst->disp / 8, dst->disp % 8);
            nodisp = 1;
        } else if (mod != 2) {
            sibval = sib(0, 0, 0);
            hassib = 1;
        }
        if (!mod) {
            mod = 1;
        }
    }

    *j->begin++ = 0xc6 | (args->size != 8);
    *j->begin++ = modrm(mod, 0, regn);

    if (hassib) {
        *j->begin++ = sibval;
    }

    if (!nodisp && dst->disp != 0) {
        if (mod == 1) {
            *j->begin++ = dst->disp;
        } else {
            if (slow(u32encode(dst->disp, &j->begin, j->end) != OK)) {
                return newerror("failed to encode displacement");
            }
        }
    }

    if (slow(uencode(src->i64val, args->size > 32 ? 32 : args->size,
                     &j->begin, j->end) != OK))
    {
        return newerror("failed to encode %dbit immediate", args->size);
    }

    return NULL;
}


static inline u8
rexfor(Reg src, Reg dst, u8 srcsize, u8 dstsize, u8 defsrcsize, u8 defdstsize)
{
    u8  size, val;

    size = 0;

    if (srcsize == 64 && defsrcsize != 64) {
        size = 1;
    }

    if (dstsize == 64 && defdstsize != 64) {
        size = 1;
    }

    val = rex(size, rexr[dst], 0, rexr[src]);
    if (val != 0x40) {
        return val;
    }

    return 0;
}


static inline void
prefixfor(u8 **begin, u8 size)
{
    u8  *p;

    p = *begin;
    switch (size) {
    case 16:
        *p++ = 0x66;
        break;
    case 32:
        *p++ = 0x67;
        break;
    default:
        return;
    }

    *begin += 1;
}


static Error *
xorregreg(Jitfn *j, Jitvalue * unused(args))
{
    Error  *err;

    growmem(j);

    *j->begin++ = 0x48;
    *j->begin++ = 0x31;
    *j->begin++ = 0xc0;

    return NULL;
}
