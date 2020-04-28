/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>

#include <stdlib.h>
#include <string.h>


#define check(ret, expr)                                                      \
    do {                                                                      \
        ret = expr;                                                           \
        if (slow(ret == NULL)) {                                              \
            return ERR;                                                       \
        }                                                                     \
    } while (0)


#define min(a, b)                                                             \
    ((a < b) ? (a) : (b))


static u8 modulefmt(String **buf, u8 **format, void *val);
static u8 sectidfmt(String **buf, u8 **format, void *val);
static u8 funcfmt(String **buf, u8 **format, void *val);
static u8 typefmt(String **buf, u8 **format, void *val);
static u8 importfmt(String **buf, u8 **format, void *val);

static const char *typestr(Type t);


u8
oakfmt(String **buf, u8 **format, void *val)
{
    u8  *fmt, *p, *end;

    fmt = *format;
    fmt++;

    if (*fmt == '(') {
        p = ++fmt;

        while (*fmt != '\0' && *fmt != ')') {
            fmt++;
        }

        if (fast(*fmt == ')')) {
            end = fmt;

        } else {
            return ERR;
        }

        if (memcmp("sectid", p, min(6, (end - p))) == 0) {
            *format = fmt;
            return sectidfmt(buf, format, val);
        }

        if (memcmp("type", p, min(4, (end - p))) == 0) {
            *format = fmt;
            return typefmt(buf, format, val);
        }

        if (memcmp("func", p, min(4, (end - p))) == 0) {
            *format = fmt;
            return funcfmt(buf, format, val);
        }

        if (memcmp("import", p, min(6, (end - p))) == 0) {
            *format = fmt;
            return importfmt(buf, format, val);
        }

        return ERR;
    }

    return modulefmt(buf, format, val);
}


static u8
modulefmt(String **buf, u8 ** unused(format), void *val)
{
    String  *out;
    Module  *m;

    m = (Module *) val;
    if (slow(m == NULL)) {
        return ERR;
    }

    check(out, cfmt("Module (nsects %d) (ntypes %d) (nimports %d) "
               "(nexports %d) (ncodes %d) (nglobals %d) (start %d)",
               len(m->sects), len(m->funcs), len(m->imports),
               len(m->exports), len(m->codes), len(m->globals), m->start));

    *buf = append(*buf, out);

    free(out);

    if (slow(*buf == NULL)) {
        return ERR;
    }

    return OK;
}


static u8
sectidfmt(String **buf, u8 ** unused(format), void *val)
{
    SectionId  id;

    id = (SectionId) val;

    switch (id) {
    case CustomId:
        *buf = appendcstr(*buf, "Custom");
        break;

    case TypeId:
        *buf = appendcstr(*buf, "Type");
        break;

    case ImportId:
        *buf = appendcstr(*buf, "Import");
        break;

    case FunctionId:
        *buf = appendcstr(*buf, "Function");
        break;

    case TableId:
        *buf = appendcstr(*buf, "Table");
        break;

    case MemoryId:
        *buf = appendcstr(*buf, "Memory");
        break;

    case GlobalId:
        *buf = appendcstr(*buf, "Global");
        break;

    case ExportId:
        *buf = appendcstr(*buf, "Export");
        break;

    case StartId:
        *buf = appendcstr(*buf, "Start");
        break;

    case ElementId:
        *buf = appendcstr(*buf, "Element");
        break;

    case CodeId:
        *buf = appendcstr(*buf, "Code");
        break;

    case DataId:
        *buf = appendcstr(*buf, "Data");
        break;

    default:
        *buf = appendcstr(*buf, "Unknown");
        break;
    }

    if (slow(*buf == NULL)) {
        return ERR;
    }

    return OK;
}


static u8
funcfmt(String **buf, u8 ** unused(format), void *val)
{
    u32       i;
    Type      *t;
    FuncDecl  *f;

    f = (FuncDecl *) val;

    *buf = appendcstr(*buf, "(");

    for (i = 0; i < len(f->params); i++) {
        if (slow(*buf == NULL)) {
            return ERR;
        }

        t = arrayget(f->params, i);
        check(*buf, appendcstr(*buf, typestr(*t)));

        if (i < (len(f->params) - 1)) {
            *buf = appendc(*buf, 1, ',');
        }
    }

    if (slow(*buf == NULL)) {
        return ERR;
    }

    check(*buf, appendc(*buf, 2, ')', ' '));

    if (len(f->rets) == 0) {
        check(*buf, appendc(*buf, 3, 'n', 'i', 'l'));
        return OK;
    }

    *buf = appendc(*buf, 1, '(');

    for (i = 0; i < len(f->rets); i++) {
        if (slow(*buf == NULL)) {
            return ERR;
        }

        t = arrayget(f->rets, i);
        check(*buf, appendcstr(*buf, typestr(*t)));

        if (i < (len(f->rets) - 1)) {
            *buf = appendc(*buf, 1, ',');
        }
    }

    *buf = appendc(*buf, 1, ')');

    return OK;
}


static u8
typefmt(String **buf, u8 ** unused(format), void *val)
{
    Type  *t;

    t = (Type *) val;

    *buf = appendcstr(*buf, typestr(*t));
    if (slow(*buf == NULL)) {
        return ERR;
    }

    return OK;
}


static u8
importfmt(String **buf, u8 ** format, void *val)
{
    ImportDecl  *import;

    import = (ImportDecl *) val;

    switch (import->kind) {
    case Function:
        check(*buf, append(*buf, import->module));
        check(*buf, appendc(*buf, 1, '.'));
        check(*buf, append(*buf, import->field));

        if (funcfmt(buf, format, &import->u.function) != OK) {
            return ERR;
        }

        break;
    default:
        check(*buf, appendcstr(*buf, "(not implemented)"));
    }

    return OK;
}


static const char *
typestr(Type t)
{
    switch (t) {
    case I32:
        return "i32";

    case I64:
        return "i64";

    case F32:
        return "f32";

    case F64:
        return "f64";

    case Anyfunc:
        return "AnyFunc";

    case Func:
        return "Func";

    case Emptyblock:
        return "EmptyBlock";
    }

    return "(unknown)";
}
