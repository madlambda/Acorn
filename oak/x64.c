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


static Error *addregreg(Jitfn *j, Jitvalue *args);
static Error *addimmreg(Jitfn *j, Jitvalue *args);
static Error *addmemreg(Jitfn *j, Jitvalue *args);

static Error *mov(Jitfn *j, Jitvalue *operands);
static Error *movregreg(Jitfn *j, Jitvalue *args);
static Error *movimmreg(Jitfn *j, Jitvalue *args);
static Error *movmemreg(Jitfn *j, Jitvalue *args);
static Error *movindreg(Jitfn *j, Jitvalue *args);
static Error *movimmind(Jitfn *j, Jitvalue *args);


static inline u8 rexfor(Reg src, Reg dst, u8 srcsize, u8 dstsize);
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
    switch (operands->size) {
    case 8:
    case 16:
    case 32:
    case 64:
        break;
    default:
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
        return movindreg(j, operands);
    case ImmInd:
        return movimmind(j, operands);
    }

    return newerror("addressing mode not supported: %d", operands->mode);
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
    u32  size;

    size = 0;
    operands->stacksize = &size;
    return epilogue(j, operands);
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
movindreg(Jitfn *j, Jitvalue *args)
{
    u8     srcsize, dstsize, rexval, sibval, mod, hassib, nodisp;
    u8     defopsize, defaddrsize;
    Reg    srcn, dstn;
    Error  *err;

    expect(args->mode == IndReg);

    if (slow(regsizes[args->src.reg] < 16)) {
        /*
         * TODO(i4k): check if r8b...r15d is allowed
         */
        return newerror("reg %d is not a valid base/index expression",
                        args->src.reg);
    }

    growmem(j);

    srcsize = regsizes[args->src.reg];
    dstsize = regsizes[args->dst.reg];

    expect(srcsize <= 64 && dstsize <= 64);

    defopsize = 32;
    defaddrsize = 64;

    if (srcsize < defaddrsize) {
        prefixfor(&j->begin, srcsize);
    }

    if (dstsize != defopsize) {
        prefixfor(&j->begin, dstsize);
    }

    rexval = rexfor(args->src.reg, args->dst.reg, defaddrsize, defopsize);
    if (rexval != 0) {
        *j->begin++ = rexval;
    }

    srcn = args->src.reg % 8;
    dstn = args->dst.reg % 8;

    mod = 0;
    hassib = 0;
    nodisp = 0;

    if (args->src.disp != 0) {
        if (absolute(args->src.disp) > 127) {
            mod = 2;

        } else {
            mod = 1;
        }
    }

    switch (args->src.reg) {
    case ESP:
    case RSP:
    case R12:
    case R12D:
        hassib = 1;
        sibval = sib(0, srcn, srcn);

        goto movl;

    case EBP:
    case RBP:
    case R13:
        hassib = 1;
        if (mod == 1) {
            sibval = sib(0, args->src.disp / 8, args->src.disp % 8);
            nodisp = 1;
        } else {
            sibval = sib(0, 0, 0);
        }
        if (!mod) {
            mod = 1;
        }

        goto movl;

    case R8B:
    case R8D:
    case R9D:
    case R10D:
    case R11D:
    case R13D:
    case R14D:
    case R15D:
    case R8:
    case R9:
    case R10:
    case R11:
    case R14:
    case R15:
    case RAX:
    case RCX:
    case RDX:
    case RBX:
    case RSI:
    case RDI:
    case EAX:
    case ECX:
    case EDX:
    case EBX:
    case ESI:
    case EDI:
        goto movl;

    default:
        return newerror("register %d not implemented", args->src.reg);
    }

    expect(0);

movl:

    *j->begin++ = 0x8a | (dstsize != 8);
    *j->begin++ = modrm(mod, dstn, srcn);

    if (hassib) {
        *j->begin++ = sibval;
    }

    if (!nodisp && args->src.disp != 0) {
        if (mod == 1) {
            *j->begin++ = args->src.disp;
        } else {
            if (slow(u32encode(args->src.disp, &j->begin, j->end) != OK)) {
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

    defaddrsize = 64;

    if (dstsize != defaddrsize) {
        prefixfor(&j->begin, dstsize);
    }

    rexval = rexfor(args->dst.reg, 0, 64, 32);
    if (rexval != 0) {
        *j->begin++ = rexval;
    }

    regn = dst->reg % 8;
    lim = &limits[args->size / 8];

    checkimm(src->i64val, lim->min, lim->max);

    if (slow(src->i64val > INT32_MAX)) {
        return newerror("use movabs");
    }

    if (dstsize == 16) {
        *j->begin++ = 0x66;
    }

    hassib = 0;
    mod = 0;

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
        mod = 1;
        sibval = sib(0, 0, 0);
        hassib = 1;
        break;
    }

    *j->begin++ = 0xc6 | (args->size != 8);
    *j->begin++ = modrm(mod, 0, regn);

    if (hassib) {
        *j->begin++ = sibval;
    }

    if (slow(uencode(src->i64val, args->size, &j->begin, j->end) != OK)) {
        return newerror("failed to encode %dbit immediate", args->size);
    }

    return NULL;
}


static inline u8
rexfor(Reg src, Reg dst, u8 defsrcsize, u8 defdstsize)
{
    u8  size, val;

    size = 0;

    if (regsizes[src] == 64 && defsrcsize != 64) {
        size = 1;
    }

    if (regsizes[dst] == 64 && defdstsize != 64) {
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
