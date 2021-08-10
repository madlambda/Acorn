/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _OAK_X64_H_
#define _OAK_X64_H_


#define MaxInsnSize 15  /* maximum x64 instruction size */


#define rex(w, r, x, b)                                                       \
    (0x40 | ((w) << 3) | ((r) << 2) | ((x) << 1) | (b))


#define modrm(mod, regop, rm)                                                 \
    (((mod) << 6)|((regop) << 3)|((rm)&7))


#define sib(scale, index, base)                                               \
    (((scale) << 6)|((index) << 3)|((base)&7))


#define regreg(a, r1, r2)                                                     \
    do {                                                                      \
        (a)->mode = RegReg;                                                   \
        (a)->src.reg = r1;                                                    \
        (a)->dst.reg = r2;                                                    \
    } while (0)


#define immreg(a, imm, r)                                                     \
    do {                                                                      \
        (a)->mode = ImmReg;                                                   \
        (a)->src.i64val = imm;                                                \
        (a)->dst.reg = r;                                                     \
    } while (0)


#define memreg(a, mem, r)                                                     \
    do {                                                                      \
        (a)->mode = MemReg;                                                   \
        (a)->src.u64val = mem;                                                \
        (a)->dst.reg = r;                                                     \
    } while (0)


#define indreg(a, displacement, r1, r2)                                       \
    do {                                                                      \
        (a)->mode = IndReg;                                                   \
        (a)->src.reg = r1;                                                    \
        (a)->src.disp = displacement;                                         \
        (a)->dst.reg = r2;                                                    \
    } while (0)


#define regmem(a, r, mem)                                                     \
    do {                                                                      \
        (a)->mode = RegMem;                                                   \
        (a)->dst.u64val = mem;                                                \
        (a)->src.reg = r;                                                     \
    } while (0)


#define regind(a, rsrc, rdst, displacement)                                   \
    do {                                                                      \
        (a)->mode = RegInd;                                                   \
        (a)->dst.reg = rdst;                                                  \
        (a)->dst.disp = displacement;                                         \
        (a)->src.reg = rsrc;                                                  \
    } while (0)


#define immind(a, imm, r, displacement)                                       \
    do {                                                                      \
        (a)->mode = ImmInd;                                                   \
        (a)->src.i64val = imm;                                                \
        (a)->dst.reg = r;                                                     \
        (a)->dst.disp = displacement;                                         \
    } while (0)


#define relreg(a, r)                                                          \
    do {                                                                      \
        (a)->mode = RelReg;                                                   \
        (a)->dst.reg = r;                                                     \
    } while (0)


#define absolute(n)                                                           \
    ((n) < 0 ? -(n) : (n))


#define checkimm(imm, min, max)                                               \
    do {                                                                      \
        if (slow((imm) > (i64)(max) || (i64)(imm) < (min))) {                 \
            return newerror("imm %d(i64) overflow range [%d(i64), %d(u64)]",  \
                            (i64)(imm), (i64)(min), (u64)(max));              \
        }                                                                     \
    } while (0)


#define checkaddr(addr, max)                                                  \
    do {                                                                      \
        if (slow((addr) > (max))) {                                           \
            return newerror("imm addr 0x%x(u64) overflow range [0, 0x%x(u64)]"\
                            ,(addr), (max));                                  \
        }                                                                     \
    } while (0)


#define checkregsizes(reg1, reg2)                                             \
    do {                                                                      \
        if (slow((regsizes[reg1] != regsizes[reg2]))) {                       \
            return newerror("reg %d and %d have different sizes", reg1, reg2);\
        }                                                                     \
    } while (0)


#define growmem(j)                                                            \
    do {                                                                      \
        if (slow(((j)->end - (j)->begin) < MaxInsnSize)) {                    \
            err = reallocrw(j);                                               \
            if (slow(err != NULL)) {                                          \
                return err;                                                   \
            }                                                                 \
        }                                                                     \
    } while (0)


#define r32(r64)  (r64 - 8)


/* Preserve the order of the enumeration below! */
typedef enum {
    AL,
    CL,
    DL,
    BL,
    AH,
    CH,
    DH,
    BH,

    AX,
    CX,
    DX,
    BX,
    SP,
    BP,
    SI,
    DI,

    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,

    RAX,    /* return register */
    RCX,
    RDX,
    RBX,    /* callee-saved */

    RSP,
    RBP,

    RSI,
    RDI,

    R8B,
    R9B,
    R10B,
    R11B,
    R12B,
    R13B,
    R14B,
    R15B,

    R8W,
    R9W,
    R10W,
    R11W,
    R12W,
    R13W,
    R14W,
    R15W,

    R8D,
    R9D,
    R10D,
    R11D,
    R12D,
    R13D,
    R14D,
    R15D,

    R8,
    R9,
    R10,
    R11,

    /* callee-saved */
    R12,
    R13,
    R14,
    R15,

#if 0
    MM0,
    MM1,
    MM2,
    MM3,
    MM4,
    MM5,
    MM6,
    MM7,

    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,
#endif

    LASTREG,
} Reg;


Error   *add(Jitfn *j, Jitvalue *operands);

Error   *movb(Jitfn *j, Jitvalue *operands);
Error   *movw(Jitfn *j, Jitvalue *operands);
Error   *movl(Jitfn *j, Jitvalue *operands);
Error   *movq(Jitfn *j, Jitvalue *operands);

Error   *lea(Jitfn *j, Jitvalue *operands);

Error   *callq(Jitfn *j, Jitvalue *operands);
Error   *nop(Jitfn *j, Jitvalue *operands);
Error   *ret(Jitfn *j, Jitvalue *operands);

Error   *prologue(Jitfn *j, Jitvalue *args);
Error   *epilogue(Jitfn *j, Jitvalue *operands);


#endif /* _OAK_X64_H_ */
