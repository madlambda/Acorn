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


#define abs(n)                                                                \
    ((n) < 0 ? -(n) : (n))



static Error *addregreg(Jitfn *j, Jitvalue *args);
static Error *addimmreg(Jitfn *j, Jitvalue *args);
static Error *movregreg(Jitfn *j, Jitvalue *args);
static Error *movimmreg(Jitfn *j, Jitvalue *args);
static Error *movmemreg(Jitfn *j, Jitvalue *args);
static Error *movindreg(Jitfn *j, Jitvalue *args);

static inline u8 rexfor(Reg r, u8 size);
static inline void prefixfor(u8 **begin, u8 size);


static const u8 regsizes[LASTREG] = {
    8,  8,  8,  8,  8,  8,  8,  8,  /* AL...BH */
    16, 16, 16, 16, 16, 16, 16, 16, /* AX...DI */
    32, 32, 32, 32, 32, 32, 32, 32, /* EAX...EDI */
    64, 64, 64, 64, 64, 64, 64, 64, /* RAX...RAX */
    64, 64, 64, 64, 64, 64, 64, 64, /* R8...R15 */
    8,  8,  8,  8,  8,  8,  8,  8,  /* R8D...R15D */
    16, 16, 16, 16, 16, 16, 16, 16, /* R8W...R15W */
};


static const u8 rexr[LASTREG] = {
    0, 0, 0, 0, 0, 0, 0, 0, /* AL...BH */
    0, 0, 0, 0, 0, 0, 0, 0, /* AX...DI */
    0, 0, 0, 0, 0, 0, 0, 0, /* EAX...EDI */
    0, 0, 0, 0, 0, 0, 0, 0, /* RAX...RAX */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8...R15 */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8D...R15D */
    1, 1, 1, 1, 1, 1, 1, 1, /* R8W...R15W */
};


static const u64 maxusizes[] = {
    0, UINT8_MAX, UINT16_MAX, 0, UINT32_MAX, 0, 0, 0, UINT64_MAX,
};


Error *
add(Jitfn *j, Jitvalue *operands)
{
    switch (operands->mode) {
    case RegReg:
        return addregreg(j, operands);
    case ImmReg:
        return addimmreg(j, operands);
    default:
        return newerror("addressing mode not supported: %d", operands->mode);
    }
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
    u8   dstsize, regn, opcode;
    i64  imm;

    imm = args->src.i64val;
    opcode = 2;
    dstsize = regsizes[args->dst.reg];

    checkimm(imm, INT32_MIN, INT32_MAX);

    if (dstsize == 16) {
        *j->begin++ = 0x66;
    }

    if (rexr[args->dst.reg] || dstsize == 64) {
        *j->begin++ = rex(dstsize == 64, rexr[args->dst.reg], 0, 0);
    }

    regn = args->dst.reg % 8;

    opcode |= (dstsize != 8 || rexr[args->dst.reg] != 0);

    *j->begin++ = opcode;
    *j->begin++ = modrm(0, regn, 4);
    *j->begin++ = sib(0, 4, 5);

    if (slow(u32encode(args->src.i64val, &j->begin, j->end) != OK)) {
        return newerror("failed to encode 32bits");
    }

    return NULL;
}


Error *
mov(Jitfn *j, Jitvalue *operands)
{
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
    }

    return newerror("addressing mode not supported: %d", operands->mode);
}


static Error *
movregreg(Jitfn *j, Jitvalue *args)
{
    Reg    src, dst;
    Error  *err;

    growmem(j);

    checkregsizes(args->src.reg, args->dst.reg);

    src = args->src.reg % 8;
    dst = args->dst.reg % 8;

    switch (args->src.reg) {
    case AL:
    case CL:
    case DL:
    case BL:
    case AH:
    case CH:
    case DH:
    case BH:
        *j->begin++ = 0x88;
        *j->begin++ = modrm(3, src, dst);

        break;

    case AX:
    case CX:
    case DX:
    case BX:
    case SP:
    case BP:
    case SI:
    case DI:
        *j->begin++ = 0x66;
        /* fall through */

    case EAX:
    case ECX:
    case EDX:
    case EBX:
    case ESP:
    case EBP:
    case ESI:
    case EDI:
        *j->begin++ = 0x89;
        *j->begin++ = modrm(3, src, dst);

        break;

    case RAX:
    case RCX:
    case RDX:
    case RBX:
    case RSP:
    case RBP:
    case RSI:
    case RDI:
        *j->begin++ = rex(1, 0, 0, 0);
        goto movq;
    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:
        *j->begin++ = rex(1, 1, 0, 1);

movq:

        *j->begin++ = 0x89;
        *j->begin++ = modrm(3, src, dst);

        break;

    case R8W:
    case R9W:
    case R10W:
    case R11W:
    case R12W:
    case R13W:
    case R14W:
    case R15W:
        *j->begin++ = 0x66;

        /* fall through */

    case R8D:
    case R9D:
    case R10D:
    case R11D:
    case R12D:
    case R13D:
    case R14D:
    case R15D:
        *j->begin++ = 0x45;
        *j->begin++ = 0x89;
        *j->begin++ = modrm(3, src, dst);

        break;

    default:
        return newerror("mov for register %d not implemented", args->src.reg);
    }

    return NULL;
}


static Error *
movimmreg(Jitfn *j, Jitvalue *args)
{
    Reg    regn;
    Error  *err;

    expect(args->mode == ImmReg);

    growmem(j);

    regn = args->dst.reg % 8;

    switch (args->dst.reg) {
    case AL:
    case CL:
    case DL:
    case BL:
    case AH:
    case CH:
    case DH:
    case BH:
        checkimm(args->src.i64val, INT8_MIN, INT8_MAX);

        *j->begin++ = 0xb0 + regn;
        *j->begin++ = (u8) args->src.i64val;
        break;

    case AX:
    case CX:
    case DX:
    case BX:
    case SP:
    case BP:
    case SI:
    case DI:
        checkimm(args->src.i64val, INT16_MIN, INT16_MAX);

        *j->begin++ = 0x66;
        *j->begin++ = 0xb8 + regn;

        if (slow(u16encode((u16) args->src.i64val, &j->begin, j->end) != OK)) {
            return newerror("failed to encode u16");
        }
        break;

    case EAX:
    case ECX:
    case EDX:
    case EBX:
    case ESP:
    case EBP:
    case ESI:
    case EDI:
        checkimm(args->src.i64val, INT32_MIN, INT32_MAX);

        *j->begin++ = 0xb8 + regn;

        if (slow(u32encode(args->src.i64val, &j->begin, j->end) != OK)) {
            return newerror("failed to encode mov immediate");
        }

        break;

    case RAX:
    case RCX:
    case RDX:
    case RBX:
    case RSP:
    case RBP:
    case RSI:
    case RDI:
        *j->begin++ = rex(1, 0, 0, 0);
        goto movl;

    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:
        *j->begin++ = 0x49;

movl:

        checkimm(args->src.i64val, INT64_MIN, INT64_MAX);

        if (args->src.i64val > INT32_MAX || args->src.i64val < INT32_MIN) {
            *j->begin++ = 0xb8 + regn;
            if (slow(u64encode(args->src.i64val, &j->begin, j->end) != OK)) {
                return newerror("failed to encode mov immediate");
            }

        } else {
            *j->begin++ = 0xc7;
            *j->begin++ = modrm(3, 0, regn);
            if (slow(u32encode(args->src.i64val, &j->begin, j->end) != OK)) {
                return newerror("failed to encode mov immediate");
            }
        }

        break;

    case R8D:
    case R9D:
    case R10D:
    case R11D:
    case R12D:
    case R13D:
    case R14D:
    case R15D:
        checkimm(args->src.i64val, INT8_MIN, INT8_MAX);

        *j->begin++ = 0x41;
        *j->begin++ = 0xb8 + regn;

        if (slow(u32encode(args->src.i64val, &j->begin, j->end) != OK)) {
            return newerror("failed to encode mov immediate 1");
        }

        break;

    case R8W:
    case R9W:
    case R10W:
    case R11W:
    case R12W:
    case R13W:
    case R14W:
    case R15W:
        checkimm(args->src.i64val, INT16_MIN, INT16_MAX);

        *j->begin++ = 0x66;
        *j->begin++ = 0x41;
        *j->begin++ = 0xb8 + regn;

        if (slow(u16encode(args->src.i64val, &j->begin, j->end) != OK)) {
            return newerror("failed to encode mov immediate 2");
        }

        break;

    default:
        return newerror("mov for register %d not implemented", args->dst.reg);
    }

    return NULL;
}


static Error *
movmemreg(Jitfn *j, Jitvalue *args)
{
    u8     mode;
    Reg    regn;
    Error  *err;
    Insarg  *r1, *r2;

    mode = args->mode;

    expect(mode == MemReg || mode == RegMem);

    growmem(j);

    if (mode == MemReg) {
        r1 = &args->src;
        r2 = &args->dst;

        checkaddr(r1->u64val, maxusizes[regsizes[r2->reg] / 8]);
    } else {
        r1 = &args->dst;
        r2 = &args->src;
    }

    regn = r2->reg % 8;

    switch (r2->reg) {
    case AL:
    case CL:
    case DL:
    case BL:
    case AH:
    case CH:
    case DH:
    case BH:

        if (mode == MemReg) {
            *j->begin++ = 0x8a;

        } else {
            *j->begin++ = 0x88;
        }

        break;

    case AX:
    case CX:
    case DX:
    case BX:
    case SP:
    case BP:
    case SI:
    case DI:
        *j->begin++ = 0x66;

        if (mode == MemReg) {
            *j->begin++ = 0x8b;

        } else {
            *j->begin++ = 0x89;
        }

        break;

    case RAX:
    case RCX:
    case RDX:
    case RBX:
    case RSP:
    case RBP:
    case RSI:
    case RDI:

        *j->begin++ = rex(1, 0, 0, 0);

        if (mode == MemReg && r1->u64val > INT32_MAX && r2->reg == RAX) {
            /* use movabsq */
            *j->begin++ = 0xa1 + regn;
            if (slow(u64encode(r1->u64val, &j->begin, j->end) != OK)) {
                return newerror("failed to encode mov immediate 4");
            }

            return NULL;
        }

        /* fall through */

    case EAX:
    case ECX:
    case EDX:
    case EBX:
    case ESP:
    case EBP:
    case ESI:
    case EDI:
        if (mode == MemReg) {
            *j->begin++ = 0x8b;

        } else {
            *j->begin++ = 0x89;
        }

        break;

    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:

        if (mode == MemReg) {
            checkaddr(r1->u64val, INT32_MAX);

            *j->begin++ = rex(1, 1, 0, 0);
            *j->begin++ = 0x8b;

        } else {
            *j->begin++ = rex(1, 1, 0, 0);
            *j->begin++ = 0x89;
        }

        break;

    case R8W:
    case R9W:
    case R10W:
    case R11W:
    case R12W:
    case R13W:
    case R14W:
    case R15W:
        *j->begin++ = 0x66;
        *j->begin++ = rex(0, 1, 0, 0);

        if (mode == MemReg) {
            *j->begin++ = 0x8b;

        } else {
            *j->begin++ = 0x89;
        }

        break;

    case R8D:
    case R9D:
    case R10D:
    case R11D:
    case R12D:
    case R13D:
    case R14D:
    case R15D:
        *j->begin++ = rex(0, 1, 0, 0);

        if (mode == MemReg) {
            *j->begin++ = 0x8b;

        } else {
            *j->begin++ = 0x89;
        }

        break;

    default:
        return newerror("mov for register %d not implemented", r2->reg);
    }

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

    if (slow(regsizes[args->src.reg] < 32)) {
        /*
         * TODO(i4k): check if r8d...r15d is allowed
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

    rexval = rexfor(args->dst.reg, dstsize);
    if (rexval != 0) {
        *j->begin++ = rexval;
    }

    srcn = args->src.reg % 8;
    dstn = args->dst.reg % 8;

    mod = 0;
    hassib = 0;
    nodisp = 0;

    if (args->src.disp != 0) {
        if (abs(args->src.disp) > 127) {
            mod = 2;

        } else {
            mod = 1;
        }
    }

    switch (args->src.reg) {
    case ESP:
    case RSP:
    case R12:
        hassib = 1;
        sibval = sib(0, srcn, srcn);

        if (args->src.reg == R12) {
            goto rexb;
        }

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

        if (args->src.reg == R13) {
            goto rexb;
        }

        goto movl;

    case R8:
    case R9:
    case R10:
    case R11:
    case R14:
    case R15:

        goto rexb;

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

rexb:
    *j->begin++ = rex(0, 0, 0, 1);

movl:

    *j->begin++ = 0x8b;
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


static inline u8
rexfor(Reg reg, u8 size)
{
    if (reg >= R8 && reg <= R15W) {
        return rex(size == 64, 1, 0, 0);
    }

    if (size == 64) {
        return rex(1, 0, 0, 0);
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
    }

    *begin += (p - *begin);
}
