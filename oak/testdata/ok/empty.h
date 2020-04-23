
static Array  emptyarray = {
    .len     = 0,
    .items   = NULL,
    .size    = sizeof(Section),
};


static Module  emptymod = {
    .version = 11,
    .sects   = &emptyarray,
    .funcs   = NULL,
};
