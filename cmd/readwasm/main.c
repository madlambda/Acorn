/*
 * Copyright (C) Madlambda Authors
 */

#include <stdlib.h>

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>


#define fileoffset(m, s)                                                      \
    (s->data - (m)->file.data)


static Error *run(const char *filename);


int
main(int argc, char **argv)
{
    Error  *err;

    if (slow(argc < 2)) {
        cprint("usage: %s <wasm binary>\n", *argv);
        return 1;
    }

    fmtadd('e', errorfmt);
    fmtadd('o', oakfmt);

    err = run(argv[1]);
    if (slow(err != NULL)) {
        cprint("error: %e\n", err);

        errorfree(err);
        return 1;
    }

    return 0;
}


static Error *
run(const char *filename)
{
    u32         i;
    Error       *err;
    Module      m;
    Section     *s;
    FuncDecl    *f;
    ImportDecl  *import;

    err = loadmodule(&m, filename);
    if (slow(err != NULL)) {
        return err;
    }

    cprint("WASM Binary Information\n\n"
           "%o\n\n"
           "Sections (%d):\n", &m, len(m.sects));

    for (i = 0; i < len(m.sects); i++) {
        s = arrayget(m.sects, i);
        cprint("\tSection %d\n"
               "\tId: %o(sectid)\n"
               "\tOffset: %d\n"
               "\tLength: %d\n\n",
               i, s->id, fileoffset(&m, s), len(s));
    }

    cprint("Types (%d):\n", len(m.types));

    for (i = 0; i < len(m.types); i++) {
        f = arrayget(m.types, i);
        cprint("\t%d -> func%o(func)\n", i, f);
    }

    cprint("\nImports (%d):\n", len(m.imports));

    for (i = 0; i < len(m.imports); i++) {
        import = arrayget(m.imports, i);
        cprint("\t%d -> %o(import)\n", i, import);
    }

    closemodule(&m);

    return NULL;
}
