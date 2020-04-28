/*
 * Copyright (C) Madlambda Authors
 */

#ifndef _OAK_MODULE_H_
#define _OAK_MODULE_H_


#include "file.h"


typedef enum {
    CustomId = 0,
    TypeId,
    ImportId,
    FunctionId,
    TableId,
    MemoryId,
    GlobalId,
    ExportId,
    StartId,
    ElementId,
    CodeId,
    DataId,
    LastSectionId,
} SectionId;


typedef struct {
    SectionId       id;
    u32             len;
    const u8*       data;
} Section;


typedef enum {
    I32             = 0x01,
    I64,
    F32,
    F64,
    Anyfunc         = 0x10,
    Func            = 0x20,
    Emptyblock      = 0x40,
} Type;


typedef enum {
    Function = 0,
    Table,
    Memory,
    Global,
} ExternalKind;


typedef struct {
    u32             flags;      /* 1-bit in MVP */
    u32             initial;
    u32             maximum;
} ResizableLimit;


typedef struct {
    Type            form;
    Array           *params;    /* of Type */
    Array           *rets;      /* of Type */
} FuncDecl;


typedef struct {
    String          *module;
    String          *field;
    ExternalKind    kind;

    union {
        FuncDecl    function;
    } u;
} ImportDecl;


typedef struct {
    String          *field;
    ExternalKind    kind;
    u32             index;      /* index into the kind index space */
} ExportDecl;


typedef struct {
    Type            type;
    ResizableLimit  limit;
} TableDecl;


typedef struct {
    ResizableLimit  limit;
} MemoryDecl;


typedef struct {
    Type            type;
    u8              mut;        /* 1-bit - mutability */
} GlobalType;


typedef struct {
    GlobalType      type;
    union {
        u32         globalindex;
        i32         i32val;
        i64         i64val;
        u32         f32val;
        u64         f64val;
    } u;
} GlobalDecl;


typedef struct {
    u32             count;
    Type            type;
} LocalEntry;


typedef struct {
    Array           *locals;     /* of LocalEntry */
    const u8        *start;      /* pointer to file offset */
    const u8        *end;
} CodeDecl;


typedef struct {
    u32             index;
    i32             offset;
    u32             size;
    const u8        *data;
} DataDecl;


typedef struct {
    File            file;
    u32             version;
    u32             start;      /* function index */
    Array           *sects;     /* of Section */
    Array           *types;     /* of FuncDecl */
    Array           *imports;   /* of ImportDecl */
    Array           *funcs;     /* of FuncDecl */
    Array           *tables;    /* of TableDecl */
    Array           *memories;  /* of MemoryDecl */
    Array           *globals;   /* of GlobalDecl */
    Array           *exports;   /* of ExportDecl */
    Array           *codes;     /* of CodeDecl */
    Array           *datas;     /* of DataDecl */
} Module;


Error   *loadmodule(Module *, const char *filename);
void    closemodule(Module *m);

u8      oakfmt(String **buf, u8 **format, void *val);

#endif /* _OAK_MODULE_H_ */
