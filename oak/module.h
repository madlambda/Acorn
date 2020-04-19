/*
 * Copyright (C) Madlambda Authors
 */


enum {
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
} SectionId;


typedef struct {
    u8          id;
    u32         size;
    u8*         data;
} Section;


typedef struct {
    File        *file;
    u32         version;
    Section     *sections;
    u16         nsections;
} Module;


Module *loadmodule(const char *filename, Error *err);
void closemodule(Module *m);
