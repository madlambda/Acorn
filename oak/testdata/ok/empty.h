
static Array  emptyarray = {
    .nitems = 0,
    .items  = NULL,
    .size   = sizeof(Section),
};


static Module  emptymod = {
    .version = 11,
    .sects   = &emptyarray,
    .funcs   = NULL,
};
