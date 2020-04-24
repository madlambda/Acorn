/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "array.h"
#include "error.h"
#include "file.h"
#include "string.h"
#include "opcodes.h"
#include "module.h"
#include "bin.h"


typedef Error *(*Parser)(Module *m, u8 *begin, const u8 *end);


/* section parsers */
static Error *parsesects(Module *m, u8 *begin, const u8 *end);
static Error *parsesect(u8 **begin, const u8 *end, Section *s);
static Error *parsecustoms(Module *m, u8 *begin, const u8 *end);
static Error *parsetypes(Module *m, u8 *begin, const u8 *end);
static Error *parseimports(Module *m, u8 *begin, const u8 *end);
static Error *parsefunctions(Module *m, u8 *begin, const u8 *end);
static Error *parsetables(Module *m, u8 *begin, const u8 *end);
static Error *parsememories(Module *m, u8 *begin, const u8 *end);
static Error *parseglobals(Module *m, u8 *begin, const u8 *end);
static Error *parseexports(Module *m, u8 *begin, const u8 *end);
static Error *parsestarts(Module *m, u8 *begin, const u8 *end);
static Error *parseelements(Module *m, u8 *begin, const u8 *end);
static Error *parsecodes(Module *m, u8 *begin, const u8 *end);
static Error *parsedatas(Module *m, u8 *begin, const u8 *end);

/* helpers */
static Error *parselimits(u8 **begin, const u8 *end, ResizableLimit *limit);


static const Parser  parsers[] = {
    parsecustoms,
    parsetypes,
    parseimports,
    parsefunctions,
    parsetables,
    parsememories,
    parseglobals,
    parseexports,
    parsestarts,
    parseelements,
    parsecodes,
    parsedatas,
};


static const char  *Edupsect        = "section \"%s\" duplicated";
static const char  *Estralloc       = "failed to allocate string";
static const char  *Earrayadd       = "failed to add item to array: %s";
static const char  *Eallocarray     = "failed to allocate array: %s";
static const char  *Ecorruptsect    = "section \"%s\" is corrupted";


#define edupsect(name)      newerror(Edupsect, name)
#define estralloc()         newerror(Estralloc)
#define earrayadd()         newerror(Earrayadd, strerror(errno))
#define earrayalloc()       newerror(Eallocarray, strerror(errno))
#define ecorruptsect(name)  newerror(Ecorruptsect, name)
#define emalformed(f,s)     newerror("malformed \"%s\" of \"%s\" section", f, s);


static const char  *typesect     = "type";
static const char  *importsect   = "import";
static const char  *functionsect = "function";
static const char  *tablesect    = "table";
static const char  *memorysect   = "memory";
static const char  *globalsect   = "global";
static const char  *exportsect   = "export";
static const char  *codesect     = "code";
static const char  *datasect     = "data";


Error *
loadmodule(Module *mod, const char *filename)
{
    u8      *begin, *end;
    File    file;
    Error   *err;

    err = openfile(&file, filename);
    if (slow(err != NULL)) {
        return error(err, "loading module");
    }

    if (slow(file.size < 8)) {
        err = newerror("WASM must have at least 8 bytes");
        goto fail;
    }

    begin = file.data;
    end = file.data + file.size;

    if (slow(memcmp(begin, "\0asm", 4) != 0)) {
        err = newerror("file is not a WASM module");
        goto fail;
    }

    begin += 4;

    memset(mod, 0, sizeof(Module));

    mod->file = file;

    mod->sects = newarray(16, sizeof(Section));
    if (slow(mod->sects == NULL)) {
        err = newerror("failed to create sects array: %s", strerror(errno));
        goto free;
    }

    u32decode(&begin, end, &mod->version);

    err = parsesects(mod, begin, end);
    if (slow(err != NULL)) {
        err = error(err, "loading module");
        goto free;
    }

    return NULL;

free:

    free(mod);

fail:

    closefile(&file);
    return err;
}


static Error *
parsesects(Module *m, u8 *begin, const u8 *end)
{
    Error    *err;
    Parser   parser;
    Section  sect;

    while (begin < end) {
        memset(&sect, 0, sizeof(Section));

        err = parsesect(&begin, end, &sect);
        if (slow(err != NULL)) {
            return error(err, "failed to parse sect");
        }

        if (slow(arrayadd(m->sects, &sect) != OK)) {
            return error(err, "failed to add section");
        }

        if (slow(sect.id >= LastSectionId)) {
            return error(err, "invalid section id: %d", sect.id);
        }

        parser = parsers[sect.id];

        err = parser(m, (u8 *) sect.data, sect.data + sect.len);
        if (slow(err != NULL)) {
            return err;
        }
    }

    return NULL;
}


static Error *
parsesect(u8 **begin, const u8 *end, Section *s)
{
    if (slow(u8vdecode(begin, end, (u8 *) &s->id) != OK)) {
        return newerror("malformed section id");
    }

    if (slow(u32vdecode(begin, end, &s->len) != OK)) {
        return newerror("malformed section len");
    }

    s->data = *begin;
    *begin += s->len;
    return NULL;
}


static Error *
parsetypes(Module *m, u8 *begin, const u8 *end)
{
    i8        i8val;
    u8        u8val;
    u32       found, count, paramcount, retcount, u32val;
    u64       i;
    Type      value;
    FuncDecl  func;

    if (slow(m->types != NULL)) {
        return edupsect(typesect);
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        return ecorruptsect(typesect);
    }

    found = 0;

    m->types = newarray(count, sizeof(FuncDecl));
    if (slow(m->types == NULL)) {
        return earrayalloc();
    }

    while (begin < end && found < count) {
        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            return emalformed("form", typesect);
        }

        memset(&func, 0, sizeof(FuncDecl));

        func.form = (Type) -i8val;

        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            return emalformed("param_count", typesect);
        }

        paramcount = u32val;

        func.params = newarray(paramcount, sizeof(Type));
        if (slow(func.params == NULL)) {
            return earrayalloc();
        }

        for (i = 0; i < paramcount; i++) {
            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                return emalformed("param type", typesect);
            }

            value = -i8val;

            if (slow(arrayadd(func.params, &value) != OK)) {
                return earrayadd();
            }
        }

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            return emalformed("return count", typesect);
        }

        retcount = u8val;

        func.rets = newarray(retcount, sizeof(Type));
        if (slow(func.rets == NULL)) {
            return earrayalloc();
        }

        for (i = 0; i < retcount; i++) {
            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                return emalformed("ret type", typesect);
            }

            value = -i8val;

            if (slow(arrayadd(func.params, &value) != OK)) {
                return earrayadd();
            }
        }

        if (slow(arrayadd(m->types, &func) != OK)) {
            return earrayadd();
        }

        found++;
    }

    expect(begin <= end);

    if (slow(found < count)) {
        return ecorruptsect(typesect);
    }

    return NULL;
}


static Error *
parseimports(Module *m, u8 *begin, const u8 *end)
{
    u8          u8val;
    u32         i, u32val, nimports;
    FuncDecl    *f;
    ImportDecl  import;

    if (slow(m->imports != NULL)) {
        return edupsect(importsect);
    }

    if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
        return ecorruptsect(importsect);
    }

    nimports = u32val;

    m->imports = newarray(nimports, sizeof(ImportDecl));
    if (slow(m->imports == NULL)) {
        return earrayalloc();
    }

    for (i = 0; i < nimports; i++) {
        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            return ecorruptsect(importsect);
        }

        import.module = allocstring(u32val);
        if (slow(import.module == NULL)) {
            return estralloc();
        }

        strset(import.module, begin, u32val);

        begin += u32val;

        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            return ecorruptsect(importsect);
        }

        import.field = allocstring(u32val);
        if (slow(import.field == NULL)) {
            return estralloc();
        }

        strset(import.field, begin, u32val);

        begin += u32val;

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            return ecorruptsect(importsect);
        }

        import.kind = u8val;

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            return ecorruptsect(importsect);
        }

        switch (import.kind) {
        case Function:
            f = arrayget(m->types, u8val);
            if (slow(f == NULL)) {
                return newerror("import section references unknown function");
            }

            import.u.function = *f;
            break;

        default:
            return newerror("external kind not supported yet");
        }

        if (slow(arrayadd(m->imports, &import) != OK)) {
            return earrayadd();
        }
    }

    return NULL;
}


static Error *
parsefunctions(Module *m, u8 *begin, const u8 *end)
{
    u32       count, uval;
    FuncDecl  *f;

    if (slow(m->funcs != NULL)) {
        return edupsect(functionsect);
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect(functionsect);
    }

    m->funcs = newarray(count, sizeof(FuncDecl));
    if (slow(m->funcs == NULL)) {
        return earrayalloc();
    }

    while (len(m->funcs) < count && begin < end) {
        if (slow(u32vdecode(&begin, end, &uval))) {
            return ecorruptsect(functionsect);
        }

        f = arrayget(m->types, uval);
        if (slow(f == NULL)) {
            return newerror("type \"%d\" not found", uval);
        }

        if (slow(arrayadd(m->funcs, f) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->funcs) < count)) {
        return ecorruptsect(functionsect);
    }

    return NULL;
}


static Error *
parsetables(Module *m, u8 *begin, const u8 *end)
{
    i8         i8val;
    u32        count;
    Error      *err;
    TableDecl  tbl;

    if (slow(m->tables != NULL)) {
        return edupsect(tablesect);
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect(tablesect);
    }

    m->tables = newarray(count, sizeof(TableDecl));
    if (slow(m->tables == NULL)) {
        return earrayalloc();
    }

    while (len(m->tables) < count && begin < end) {
        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            return ecorruptsect(tablesect);
        }

        memset(&tbl, 0, sizeof(TableDecl));

        tbl.type = (Type) -i8val;

        err = parselimits(&begin, end, &tbl.limit);
        if (slow(err != NULL)) {
            return error(err, "parsing table section");
        }

        if (slow(arrayadd(m->tables, &tbl) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->tables) < count)) {
        return ecorruptsect(tablesect);
    }

    return NULL;
}


static Error *
parsememories(Module *m, u8 *begin, const u8 *end)
{
    u32         count;
    Error       *err;
    MemoryDecl  mem;

    if (slow(m->memories != NULL)) {
        return ecorruptsect(memorysect);
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect(memorysect);
    }

    m->memories = newarray(count, sizeof(MemoryDecl));
    if (slow(m->memories == NULL)) {
        return earrayalloc();
    }

    while (len(m->memories) < count && begin < end) {
        memset(&mem, 0, sizeof(MemoryDecl));

        err = parselimits(&begin, end, &mem.limit);
        if (slow(err != NULL)) {
            return error(err, "parsing memory section");
        }

        if (slow(arrayadd(m->memories, &mem) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->memories) < count)) {
        return ecorruptsect(memorysect);
    }

    return NULL;
}


static Error *
parseglobals(Module *m, u8 *begin, const u8 *end)
{
    i8          i8val;
    u8          opcode;
    u32         count;
    GlobalDecl  global;

    if (slow(m->globals != NULL)) {
        return ecorruptsect(globalsect);
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect(globalsect);
    }

    m->globals = newarray(count, sizeof(GlobalDecl));
    if (slow(m->globals == NULL)) {
        return earrayalloc();
    }

    while (len(m->globals) < count && begin < end) {
        memset(&global, 0, sizeof(GlobalDecl));

        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            return emalformed("type", globalsect);
        }

        global.type.type = (Type) -i8val;

        if (slow(u8vdecode(&begin, end, &global.type.mut) != OK)) {
            return emalformed("mutability", globalsect);
        }

        opcode = *begin++;

        switch (opcode) {
        case OpgetGlobal:
            if (slow(u32vdecode(&begin, end, &global.u.globalindex) != OK)) {
                return emalformed("get_global", globalsect);
            }

            break;

        case Opi32const:
            if (slow(s32vdecode(&begin, end, &global.u.i32val) != OK)) {
                return emalformed("i32.const", globalsect);
            }

            break;

        case Opi64const:
            if (slow(s64vdecode(&begin, end, &global.u.i64val) != OK)) {
                return emalformed("i64.const", globalsect);
            }

            break;

        case Opf32const:
            if (slow(u32decode(&begin, end, &global.u.f32val) != OK)) {
                return emalformed("f32.const", globalsect);
            }

            break;

        case Opf64const:
            if (slow(u32decode(&begin, end, &global.u.f32val) != OK)) {
                emalformed("f64.const", globalsect);
            }

            break;

        default:
            return newerror("unsupported global expression");
        }

        if (slow(arrayadd(m->globals, &global) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->globals) < count)) {
        return ecorruptsect(globalsect);
    }

    return NULL;
}


static Error *
parseexports(Module *m, u8 *begin, const u8 *end)
{
    u32         count, uval;
    ExportDecl  export;

    if (slow(m->exports != NULL)) {
        return edupsect(exportsect);
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect(exportsect);
    }

    m->exports = newarray(count, sizeof(ExportDecl));
    if (slow(m->exports == NULL)) {
        return earrayalloc();
    }

    while (len(m->exports) < count && begin < end) {
        if (slow(u32vdecode(&begin, end, &uval) != OK)) {
            return emalformed("field_len", exportsect);
        }

        memset(&export, 0, sizeof(ExportDecl));

        export.field = allocstring(uval);
        if (slow(export.field == NULL)) {
            return estralloc();
        }

        strset(export.field, begin, uval);

        begin += uval;

        export.kind = (u8) *begin++;

        if (slow(u32vdecode(&begin, end, &export.index) != OK)) {
            return emalformed("index", exportsect);
        }

        if (slow(arrayadd(m->exports, &export) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->exports) < count)) {
        return ecorruptsect(exportsect);
    }

    return NULL;
}


static Error *
parsestarts(Module *m, u8 *begin, const u8 *end)
{
    if (slow(u32vdecode(&begin, end, &m->start) != OK)) {
        return newerror("failed to parse start function");
    }

    return NULL;
}


static Error *
parsecodes(Module *m, u8 *begin, const u8 *end)
{
    i8          i8val;
    u8          *bodybegin, *bodyend;
    u32         count, bodysize, localcount;
    CodeDecl    code;
    LocalEntry  local;

    if (slow(m->codes != NULL)) {
        return edupsect(codesect);
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        return ecorruptsect(codesect);
    }

    m->codes = newarray(count, sizeof(CodeDecl));
    if (slow(m->codes == NULL)) {
        return estralloc();
    }

    bodyend = 0;

    while (len(m->codes) < count && begin < end) {
        bodybegin = begin;

        if (slow(u32vdecode(&begin, end, &bodysize) != OK)) {
            return emalformed("function body size", codesect);
        }

        bodyend = (bodybegin + bodysize);

        if (slow(*bodyend != 0x0b)) {
            return newerror("malformed body: missing 0x0b in the end");
        }

        if (slow(u32vdecode(&begin, end, &localcount) != OK)) {
            return emalformed("number of locals", codesect);
        }

        memset(&code, 0, sizeof(CodeDecl));

        code.locals = newarray(localcount, sizeof(LocalEntry));
        if (slow(code.locals == NULL)) {
            return earrayalloc();
        }

        while (len(code.locals) < localcount && begin < end) {
            memset(&local, 0, sizeof(LocalEntry));

            if (slow(u32vdecode(&begin, end, &local.count) != OK)) {
                return emalformed("local_count", codesect);
            }

            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                return emalformed("local type", codesect);
            }

            local.type = (Type) -i8val;

            if (slow(arrayadd(code.locals, &local) != OK)) {
                return earrayadd();
            }
        }

        if (slow(len(code.locals) < localcount)) {
            return ecorruptsect(codesect);
        }

        code.start = begin;
        code.end = bodyend;

        if (slow(arrayadd(m->codes, &code) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->codes) < count)) {
        return ecorruptsect(codesect);
    }

    if (slow(count > 0 && (end - bodyend) != 1)) {
        return newerror("surplus bytes in the end of code section");
    }

    return NULL;
}


static Error *
parselimits(u8 **begin, const u8 *end, ResizableLimit *limit)
{
    u8  u8val;

    if (slow(u8vdecode(begin, end, &u8val) != OK)) {
        return newerror("malformed limits. Is it really WASM MVP binary?");
    }

    limit->flags = u8val;

    if (slow(u32vdecode(begin, end, &limit->initial) != OK)) {
        return ecorruptsect("limits");
    }

    if (slow((limit->flags & 1) &&
             u32vdecode(begin, end, &limit->maximum) != OK))
    {
        return ecorruptsect("limits");
    }

    return NULL;
}


static Error *
parsedatas(Module *m, u8 *begin, const u8 *end)
{
    u8        opcode;
    u32       count;
    DataDecl  data;

    if (slow(m->datas != NULL)) {
        return edupsect(datasect);
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        return ecorruptsect(datasect);
    }

    m->datas = newarray(count, sizeof(DataDecl));
    if (slow(m->datas == NULL)) {
        return earrayalloc();
    }

    while (len(m->datas) < count && begin < end) {
        memset(&data, 0, sizeof(DataDecl));

        if (slow(u32vdecode(&begin, end, &data.index) != OK)) {
            return emalformed("index", datasect);
        }

        opcode = *begin++;

        switch (opcode) {
        case Opi32const:
            if (slow(s32vdecode(&begin, end, &data.offset) != OK)) {
                return emalformed("i32.const", datasect);
            }

            break;

        default:
            return newerror("unsupported data init expression");
        }

        if (slow(u32vdecode(&begin, end, &data.size) != OK)) {
            return emalformed("size", datasect);
        }

        data.data = begin;

        if (slow(arrayadd(m->datas, &data) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->datas) < count)) {
        return ecorruptsect(datasect);
    }

    return NULL;
}


static Error *
parsecustoms(__attribute__ ((unused)) Module *m,
            __attribute__ ((unused)) u8 *begin,
            __attribute__ ((unused)) const u8 *end)
{
    /* TODO(i4k) */
    return NULL;
}


static Error *
parseelements(__attribute__ ((unused)) Module *m,
            __attribute__ ((unused)) u8 *begin,
            __attribute__ ((unused)) const u8 *end)
{
    /* TODO(i4k) */
    return NULL;
}


void
closemodule(Module *m)
{
    u32         i;
    CodeDecl    *code;
    FuncDecl    *f;
    ImportDecl  *import;
    ExportDecl  *export;

    /*
     * TODO(i4k): Urgently in need of a memory pool.
     */

    closefile(&m->file);
    freearray(m->sects);

    if (m->types) {
        for (i = 0; i < len(m->types); i++) {
            f = arrayget(m->types, i);

            if (f->params) {
                freearray(f->params);
            }

            if (f->rets) {
                freearray(f->rets);
            }
        }
        freearray(m->types);
    }

    if (m->funcs) {
        freearray(m->funcs);
    }

    if (m->imports) {
        for (i = 0; i < len(m->imports); i++) {
            import = arrayget(m->imports, i);
            if (fast(import != NULL)) {
                free(import->module);
                free(import->field);
            }
        }

        freearray(m->imports);
    }

    if (m->tables) {
        freearray(m->tables);
    }

    if (m->memories) {
        freearray(m->memories);
    }

    if (m->globals) {
        freearray(m->globals);
    }

    if (m->exports) {
        for (i = 0; i < len(m->exports); i++) {
            export = arrayget(m->exports, i);
            if (fast(export != NULL)) {
                free(export->field);
            }
        }

        freearray(m->exports);
    }

    if (m->codes) {
        for (i = 0; i < len(m->codes); i++) {
            code = arrayget(m->codes, i);
            if (fast(code != NULL)) {
                free(code->locals);
            }
        }

        freearray(m->codes);
    }
}
