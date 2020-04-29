/*
 * Copyright (C) Madlambda Authors
 */

#include <stdlib.h>
#include <string.h>

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>


#define fileoffset(m, s)                                                      \
    (s->data - (m)->file.data)


static Error *show(const char *filename);
static Error *showexport(const char *filename, char *fn);
static ExportDecl *searchexport(Module *m, String *field);


int
main(int argc, char **argv)
{
    Error       *err;
    const char  *filename, *opt;

    if (slow(argc < 2)) {
        cprint("usage: %s <wasm binary>\n", *argv);
        return 1;
    }

    fmtadd('e', errorfmt);
    fmtadd('o', oakfmt);

    filename = argv[1];

    if (argc > 2) {
        opt = argv[2];

        if (slow(*opt != '-')) {
            cprint("invalid argument: %s\n", opt);
            return 1;
        }

        opt++;

        switch (*opt) {
        case 'e':
            if (slow(argc < 4)) {
                cprint("param -e needs a function name\n");
                return 1;
            }

            err = showexport(filename, argv[3]);
            break;

        default:
            cprint("invalid argument: %s\n", *opt);
            return 1;
        }

    } else {
        err = show(argv[1]);
    }

    if (slow(err != NULL)) {
        cprint("error: %e\n", err);

        errorfree(err);
        return 1;
    }

    return 0;
}


static Error *
show(const char *filename)
{
    u32         i;
    Error       *err;
    Module      m;
    Section     *s;
    FuncDecl    *f;
    ImportDecl  *import;
    ExportDecl  *export;

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

    cprint("\nFunctions (%d):\n", len(m.funcs));

    for (i = 0; i < len(m.funcs); i++) {
        f = arrayget(m.funcs, i);
        cprint("\t%d -> %o(func)\n", i, f);
    }

    cprint("\nExports (%d):\n", len(m.exports));

    for (i = 0; i < len(m.exports); i++) {
        export = arrayget(m.exports, i);
        cprint("\t%d -> %o(export)\n", i, export);
    }

    closemodule(&m);

    return NULL;
}


static Error *
showexport(const char *filename, char *funcname)
{
    u32         i;
    Error       *err;
    String      field;
    Module      m;
    FuncDecl    *fn;
    CodeDecl    *code;
    ExportDecl  *export;

    cstr(&field, (u8 *) funcname);

    err = loadmodule(&m, filename);
    if (slow(err != NULL)) {
        return err;
    }

    export = searchexport(&m, &field);
    if (slow(export == NULL)) {
        err = newerror("no export named \"%S\"", &field);
        goto fail;
    }

    if (slow(export->kind != Function)) {
        err = newerror("export is not a function but %o(extkind)", export->kind);
        goto fail;
    }

    for (i = 0; i < len(m.funcs); i++) {
        fn = arrayget(m.funcs, i);

        if (fn->type.index == export->u.type.index) {
            cprint("found func: %o(func)\n", fn);

            code = arrayget(m.codes, i);
            if (fast(code != NULL)) {
                cprint("found code %d\n", i);
                break;
            }
        }
    }

    err = NULL;

fail:

    closemodule(&m);
    return err;
}


static ExportDecl *
searchexport(Module *m, String *field)
{
    u32         i;
    ExportDecl  *export;

    for (i = 0; i < len(m->exports); i++) {
        export = arrayget(m->exports, i);
        if (stringcmp(export->field, field)) {
            return export;
        }
    }

    return NULL;
}
