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


static TypeId  call1params[] = {I32};


static Array call1typeparams1 = {
    .nitems = 1,
    .items  = &call1params,
    .size   = sizeof(TypeId),
};


static Array call1typeparams2 = {
    .nitems = 0,
    .items  = NULL,
    .size   = sizeof(TypeId),
};


static Array call1typerets = {
    .nitems = 0,
    .items  = NULL,
    .size   = sizeof(TypeId),
};


static FuncDecl call1funcvals[] = {
    {Func, &call1typeparams1, &call1typerets},
    {Func, &call1typeparams2, &call1typerets},
};


static Array  call1sects = {
    .nitems = 5,
    .items  = &call1data,
    .size   = sizeof(Section),
};


static Array  call1funcs = {
    .nitems = 2,
    .items  = &call1funcvals,
    .size   = sizeof(FuncDecl),
};


static Module  call1mod = {
    .version = 1,
    .sects   = &call1sects,
    .funcs   = &call1funcs,
};
