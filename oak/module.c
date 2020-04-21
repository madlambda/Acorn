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
#include "module.h"
#include "bin.h"


typedef u8 (*Parser)(Module *m, u8 *begin, const u8 *end, Error *err);


static u8 parsesects(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 *parsesect(u8 *begin, const u8 *end, Section *s, Error *err);
static u8 parsetypes(Module *m, u8 *begin, const u8 *end, Error *err);
static u8 parseimports(Module *m, u8 *begin, const u8 *end, Error *err);


static const Parser  parsers[] = {
    parsetypes,
    parseimports,
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

    u32decode(begin, &mod->version);
    begin += 4;

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
        begin = parsesect(begin, end, &sect, err);
        if (slow(begin == NULL)) {
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


static u8 *
parsesect(u8 *begin, const u8 *end, Section *s, Error *err)
{
    u64      val;
    ssize_t  read;

    if (slow(udecode(begin, end, &val) != 1)) {
        errset(err, "malformed section code");
        return NULL;
    }

    begin++;

    s->id = (u8) val;

    read = udecode(begin, end, &val);
    if (slow(read < 0 || read > 4)) {
        errset(err, "malformed section len");
        return NULL;
    }

    s->len = (u32) val;
    begin += read;

    s->data = begin;

    return begin + s->len;
}


static u8
parsetypes(Module *m, u8 *begin, const u8 *end, Error *err)
{
    u64       i, uval;
    i64       ival;
    u32       found, count;
    ssize_t   read;
    TypeId    value;
    FuncDecl  func;

    if (slow(m->funcs != NULL)) {
        errset(err, "multiple type section entries");
        return ERR;
    }

    m->funcs = newarray(16, sizeof(FuncDecl));
    if (slow(m->funcs == NULL)) {
        return ERR;
    }

    read = udecode(begin, end, &uval);
    if (slow(read < 0 || read > 4)) {
        errset(err, "malformed type section");
        return ERR;
    }

    begin += read;

    count = uval;
    found = 0;

    while (begin < end && found < count) {
        read = sdecode(begin, end, &ival);
        if (slow(read < 0 || read > 1)) {
            errset(err, "malformed form of func_type");
            return ERR;
        }

        begin += read;

        memset(&func, 0, sizeof(FuncDecl));

        func.form = -ival;

        read = udecode(begin, end, &uval);
        if (slow(read < 0 || read > 4)) {
            errset(err, "malformed param_count of func_type");
            return ERR;
        }

        begin += read;

        func.params = newarray(uval, sizeof(TypeId));
        if (slow(func.params == NULL)) {
            errset(err, "failed to create func array");
            return ERR;
        }

        for (i = 0; i < uval; i++) {
            read = sdecode(begin, end, &ival);
            if (slow(read != 1)) {
                errset(err, "malformed param type of func_type");
                return ERR;
            }

            begin += read;

            value = -ival;

            if (slow(arrayadd(func.params, &value) != OK)) {
                errset(err, "failed to add type");
                return ERR;
            }
        }

        read = udecode(begin, end, &uval);
        if (slow(read != 1)) {
            errset(err, "failed to get return count");
            return ERR;
        }

        begin += read;

        func.rets = newarray(uval, sizeof(TypeId));
        if (slow(func.rets == NULL)) {
            errset(err, "failed to create ret array");
            return ERR;
        }

        for (i = 0; i < uval; i++) {
            read = sdecode(begin, end, &ival);
            if (slow(read != 1)) {
                errset(err, "malformed param type of func_type");
                return ERR;
            }

            begin += read;

            value = -ival;

            if (slow(arrayadd(func.params, &value) != OK)) {
                errset(err, "failed to add type");
                return ERR;
            }
        }

        if (slow(arrayadd(m->funcs, &func) != OK)) {
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
    u32  read, uval;

    if (slow(m->imports != NULL)) {
        errset(err, "multiple imports section");
        return ERR;
    }

    read = udecode(begin, end, &uval);
    if (slow(read <= 0 || read > 4)) {
        errset(err, "imports section malformed");
        return ERR;
    }

    m->imports = newarray(uval, sizeof(ImportDecl));
    if (slow(m->imports == NULL)) {
        errset(err, "failed to create imports array");
        return ERR;
    }

    return OK;
}


void
closemodule(Module *m)
{
    u32       i;
    FuncDecl  *f;

    closefile(m->file);
    freearray(m->sects);

    if (m->funcs) {
        for (i = 0; i < arraylen(m->funcs); i++) {
            f = arrayget(m->funcs, i);

            if (f->params) {
                freearray(f->params);
            }

            if (f->rets) {
                freearray(f->rets);
            }
        }
        freearray(m->funcs);
    }
    free(m);
}
