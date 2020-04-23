/*
 * Expected data structure for call1.wasm
 */

static const u8 call1typedata[] = {
    0x2, 0x60, 0x1, 0x7f, 0x0, 0x60, 0x0, 0x0
};


static const u8 call1impodata[] = {
    0x01, 0x07, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74,
    0x73, 0x0d, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74,
    0x65, 0x64, 0x5f, 0x66, 0x75, 0x6e, 0x63, 0x00,
    0x00
};


static const u8 call1funcdata[] = {0x01, 0x01};


static const u8 call1expodata[] = {
    0x01, 0x0d, 0x65, 0x78, 0x70, 0x6f, 0x72, 0x74,
    0x65, 0x64, 0x5f, 0x66, 0x75, 0x6e, 0x63, 0x00,
    0x01,
};


static const u8 call1codedata[] = {
    0x01, 0x06, 0x00, 0x41, 0x2a, 0x10, 0x00, 0x0b,
};


static Section call1data[] = {
    {1, 0x8, call1typedata},
    {2, 0x19, call1impodata},
    {3, 2, call1funcdata},
    {7, 0x11, call1expodata},
    {10, 8, call1codedata},
};


static Type  call1params[] = {I32};


static Array call1typeparams1 = {
    .len        = 1,
    .items      = &call1params,
    .size       = sizeof(Type),
};


static Array call1typeparams2 = {
    .len        = 0,
    .items      = NULL,
    .size       = sizeof(Type),
};


static Array call1typerets = {
    .len        = 0,
    .items      = NULL,
    .size       = sizeof(Type),
};


static FuncDecl call1typevals[] = {
    {Func, &call1typeparams1, &call1typerets},
    {Func, &call1typeparams2, &call1typerets},
};


static FuncDecl call1funcvals[] = {
    {Func, &call1typeparams2, &call1typerets},
};


static Array  call1sects = {
    .len        = 5,
    .items      = &call1data,
    .size       = sizeof(Section),
};


static Array  call1types = {
    .len        = 2,
    .items      = &call1typevals,
    .size       = sizeof(FuncDecl),
};


static String  call1importmodname = {
    .len        = 7,
    .start      = (u8 *) "imports",
};


static String  call1importfieldname = {
    .len        = 13,
    .start      = (u8 *) "imported_func",
};


static String  call1exportfieldname = {
    .len        = 13,
    .start      = (u8 *) "exported_func",
};


static ImportDecl  call1importvals = {
    .module     = &call1importmodname,
    .field      = &call1importfieldname,
    .kind       = Function,
    .u          = {
        .function = {Func, &call1typeparams1, &call1typerets},
    },
};


static Array  call1imports = {
    .len        = 1,
    .items      = &call1importvals,
    .size       = sizeof(ImportDecl),
};


static Array  call1funcs = {
    .len        = 1,
    .items      = &call1funcvals,
    .size       = sizeof(FuncDecl),
};


static ExportDecl  call1exportvals[] = {
    {
        .field  = &call1exportfieldname,
        .kind   = Function,
        .index  = 1,
    },
};


static Array  call1exports = {
    .len        = 1,
    .items      = &call1exportvals,
    .size       = sizeof(ExportDecl),
};


static Array  call1locals = {
    .len        = 0,
    .items      = NULL,
    .size       = sizeof(LocalEntry),
};


static CodeDecl  call1codevals[] = {
    {
        .locals = &call1locals,
        .start  = (call1codedata + 3), /* skip over count,bodysize,localcount */
        .end    = (call1codedata + (sizeof(call1codedata) - 1)),
    },
};

static Array  call1codes = {
    .len        = 1,
    .items      = &call1codevals,
    .size       = sizeof(CodeDecl),
};


static Module  call1mod = {
    .version    = 1,
    .sects      = &call1sects,
    .types      = &call1types,
    .imports    = &call1imports,
    .funcs      = &call1funcs,
    .tables     = NULL,
    .memories   = NULL,
    .globals    = NULL,
    .exports    = &call1exports,
    .start      = 0,
    .codes      = &call1codes,
};
