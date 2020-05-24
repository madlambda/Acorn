/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _OAK_INSTANCE_H_
#define _OAK_INSTANCE_H_


#define nlocalitems(fn)                                                       \
    (((fn)->code) ? (len((fn)->sig.params) + len((fn)->code->locals)) :       \
        (len((fn)->sig.params)))


typedef struct {
    Type            type;
    union {
        i32         ival;
    } u;
} Value;


typedef struct {
    u8              *data;
    u32             len;
} Memory;


typedef void *(*Getfn)(void *instance, u32 index);


typedef struct {
    Module          *module;
    Memory          mem;

    Array           *imports;
    Array           *funcs;

    Array           *stack;     /* of Value (used per call) */

    Getfn           getfn;
} Instance;


typedef struct {
    Array           *locals;    /* of Value (args + locals) */
    Array           *returns;   /* of Value */
} Local;


typedef struct sFunction  Function;
typedef Error *(*Funcall)(Function *fn, Local *local);



typedef struct {
    String          module;
    String          field;
    ExternalKind    kind;
    Funcall         fn;
} Import;


Error   *instantiate(Instance *instance, Array *imports);
Error   *vminvoke(Instance *ins, const char *func, Array *params,
            Array *returns);
Error   *interp(Function *fn, Local *local);
void    closeinstance(Instance *instance);


#endif /* _OAK_INSTANCE_H_ */
