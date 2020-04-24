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


typedef u8 (*Parser)(Module *m, u8 *begin, const u8 *end, Error *err);


static u8 parsesects(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsesect(u8 **begin, const u8 *end, Section *s, Error *err);
static u8 parsecustoms(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsetypes(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parseimports(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsefunctions(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsetables(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsememories(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parseglobals(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parseexports(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsestarts(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parseelements(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsecodes(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parsedatas(Module *m, u8 *begin, const u8 *end, Error *err);

static u8 parselimits(u8 **begin, const u8 *end, ResizableLimit *limit,
    Error *err);


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


static const char  *Edupsect        = "section duplicated";
static const char  *Estralloc       = "failed to allocate string";
static const char  *Earrayadd       = "failed to add item to array: %E";
static const char  *Eallocarray     = "failed to allocate array: %E";
static const char  *Ecorruptsect    = "section corrupted";


#define edupsect()      error(err, Edupsect)
#define estralloc()     error(err, Estralloc)
#define earrayadd()     error(err, Earrayadd, errno)
#define earrayalloc()   error(err, Eallocarray, errno)
#define ecorruptsect()  error(err, Ecorruptsect)


Module *
loadmodule(const char *filename, Error *err)
{
    u8      *begin, *end;
    File    *file;
    Module  *mod;

    file = openfile(filename, err);
    if (slow(file == NULL)) {
        return NULL;
    }

    if (slow(file->size < 8)) {
        error(err, "WASM must have at least 8 bytes");
        goto fail;
    }

    begin = file->data;
    end = file->data + file->size;

    if (slow(memcmp(begin, "\0asm", 4) != 0)) {
        error(err, "file is not a WASM module");
        goto fail;
    }

    begin += 4;

    mod = zmalloc(sizeof(Module));
    if (slow(mod == NULL)) {
        goto fail;
    }

    mod->file = file;

    mod->sects = newarray(16, sizeof(Section));
    if (slow(mod->sects == NULL)) {
        goto free;
    }

    u32decode(&begin, end, &mod->version);

    if (slow(parsesects(mod, begin, end, err) != OK)) {
        goto free;
    }

    return mod;

free:

    free(mod);

fail:

    closefile(file);
    return NULL;
}


static u8
parsesects(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u8       ret;
    Parser   parser;
    Section  sect;

    while (begin < end) {
        memset(&sect, 0, sizeof(Section));

        if (slow(parsesect(&begin, end, &sect, err) != OK)) {
            return error(err, "failed to parse sect");
        }

        if (slow(arrayadd(m->sects, &sect) != OK)) {
            return error(err, "failed to add section");
        }

        if (slow(sect.id >= LastSectionId)) {
            return error(err, "invalid section id: %d", sect.id);
        }

        parser = parsers[sect.id];

        ret = parser(m, (u8 *) sect.data, sect.data + sect.len, err);
        if (slow(ret != OK)) {
            return ERR;
        }
    }

    return OK;
}


static u8
parsesect(u8 **begin, const u8 *end, Section *s, Error *err)
{
    if (slow(u8vdecode(begin, end, (u8 *) &s->id) != OK)) {
        return error(err, "malformed section id");
    }

    if (slow(u32vdecode(begin, end, &s->len) != OK)) {
        return error(err, "malformed section len");
    }

    s->data = *begin;
    *begin += s->len;
    return OK;
}


static u8
parsetypes(Module *m, u8 *begin, const u8 *end, Error *err)
{
    i8        i8val;
    u8        u8val;
    u32       found, count, paramcount, retcount, u32val;
    u64       i;
    Type      value;
    FuncDecl  func;

    if (slow(m->types != NULL)) {
        return error(err, "multiple \"type\" section entries");
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        return error(err, "malformed \"type\" section");
    }

    found = 0;

    m->types = newarray(count, sizeof(FuncDecl));
    if (slow(m->types == NULL)) {
        return earrayalloc();
    }

    while (begin < end && found < count) {
        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            return error(err, "malformed \"form\" of \"type\" section");
        }

        memset(&func, 0, sizeof(FuncDecl));

        func.form = (Type) -i8val;

        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            return error(err, "malformed param_count of func_type");
        }

        paramcount = u32val;

        func.params = newarray(paramcount, sizeof(Type));
        if (slow(func.params == NULL)) {
            return earrayalloc();
        }

        for (i = 0; i < paramcount; i++) {
            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                return error(err, "malformed param type of func_type");
            }

            value = -i8val;

            if (slow(arrayadd(func.params, &value) != OK)) {
                return earrayadd();
            }
        }

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            return error(err, "failed to get return count");
        }

        retcount = u8val;

        func.rets = newarray(retcount, sizeof(Type));
        if (slow(func.rets == NULL)) {
            return earrayalloc();
        }

        for (i = 0; i < retcount; i++) {
            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                return error(err, "malformed param type of func_type");
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
        return ecorruptsect();
    }

    return OK;
}


static u8
parseimports(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u8          u8val;
    u32         i, u32val, nimports;
    FuncDecl    *f;
    ImportDecl  import;

    if (slow(m->imports != NULL)) {
        return edupsect();
    }

    if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
        return ecorruptsect();
    }

    nimports = u32val;

    m->imports = newarray(nimports, sizeof(ImportDecl));
    if (slow(m->imports == NULL)) {
        return earrayalloc();
    }

    for (i = 0; i < nimports; i++) {
        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            return ecorruptsect();
        }

        import.module = allocstring(u32val);
        if (slow(import.module == NULL)) {
            return estralloc();
        }

        strset(import.module, begin, u32val);

        begin += u32val;

        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            return ecorruptsect();
        }

        import.field = allocstring(u32val);
        if (slow(import.field == NULL)) {
            return estralloc();
        }

        strset(import.field, begin, u32val);

        begin += u32val;

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            return ecorruptsect();
        }

        import.kind = u8val;

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            return ecorruptsect();
        }

        switch (import.kind) {
        case Function:
            f = arrayget(m->types, u8val);
            if (slow(f == NULL)) {
                return error(err, "import section references unknown function");
            }

            import.u.function = *f;
            break;

        default:
            return error(err, "external kind not supported yet");
        }

        if (slow(arrayadd(m->imports, &import) != OK)) {
            return earrayadd();
        }
    }

    return OK;
}


static u8
parsefunctions(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u32       count, uval;
    FuncDecl  *f;

    if (slow(m->funcs != NULL)) {
        return edupsect();
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect();
    }

    m->funcs = newarray(count, sizeof(FuncDecl));
    if (slow(m->funcs == NULL)) {
        return earrayalloc();
    }

    while (len(m->funcs) < count && begin < end) {
        if (slow(u32vdecode(&begin, end, &uval))) {
            return ecorruptsect();
        }

        f = arrayget(m->types, uval);
        if (slow(f == NULL)) {
            return error(err, "type %d not found", uval);
        }

        if (slow(arrayadd(m->funcs, f) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->funcs) < count)) {
        return ecorruptsect();
    }

    return OK;
}


static u8
parsetables(Module *m, u8 *begin, const u8 *end, Error *err)
{
    i8         i8val;
    u32        count;
    TableDecl  tbl;

    if (slow(m->tables != NULL)) {
        return edupsect();
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect();
    }

    m->tables = newarray(count, sizeof(TableDecl));
    if (slow(m->tables == NULL)) {
        return earrayalloc();
    }

    while (len(m->tables) < count && begin < end) {
        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            return ecorruptsect();
        }

        memset(&tbl, 0, sizeof(TableDecl));

        tbl.type = (Type) -i8val;

        if (slow(parselimits(&begin, end, &tbl.limit, err) != OK)) {
            return ERR;
        }

        if (slow(arrayadd(m->tables, &tbl) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->tables) < count)) {
        return ecorruptsect();
    }

    return OK;
}


static u8
parsememories(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u32         count;
    MemoryDecl  mem;

    if (slow(m->memories != NULL)) {
        return ecorruptsect();
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect();
    }

    m->memories = newarray(count, sizeof(MemoryDecl));
    if (slow(m->memories == NULL)) {
        return earrayalloc();
    }

    while (len(m->memories) < count && begin < end) {
        memset(&mem, 0, sizeof(MemoryDecl));

        if (slow(parselimits(&begin, end, &mem.limit, err) != OK)) {
            return ERR;
        }

        if (slow(arrayadd(m->memories, &mem) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->memories) < count)) {
        return ecorruptsect();
    }

    return OK;
}


static u8
parseglobals(Module *m, u8 *begin, const u8 *end, Error *err)
{
    i8          i8val;
    u8          opcode;
    u32         count;
    GlobalDecl  global;

    if (slow(m->globals != NULL)) {
        return ecorruptsect();
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect();
    }

    m->globals = newarray(count, sizeof(GlobalDecl));
    if (slow(m->globals == NULL)) {
        return earrayalloc();
    }

    while (len(m->globals) < count && begin < end) {
        memset(&global, 0, sizeof(GlobalDecl));

        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            return error(err, "failed to parse global type");
        }

        global.type.type = (Type) -i8val;

        if (slow(u8vdecode(&begin, end, &global.type.mut) != OK)) {
            return error(err, "failed to parse global mutability");
        }

        opcode = *begin++;

        switch (opcode) {
        case OpgetGlobal:
            if (slow(u32vdecode(&begin, end, &global.u.globalindex) != OK)) {
                return error(err, "failed to parse get_global value");
            }

            break;

        case Opi32const:
            if (slow(s32vdecode(&begin, end, &global.u.i32val) != OK)) {
                return error(err, "failed to parse i32.const value");
            }

            break;

        case Opi64const:
            if (slow(s64vdecode(&begin, end, &global.u.i64val) != OK)) {
                return error(err, "failed to parse i64.const value");
            }

            break;

        case Opf32const:
            if (slow(u32decode(&begin, end, &global.u.f32val) != OK)) {
                return error(err, "failed to parse f32.const");
            }

            break;

        case Opf64const:
            if (slow(u32decode(&begin, end, &global.u.f32val) != OK)) {
                return error(err, "failed to parse f32.const");
            }

            break;

        default:
            return error(err, "unsupported global expression");
        }

        if (slow(arrayadd(m->globals, &global) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->globals) < count)) {
        return ecorruptsect();
    }

    return OK;
}


static u8
parseexports(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u32         count, uval;
    ExportDecl  export;

    if (slow(m->exports != NULL)) {
        return edupsect();
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        return ecorruptsect();
    }

    m->exports = newarray(count, sizeof(ExportDecl));
    if (slow(m->exports == NULL)) {
        return earrayalloc();
    }

    while (len(m->exports) < count && begin < end) {
        if (slow(u32vdecode(&begin, end, &uval) != OK)) {
            return error(err, "failed to parse field_len");
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
            return error(err, "failed to parse exports index");
        }

        if (slow(arrayadd(m->exports, &export) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->exports) < count)) {
        return ecorruptsect();
    }

    return OK;
}


static u8
parsestarts(Module *m, u8 *begin, const u8 *end, Error *err)
{
    if (slow(u32vdecode(&begin, end, &m->start) != OK)) {
        return error(err, "failed to parse start function");
    }

    return OK;
}


static u8
parsecodes(Module *m, u8 *begin, const u8 *end, Error *err)
{
    i8          i8val;
    u8          *bodybegin, *bodyend;
    u32         count, bodysize, localcount;
    CodeDecl    code;
    LocalEntry  local;

    if (slow(m->codes != NULL)) {
        return edupsect();
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        return ecorruptsect();
    }

    m->codes = newarray(count, sizeof(CodeDecl));
    if (slow(m->codes == NULL)) {
        return estralloc();
    }

    bodyend = 0;

    while (len(m->codes) < count && begin < end) {
        bodybegin = begin;

        if (slow(u32vdecode(&begin, end, &bodysize) != OK)) {
            return error(err, "failed to parse function body size");
        }

        bodyend = (bodybegin + bodysize);

        if (slow(*bodyend != 0x0b)) {
            return error(err, "malformed body: missing 0x0b in the end");
        }

        if (slow(u32vdecode(&begin, end, &localcount) != OK)) {
            return error(err, "failed to parse body local count");
        }

        memset(&code, 0, sizeof(CodeDecl));

        code.locals = newarray(localcount, sizeof(LocalEntry));
        if (slow(code.locals == NULL)) {
            return earrayalloc();
        }

        while (len(code.locals) < localcount && begin < end) {
            memset(&local, 0, sizeof(LocalEntry));

            if (slow(u32vdecode(&begin, end, &local.count) != OK)) {
                return error(err, "failed to parse local count");
            }

            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                return error(err, "failed to parse local type");
            }

            local.type = (Type) -i8val;

            if (slow(arrayadd(code.locals, &local) != OK)) {
                return earrayadd();
            }
        }

        if (slow(len(code.locals) < localcount)) {
            return ecorruptsect();
        }

        code.start = begin;
        code.end = bodyend;

        if (slow(arrayadd(m->codes, &code) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->codes) < count)) {
        return ecorruptsect();
    }

    if (slow(count > 0 && (end - bodyend) != 1)) {
        return error(err, "surplus bytes in the end of code section");
    }

    return OK;
}


static u8
parselimits(u8 **begin, const u8 *end, ResizableLimit *limit, Error *err)
{
    u8  u8val;

    if (slow(u8vdecode(begin, end, &u8val) != OK)) {
        return error(err, "malformed limits. Is it really WASM MVP binary?");
    }

    limit->flags = u8val;

    if (slow(u32vdecode(begin, end, &limit->initial) != OK)) {
        return ecorruptsect();
    }

    if (slow((limit->flags & 1) &&
             u32vdecode(begin, end, &limit->maximum) != OK))
    {
        return ecorruptsect();
    }

    return OK;
}


static u8
parsedatas(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u8        opcode;
    u32       count;
    DataDecl  data;

    if (slow(m->datas != NULL)) {
        return edupsect();
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        return ecorruptsect();
    }

    m->datas = newarray(count, sizeof(DataDecl));
    if (slow(m->datas == NULL)) {
        return earrayalloc();
    }

    while (len(m->datas) < count && begin < end) {
        memset(&data, 0, sizeof(DataDecl));

        if (slow(u32vdecode(&begin, end, &data.index) != OK)) {
            return error(err, "failed to get data index");
        }

        opcode = *begin++;

        switch (opcode) {
        case Opi32const:
            if (slow(s32vdecode(&begin, end, &data.offset) != OK)) {
                return error(err, "failed to parse i32.const value");
            }
            break;

        default:
            return error(err, "unsupported data init expression");
        }

        if (slow(u32vdecode(&begin, end, &data.size) != OK)) {
            return error(err, "failed to parse data size");
        }

        data.data = begin;

        if (slow(arrayadd(m->datas, &data) != OK)) {
            return earrayadd();
        }
    }

    if (slow(len(m->datas) < count)) {
        return ecorruptsect();
    }

    return OK;
}


static u8
parsecustoms(__attribute__ ((unused)) Module *m,
            __attribute__ ((unused)) u8 *begin,
            __attribute__ ((unused)) const u8 *end,
            __attribute__ ((unused)) Error *err)
{
    /* TODO(i4k) */
    return OK;
}


static u8
parseelements(__attribute__ ((unused)) Module *m,
            __attribute__ ((unused)) u8 *begin,
            __attribute__ ((unused)) const u8 *end,
            __attribute__ ((unused)) Error *err)
{
    /* TODO(i4k) */
    return OK;
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

    closefile(m->file);
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

    free(m);
}
