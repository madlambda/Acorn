/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "array.h"
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


Module *
loadmodule(const char *filename, Error *err)
{
    u8      *begin, *end;
    File    *file;
    Module  *mod;

    err->msg = NULL;

    file = openfile(filename, err);
    if (slow(file == NULL)) {
        return NULL;
    }

    if (slow(file->size < 8)) {
        errset(err, "WASM must have at least 8 bytes");
        goto fail;
    }

    begin = file->data;
    end = file->data + file->size;

    if (slow(memcmp(begin, "\0asm", 4) != 0)) {
        errset(err, "file is not a WASM module");
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
            return ERR;
        }

        if (slow(arrayadd(m->sects, &sect) != OK)) {
            errset(err, "failed to add section");
            return ERR;
        }

        if (slow(sect.id >= LastSectionId)) {
            errset(err, "invalid section id");
            return ERR;
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
        errset(err, "malformed section code");
        return ERR;
    }

    if (slow(u32vdecode(begin, end, &s->len) != OK)) {
        errset(err, "malformed section len");
        return ERR;
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
        errset(err, "multiple type section entries");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        errset(err, "malformed type section");
        return ERR;
    }

    found = 0;

    m->types = newarray(count, sizeof(FuncDecl));
    if (slow(m->types == NULL)) {
        return ERR;
    }

    while (begin < end && found < count) {
        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            errset(err, "malformed form of func_type");
            return ERR;
        }

        memset(&func, 0, sizeof(FuncDecl));

        func.form = (Type) -i8val;

        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            errset(err, "malformed param_count of func_type");
            return ERR;
        }

        paramcount = u32val;

        func.params = newarray(paramcount, sizeof(Type));
        if (slow(func.params == NULL)) {
            errset(err, "failed to create func array");
            return ERR;
        }

        for (i = 0; i < paramcount; i++) {
            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                errset(err, "malformed param type of func_type");
                return ERR;
            }

            value = -i8val;

            if (slow(arrayadd(func.params, &value) != OK)) {
                errset(err, "failed to add type");
                return ERR;
            }
        }

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            errset(err, "failed to get return count");
            return ERR;
        }

        retcount = u8val;

        func.rets = newarray(retcount, sizeof(Type));
        if (slow(func.rets == NULL)) {
            errset(err, "failed to create ret array");
            return ERR;
        }

        for (i = 0; i < retcount; i++) {
            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                errset(err, "malformed param type of func_type");
                return ERR;
            }

            value = -i8val;

            if (slow(arrayadd(func.params, &value) != OK)) {
                errset(err, "failed to add type");
                return ERR;
            }
        }

        if (slow(arrayadd(m->types, &func) != OK)) {
            errset(err, "failed to add func type");
            return ERR;
        }

        found++;
    }

    expect(begin <= end);

    if (slow(found < count)) {
        errset(err, "some types are missing, binary malformed");
        return ERR;
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
        errset(err, "multiple imports section");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
        errset(err, "imports section malformed");
        return ERR;
    }

    nimports = u32val;

    m->imports = newarray(nimports, sizeof(ImportDecl));
    if (slow(m->imports == NULL)) {
        errset(err, "failed to create imports array");
        return ERR;
    }

    for (i = 0; i < nimports; i++) {
        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            errset(err, "imports section malformed");
            return ERR;
        }

        import.module = allocstring(u32val);
        if (slow(import.module == NULL)) {
            errset(err, "failed to allocate module string");
            return ERR;
        }

        strset(import.module, begin, u32val);

        begin += u32val;

        if (slow(u32vdecode(&begin, end, &u32val) != OK)) {
            errset(err, "imports section malformed");
            return ERR;
        }

        import.field = allocstring(u32val);
        if (slow(import.field == NULL)) {
            errset(err, "failed to allocate field string");
            return ERR;
        }

        strset(import.field, begin, u32val);

        begin += u32val;

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            errset(err, "imports section malformed");
            return ERR;
        }

        import.kind = u8val;

        if (slow(u8vdecode(&begin, end, &u8val) != OK)) {
            errset(err, "imports section malformed");
            return ERR;
        }

        switch (import.kind) {
        case Function:
            f = arrayget(m->types, u8val);
            if (slow(f == NULL)) {
                errset(err, "import section references unknown function type");
                return ERR;
            }

            import.u.function = *f;
            break;

        default:
            errset(err, "external kind not supported yet");
            return ERR;
        }

        if (slow(arrayadd(m->imports, &import) != OK)) {
            return ERR;
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
        errset(err, "multiple function sections");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        errset(err, "function section malformed");
        return ERR;
    }

    m->funcs = newarray(count, sizeof(FuncDecl));
    if (slow(m->funcs == NULL)) {
        errset(err, "failed to allocate funcs array");
        return ERR;
    }

    while (len(m->funcs) < count && begin < end) {
        if (slow(u32vdecode(&begin, end, &uval))) {
            errset(err, "function section malformed");
            return ERR;
        }

        f = arrayget(m->types, uval);
        if (slow(f == NULL)) {
            errset(err, "type decl not found");
            return ERR;
        }

        if (slow(arrayadd(m->funcs, f) != OK)) {
            errset(err, "failed to add function to array");
            return ERR;
        }
    }

    if (slow(len(m->funcs) < count)) {
        errset(err, "functions missing, binary malformed");
        return ERR;
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
        errset(err, "multiple table sections");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        errset(err, "table section malformed");
        return ERR;
    }

    m->tables = newarray(count, sizeof(TableDecl));
    if (slow(m->tables == NULL)) {
        errset(err, "failed to allocate tables array");
        return ERR;
    }

    while (len(m->tables) < count && begin < end) {
        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            errset(err, "malformed binary");
            return ERR;
        }

        memset(&tbl, 0, sizeof(TableDecl));

        tbl.type = (Type) -i8val;

        if (slow(parselimits(&begin, end, &tbl.limit, err) != OK)) {
            return ERR;
        }

        if (slow(arrayadd(m->tables, &tbl) != OK)) {
            errset(err, "failed to add table");
            return ERR;
        }
    }

    if (slow(len(m->tables) < count)) {
        errset(err, "failed to parse all table elements.");
        return ERR;
    }

    return OK;
}


static u8
parsememories(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u32         count;
    MemoryDecl  mem;

    if (slow(m->memories != NULL)) {
        errset(err, "multiple memory sections");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        errset(err, "memory section malformed");
        return ERR;
    }

    m->memories = newarray(count, sizeof(MemoryDecl));
    if (slow(m->memories == NULL)) {
        errset(err, "failed to create array");
        return ERR;
    }

    while (len(m->memories) < count && begin < end) {
        memset(&mem, 0, sizeof(MemoryDecl));

        if (slow(parselimits(&begin, end, &mem.limit, err) != OK)) {
            return ERR;
        }

        if (slow(arrayadd(m->memories, &mem) != OK)) {
            errset(err, "failed to add memory entry");
            return ERR;
        }
    }

    if (slow(len(m->memories) < count)) {
        errset(err, "failed to parse all memory entries");
        return ERR;
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
        errset(err, "multiple global sections");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        errset(err, "global section malformed");
        return ERR;
    }

    m->globals = newarray(count, sizeof(GlobalDecl));
    if (slow(m->globals == NULL)) {
        errset(err, "failed to create array");
        return ERR;
    }

    while (len(m->globals) < count && begin < end) {
        memset(&global, 0, sizeof(GlobalDecl));

        if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
            errset(err, "failed to parse global type");
            return ERR;
        }

        global.type.type = (Type) -i8val;

        if (slow(u8vdecode(&begin, end, &global.type.mut) != OK)) {
            errset(err, "failed to parse global mutability");
            return ERR;
        }

        opcode = *begin++;

        switch (opcode) {
        case OpgetGlobal:
            if (slow(u32vdecode(&begin, end, &global.u.globalindex) != OK)) {
                errset(err, "failed to parse get_global value");
                return ERR;
            }
            break;

        case Opi32const:
            if (slow(s32vdecode(&begin, end, &global.u.i32val) != OK)) {
                errset(err, "failed to parse i32.const value");
                return ERR;
            }
            break;

        case Opi64const:
            if (slow(s64vdecode(&begin, end, &global.u.i64val) != OK)) {
                errset(err, "failed to parse i64.const value");
                return ERR;
            }
            break;

        case Opf32const:
            if (slow(u32decode(&begin, end, &global.u.f32val) != OK)) {
                errset(err, "failed to parse f32.const");
                return ERR;
            }
            break;

        case Opf64const:
            if (slow(u32decode(&begin, end, &global.u.f32val) != OK)) {
                errset(err, "failed to parse f32.const");
                return ERR;
            }
            break;

        default:
            errset(err, "unsupported global expression");
            return ERR;
        }

        if (slow(arrayadd(m->globals, &global) != OK)) {
            errset(err, "failed to add array");
            return ERR;
        }
    }

    if (slow(len(m->globals) < count)) {
        errset(err, "failed to parse all globals");
        return ERR;
    }

    return OK;
}


static u8
parseexports(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u32         count, uval;
    ExportDecl  export;

    if (slow(m->exports != NULL)) {
        errset(err, "multiple export sections");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count))) {
        errset(err, "exports section malformed");
        return ERR;
    }

    m->exports = newarray(count, sizeof(ExportDecl));
    if (slow(m->exports == NULL)) {
        errset(err, "failed to allocate exports array");
        return ERR;
    }

    while (len(m->exports) < count && begin < end) {
        if (slow(u32vdecode(&begin, end, &uval) != OK)) {
            errset(err, "failed to parse field_len");
            return ERR;
        }

        memset(&export, 0, sizeof(ExportDecl));

        export.field = allocstring(uval);
        if (slow(export.field == NULL)) {
            errset(err, "failed to allocate string");
            return ERR;
        }

        strset(export.field, begin, uval);

        begin += uval;

        export.kind = (u8) *begin++;

        if (slow(u32vdecode(&begin, end, &export.index) != OK)) {
            errset(err, "failed to parse exports index");
            return ERR;
        }

        if (slow(arrayadd(m->exports, &export) != OK)) {
            errset(err, "failed to add export entry");
            return ERR;
        }
    }

    if (slow(len(m->exports) < count)) {
        errset(err, "failed to add all export entries");
        return ERR;
    }

    return OK;
}


static u8
parsestarts(Module *m, u8 *begin, const u8 *end, Error *err)
{
    if (slow(u32vdecode(&begin, end, &m->start) != OK)) {
        errset(err, "failed to parse start function");
        return ERR;
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
        errset(err, "multiple code sections");
        return ERR;
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        errset(err, "malformed code section");
        return ERR;
    }

    m->codes = newarray(count, sizeof(CodeDecl));
    if (slow(m->codes == NULL)) {
        errset(err, "failed to allocate array");
        return ERR;
    }

    bodyend = 0;

    while (len(m->codes) < count && begin < end) {
        bodybegin = begin;

        if (slow(u32vdecode(&begin, end, &bodysize) != OK)) {
            errset(err, "failed to parse function body size");
            return ERR;
        }

        bodyend = (bodybegin + bodysize);

        if (slow(*bodyend != 0x0b)) {
            errset(err, "malformed function body");
            return ERR;
        }

        if (slow(u32vdecode(&begin, end, &localcount) != OK)) {
            errset(err, "failed to parse body local count");
            return ERR;
        }

        memset(&code, 0, sizeof(CodeDecl));

        code.locals = newarray(localcount, sizeof(LocalEntry));
        if (slow(code.locals == NULL)) {
            errset(err, "failed to allocate array");
            return ERR;
        }

        while (len(code.locals) < localcount && begin < end) {
            memset(&local, 0, sizeof(LocalEntry));

            if (slow(u32vdecode(&begin, end, &local.count) != OK)) {
                errset(err, "failed to parse local count");
                return ERR;
            }

            if (slow(s8vdecode(&begin, end, &i8val) != OK)) {
                errset(err, "failed to parse local type");
                return ERR;
            }

            local.type = (Type) -i8val;

            if (slow(arrayadd(code.locals, &local) != OK)) {
                errset(err, "failed to add local to array");
                return ERR;
            }
        }

        if (slow(len(code.locals) < localcount)) {
            errset(err, "failed to parse all locals");
            return ERR;
        }

        code.start = begin;
        code.end = bodyend;

        if (slow(arrayadd(m->codes, &code) != OK)) {
            errset(err, "failed to add code");
            return ERR;
        }
    }

    if (slow(len(m->codes) < count)) {
        errset(err, "failed to parse all function bodies");
        return ERR;
    }

    if (slow(count > 0 && (end - bodyend) != 1)) {
        errset(err, "surplus bytes in the end of code section");
        return ERR;
    }

    return OK;
}


static u8
parselimits(u8 **begin, const u8 *end, ResizableLimit *limit, Error *err)
{
    u8  u8val;

    if (slow(u8vdecode(begin, end, &u8val) != OK)) {
        errset(err, "malformed binary. Is it really WASM MVP binary?");
        return ERR;
    }

    limit->flags = u8val;

    if (slow(u32vdecode(begin, end, &limit->initial) != OK)) {
        errset(err, "malformed binary");
        return ERR;
    }

    if (slow((limit->flags & 1) &&
             u32vdecode(begin, end, &limit->maximum) != OK))
    {
        errset(err, "malformed binary");
        return ERR;
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
        errset(err, "multiple data sections");
        return  ERR;
    }

    if (slow(u32vdecode(&begin, end, &count) != OK)) {
        errset(err, "failed to get count from data section");
        return ERR;
    }

    m->datas = newarray(count, sizeof(DataDecl));
    if (slow(m->datas == NULL)) {
        errset(err, "failed to create array");
        return ERR;
    }

    while (len(m->datas) < count && begin < end) {
        memset(&data, 0, sizeof(DataDecl));

        if (slow(u32vdecode(&begin, end, &data.index) != OK)) {
            errset(err, "failed to get data index");
            return ERR;
        }

        opcode = *begin++;

        switch (opcode) {
        case Opi32const:
            if (slow(s32vdecode(&begin, end, &data.offset) != OK)) {
                errset(err, "failed to parse i32.const value");
                return ERR;
            }
            break;

        default:
            errset(err, "unsupported data init expression");
            return ERR;
        }

        if (slow(u32vdecode(&begin, end, &data.size) != OK)) {
            errset(err, "failed to parse data size");
            return ERR;
        }

        data.data = begin;

        if (slow(arrayadd(m->datas, &data) != OK)) {
            errset(err, "failed to add data array");
            return ERR;
        }
    }

    if (slow(len(m->datas) < count)) {
        errset(err, "not all data parsed");
        return ERR;
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
