static const u8 globalconstdata[] = {
    0x01, 0x7f, 0x00, 0x41, 0x2a, 0x0b,
};


static const u8 globalexportdata[] = {
    0x01, 0x06, 0x61, 0x6e, 0x73, 0x77, 0x65, 0x72, 0x03, 0x00,
};


static Section  globalconstsectvals[] = {
    {
        .id     = 6,
        .len    = 6,
        .data   = globalconstdata,
    },
    {
        .id     = 7,
        .len    = 10,
        .data   = globalexportdata,
    },
};


static Array  globalconstsects = {
    .len    = 2,
    .items  = globalconstsectvals,
    .size   = sizeof(Section),
};


GlobalDecl  globalconstglobalvals[] = {
    {
        .type   = {
            .type = I32,
            .mut = 0,
        },
        .u = {
            .i32val = 42,
        },
    },
};


static Array  globalconstglobals = {
    .len    = 1,
    .items  = globalconstglobalvals,
    .size   = sizeof(GlobalDecl),
};


static String  globalconststring = {
    .len    = 6,
    .start  = (u8 *) "answer",
};


static ExportDecl  globalconstexportvals[] = {
    {
        .field  = &globalconststring,
        .kind   = Global,
        .u.global = {
            .type   = {
                .type = I32,
                .mut = 0,
            },
            .u = {
                .i32val = 42,
            },
        },
    },
};


static Array  globalconstexports = {
    .len    = 1,
    .items  = globalconstexportvals,
    .size   = sizeof(ExportDecl),
};


static Module  globalconstmod = {
    .version    = 1,
    .sects      = &globalconstsects,
    .globals    = &globalconstglobals,
    .exports    = &globalconstexports,
};
