/*
 * Copyright (C) Madlambda Authors
 */


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
} SectionId;


typedef struct {
    SectionId   id;
    u32         len;
    const u8*   data;
} Section;


typedef struct {
    File        *file;
    u32         version;
    Array       *sect;      /* of Section */
} Module;


Module *loadmodule(const char *filename, Error *err);
void closemodule(Module *m);
