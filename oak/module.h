/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _OAK_MODULE_H_
#define _OAK_MODULE_H_

typedef enum {
    Custom = 0,
    Type,
    Import,
    Function,
    Table,
    Memory,
    Global,
    Export,
    Start,
    Element,
    Code,
    Data,
    LastSectionId,
} SectionId;


typedef struct {
    SectionId   id;
    u32         len;
    const u8*   data;
} Section;


typedef enum {
    I32         = 0x01,
    I64,
    F32,
    F64,
    Anyfunc     = 0x10,
    Func        = 0x20,
    Emptyblock  = 0x40,
} TypeId;


typedef enum {
    Function = 0,
    Table,
    Memory,
    Global,
} ExternalKind;


typedef struct {
    TypeId      form;
    Array       *params;    /* of TypeId */
    Array       *rets;      /* of TypeId */
} FuncDecl;


typedef struct {
    String        module;
    String        field;
    ExternalKind  kind;
} ImportDecl;


typedef struct {
    File        *file;
    u32         version;
    Array       *sects;     /* of Section */
    Array       *funcs;     /* of FuncDecl */
    Array       *imports;   /* of ImportDecl */
} Module;


Module *loadmodule(const char *filename, Error *err);
void closemodule(Module *m);

#endif /* _OAK_MODULE_H_ */