/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include <oak/instance.h>

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
static u8 typefmt(String **buf, u8 **format, void *val);
static u8 typedeclfmt(String **buf, u8 **format, void *val);
static u8 extkindfmt(String **buf, u8 **format, void *val);
static u8 importfmt(String **buf, u8 **format, void *val);
static u8 exportfmt(String **buf, u8 **format, void *val);
static u8 valuefmt(String **buf, u8 **format, void *val);

static const char *typestr(Type t);
static const char *extkindstr(ExternalKind kind);


u8
oakfmt(String **buf, u8 **format, void *val)
{
    u8      *fmt, *p, *end;
    String  typestr;

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

        typestr.start = p;
        typestr.len = (end - p);

        if (cstringcmp(&typestr, "sectid")) {
            *format = fmt;
            return sectidfmt(buf, format, val);
        }

        if (cstringcmp(&typestr, "type")) {
            *format = fmt;
            return typefmt(buf, format, val);
        }

        if (cstringcmp(&typestr, "typedecl")) {
            *format = fmt;
            return typedeclfmt(buf, format, val);
        }

        if (cstringcmp(&typestr, "extkind")) {
            *format = fmt;
            return extkindfmt(buf, format, val);
        }

        if (cstringcmp(&typestr, "import")) {
            *format = fmt;
            return importfmt(buf, format, val);
        }

        if (cstringcmp(&typestr, "export")) {
            *format = fmt;
            return exportfmt(buf, format, val);
        }

        /* instance values */

        if (cstringcmp(&typestr, "value")) {
            *format = fmt;
            return valuefmt(buf, format, val);
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
typedeclfmt(String **buf, u8 ** unused(format), void *val)
{
    u32       i;
    Type      *t;
    TypeDecl  *type;

    type = (TypeDecl *) val;

    check(*buf, appendc(*buf, 1, '('));

    for (i = 0; i < len(type->params); i++) {
        if (slow(*buf == NULL)) {
            return ERR;
        }

        t = arrayget(type->params, i);
        check(*buf, appendcstr(*buf, typestr(*t)));

        if (i < (len(type->params) - 1)) {
            *buf = appendc(*buf, 1, ',');
        }
    }

    if (slow(*buf == NULL)) {
        return ERR;
    }

    check(*buf, appendc(*buf, 2, ')', ' '));

    if (len(type->rets) == 0) {
        check(*buf, appendc(*buf, 3, 'n', 'i', 'l'));
        return OK;
    }

    *buf = appendc(*buf, 1, '(');

    for (i = 0; i < len(type->rets); i++) {
        if (slow(*buf == NULL)) {
            return ERR;
        }

        t = arrayget(type->rets, i);
        check(*buf, appendcstr(*buf, typestr(*t)));

        if (i < (len(type->rets) - 1)) {
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
extkindfmt(String **buf, u8 ** unused(format), void *val)
{
    ExternalKind  *kind;

    kind = (ExternalKind *) val;

    check(*buf, appendcstr(*buf, extkindstr(*kind)));

    return OK;
}


static u8
importfmt(String **buf, u8 ** format, void *val)
{
    ImportDecl  *import;

    import = (ImportDecl *) val;

    switch (import->kind) {
    case FunctionKind:
        check(*buf, append(*buf, import->module));
        check(*buf, appendc(*buf, 1, '.'));
        check(*buf, append(*buf, import->field));

        if (typedeclfmt(buf, format, &import->u.type) != OK) {
            return ERR;
        }

        break;
    default:
        check(*buf, appendcstr(*buf, "(not implemented)"));
    }

    return OK;
}


static u8
exportfmt(String **buf, u8 ** format, void *val)
{
    ExportDecl  *export;

    export = (ExportDecl *) val;

    switch (export->kind) {
    case FunctionKind:
        check(*buf, append(*buf, export->field));
        if (typedeclfmt(buf, format, &export->u.type) != OK) {
            return ERR;
        }

        break;

    default:
        check(*buf, appendcstr(*buf, "(not implemented)"));
    }

    return OK;
}


static u8
valuefmt(String **buf, u8 ** unused(format), void *val)
{
    Value  *v;

    v = (Value *) val;

    switch (v->type) {
    case I32:
        *buf = cfmtbuf(*buf, "%d", v->u.ival);
        break;
    default:
        *buf = appendcstr(*buf, "<unknown>");
        break;
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


static const char *
extkindstr(ExternalKind kind)
{
    switch (kind) {
    case FunctionKind:
        return "Function";

    case TableKind:
        return "Table";

    case MemoryKind:
        return "Memory";

    case GlobalKind:
        return "Global";
    }

    return "(unknown)";
}
