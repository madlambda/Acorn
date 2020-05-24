/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _OAK_OPCODES_H_
#define _OAK_OPCODES_H_


typedef enum {
    Opreturn        = 0x0f,
    Opcall          = 0x10,
    Opgetlocal      = 0x20,
    Opgetglobal     = 0x23,

    Opi32const      = 0x41,
    Opi64const,
    Opf32const,
    Opf64const,

    Opi32add        = 0x6a,
} Opcode;


#endif
