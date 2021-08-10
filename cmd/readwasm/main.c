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
    u8     export;
    char   *s;
    Error  *err;

    fmtadd('e', errorfmt);
    fmtadd('o', oakfmt);

    export = 0;

    while (--argc > 0 && (++argv)[0][0] == '-') {
        for (s = argv[0] + 1; *s != '\0'; s++) {
            switch (*s) {
            case 'e':
                export = 1;
                break;
            default:
                cprint("Illegal option %c\n", *s);
                argc = 0;
                break;
            }
        }
    }

    if (slow(argc < 1)) {
        cprint("usage: readwasm [-e exportname] <filename>\n");
        return 1;
    }

    if (export) {
        err = showexport(argv[1], argv[0]);

    } else {
        err = show(argv[0]);
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
    TypeDecl    *type;
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
        type = arrayget(m.types, i);
        cprint("\t%d -> func%o(typedecl)\n", i, type);
    }

    cprint("\nImports (%d):\n", len(m.imports));

    for (i = 0; i < len(m.imports); i++) {
        import = arrayget(m.imports, i);
        cprint("\t%d -> %o(import)\n", i, import);
    }

    cprint("\nFunctions (%d):\n", len(m.funcs));

    for (i = 0; i < len(m.functypes); i++) {
        type = arrayget(m.functypes, i);
        cprint("\t%d -> %o(typedecl)\n", i, type);
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
    TypeDecl    *type;
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

    if (slow(export->kind != FunctionKind)) {
        err = newerror("export is not a function but %o(extkind)", export->kind);
        goto fail;
    }

    for (i = 0; i < len(m.functypes); i++) {
        type = arrayget(m.functypes, i);

        if (type->index == export->u.type.index) {
            cprint("found func: %o(typedecl)\n", type);

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
