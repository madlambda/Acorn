/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _OAK_JIT_H_
#define _OAK_JIT_H_


/* <Src><Dst> */
typedef enum {
    RegReg,
    ImmReg,     /* Immediate constant */
    MemReg,     /* Immediate direct memory */
    IndReg,     /* Indirect memory + optional displacement*/
} AddressingMode;


typedef struct {
    u8          *data;          /* machine-dependent data */
    u8          *begin;         /* next character to be encoded */
    u8          *end;
    size_t      size;
    Array       *blocks;        /* of Block */
} Jitfn;


typedef struct {
    i32         disp;           /* displacement */
    i32         scale;
    i64         i64val;
    u64         u64val;
    u8          reg;
    u8          dispreg;        /* register displacement */
} Insarg;


typedef struct {
    AddressingMode  mode;
    Insarg          src;
    Insarg          dst;
    u32             *stacksize; /* used by jit prologue/epilogue */
} Jitvalue;


typedef Error * (*Insencoder)(Jitfn *j, Jitvalue *args);


typedef struct {
    Insencoder      encoder;
    Jitvalue        args;
} Insdata;


typedef struct sBlock {
    Array           *insdata;   /* of Insdata */
    struct sBlock   *parent;
} Block;


struct sFunction {
    Funcall         fn;
    TypeDecl        sig;
    CodeDecl        *code;
    Instance        *instance;

    Jitfn           jitfn;
};


Error   *compile(Module *m);
Error   *encodeblock();
Error   *x64compile(Module *m, Function *f);
Error   *allocrw(Jitfn *j, size_t size);
Error   *reallocrw(Jitfn *j);
Error   *mkexec(Jitfn *j);
void    freejit(Jitfn *j);


#endif /* _OAK_JIT_H_ */