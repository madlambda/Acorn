/*
 * Copyright (C) Madlambda Authors.
 */

#include <acorn.h>
#include <acorn/array.h>
#include <oak/module.h>
#include <oak/instance.h>
#include "jit.h"
#include "x64.h"
#include "test.h"

#include <capstone/capstone.h>


typedef struct {
    u8   u8val;
    u16  u16val;
    u32  u32val;
    u64  u64val;
} Data;


typedef struct {
    Reg         start;
    Reg         end;
    Insencoder  enc;
} Testrange;


typedef Error *(*testencoder)(Jitfn *j);


typedef struct {
    testencoder  testfn;
    const char   *want;
    const char   *err;
} Testcase;


typedef void (*Fn)(Data *d);


static Error *test(const Testcase *tc);
static Error *test_execbasic();
static Error *test_execinitlocal();
static Error *test_add(Jitfn *j);
static Error *test_mov(Jitfn *j);
static Error *test_lea(Jitfn *j);
static Error *test_call(Jitfn *jit);

static Error *callback(Function *fn, Local *locals);


static const Testrange  movranges[8] = {
    {.start = AL,   .end = BH,      .enc = movb},
    {.start = AX,   .end = DI,      .enc = movw},
    {.start = EAX,  .end = EDI,     .enc = movl},
    {.start = RAX,  .end = RDI,     .enc = movq},
    {.start = R8B,  .end = R15B,    .enc = movb},
    {.start = R8W,  .end = R15W,    .enc = movw},
    {.start = R8D,  .end = R15D,    .enc = movl},
    {.start = R8,   .end = R15,     .enc = movq},
};


static const Testcase  testcases[] = {
    {
        .testfn = test_add,
        .want   = "testdata/ok/x64/add.jit",
        .err    = NULL,
    },
    {
        .testfn = test_mov,
        .want   = "testdata/ok/x64/mov.jit",
        .err    = NULL,
    },
    {
        .testfn = test_lea,
        .want   = "testdata/ok/x64/lea.jit",
        .err    = NULL,
    },
    {
        .testfn = test_call,
        .want   = "testdata/ok/x64/call.jit",
        .err    = NULL,
    },
};


int
main()
{
    u32    i;
    Error  *err;

    fmtadd('e', errorfmt);
    fmtadd('J', fmtjit);
    fmtadd('C', fmtjitcmp);

    err = test_execbasic();
    if (slow(err != NULL)) {
        cprint("error: %e\n", err);
        errorfree(err);
        return 1;
    }

    err = test_execinitlocal();
    if (slow(err != NULL)) {
        cprint("error: %e\n", err);
        errorfree(err);
        return 1;
    }

    for (i = 0; i < nitems(testcases); i++) {
        err = test(&testcases[i]);
        if (slow(err != NULL)) {
            cprint("error: %e\n", err);
            errorfree(err);
            return 1;
        }
    }

    return 0;
}


#define min(a, b) ((a) < (b) ? (a) : (b))


static Error *
test_execbasic()
{
    Fn        fn;
    Data      data;
    Error     *err;
    Jitfn     jit;
    Jitvalue  arg;

    err = allocrw(&jit, 4096);
    if (slow(err != NULL)) {
        return error(err, "allocating jit data");
    }

    jit.begin = jit.data;
    jit.end = (jit.data + jit.size);

    /*
     * _start:
     * mov $1, (%rdi)
     * add $<u16offset>, %rdi
     * mov $1, %rdi
     * add $<u32offset>, %rsi
     * mov $1, %rdi
     * add $<u64offset>, %rdi
     * mov $1, %rdi
     * ret
     */

    memset(&arg, 0, sizeof(Jitvalue));
    memset(&data, 0, sizeof(Data));

    immind(&arg, 1, RDI, 0);
    err = movb(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: mov encode");
    }

    immreg(&arg, offsetof(Data, u16val), RDI);
    err = add(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: add encode");
    }

    immind(&arg, 1, RDI, 0);
    err = movb(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: mov encode");
    }

    immreg(&arg, offsetof(Data, u32val) - offsetof(Data, u16val), RDI);
    err = add(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: add encode");
    }

    immind(&arg, 1, RDI, 0);
    err = movb(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: mov encode");
    }

    immreg(&arg, offsetof(Data, u64val) - offsetof(Data, u32val), RDI);
    err = add(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: add encode");
    }

    immind(&arg, 1, RDI, 0);
    err = movb(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: mov encode");
    }

    err = ret(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: ret encode");
    }

    err = mkexec(&jit);
    if (slow(err != NULL)) {
        return error(err, "making executable memory");
    }

    fn = NULL;

    copyptr((ptr) &fn, (ptr) &jit.data);

    fn(&data);

    if (slow(data.u8val != 1 || data.u16val != 1 || data.u32val != 1
             || data.u64val != 1))
    {
        cprint("%d(u8),%d(u16),%d(u32),%d(u64)\n", data.u8val, data.u16val,
               data.u32val, data.u64val);
        return newerror("unexpected data");
    }

    freejit(&jit);

    return NULL;
}


static Error *
callback(Function * unused(fn), Local *locals)
{
    u32  i;

    for (i = 0; i < len(locals->locals); i++) {
        cprint("called with value: %d", i);
    }

    if (slow(locals->locals->nalloc != 2)) {
        return newerror("invalid number of locals");
    }

    if (slow(locals->returns->nalloc != 1)) {
        return newerror("invalid number of returns");
    }

    cprint("callback called from asm\n");

    return NULL;
}


static Error *
test_execinitlocal()
{
    u32       stacksize, nlocals, nrets, localsoff, retsoff;
    Data      data;
    Error     *err;
    Jitfn     jit;
    Local     locals;
    Funcall   fncall;
    Function  func;
    Jitvalue  arg;

    err = allocrw(&jit, 4096);
    if (slow(err != NULL)) {
        return error(err, "allocating jit data");
    }

    jit.begin = jit.data;
    jit.end = (jit.data + jit.size);

    /*
     * _start:
     * add $0x10 + sizeof(Local) + arraytotalsize(nlocals, sizeof(Value))
     *     + arraytotalsize(nrets, sizeof(Value));
     * ret
     */

    memset(&arg, 0, sizeof(Jitvalue));
    memset(&data, 0, sizeof(Data));

    nlocals = 2;
    nrets = 1;

    stacksize = 0x10 + sizeof(Local) + arraytotalsize(nlocals, sizeof(Value))
                + arraytotalsize(nrets, sizeof(Value));

    arg.stacksize = &stacksize;

    err = prologue(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "prologue");
    }

    /* RCX = localbuf */
    indreg(&arg, 0x18, RSP, RCX);
    err = lea(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq 0x18(rsp), rcx");
    }

    /* RCX = localbuf.locals */
    immreg(&arg, offsetof(Local, locals), RCX);
    err = add(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $%d, RCX", offsetof(Local, locals));
    }

    localsoff = 0x18 + sizeof(Local);

    /* RDX = localsarraybuf */
    indreg(&arg, localsoff, RSP, RDX);
    err = lea(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "lea $%d, RDX", localsoff);
    }

    /* localsbuf.locals = localsarraybuf */
    regind(&arg, RDX, RCX, 0);
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq RDX, (RCX)");
    }

    immind(&arg, 0, RDX, offsetof(Array, len));
    err = movl(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $%d, %d(RDX)", 0, offsetof(Array, len));
    }

    immind(&arg, nlocals, RDX, offsetof(Array, nalloc));
    err = movl(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $%d, %d(RDX)", nlocals, offsetof(Array, nalloc));
    }

    immind(&arg, sizeof(Value), RDX, offsetof(Array, size));
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $%d, %d(RDX)", sizeof(Value), offsetof(Array, size));
    }

    immind(&arg, 0, RDX, offsetof(Array, heap));
    err = movb(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $0, %d(RDX)", offsetof(Array, heap));
    }

    localsoff += sizeof(Array);
    indreg(&arg, localsoff, RSP, RCX);
    err = lea(&jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    regind(&arg, RCX, RDX, offsetof(Array, items));
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq RCX, %d(RDX)", offsetof(Array, items));
    }

    /* RCX = localbuf */
    indreg(&arg, 0x18, RSP, RCX);
    err = lea(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq 0x18(rsp), rcx");
    }

    /* RCX = localbuf.returns */
    immreg(&arg, offsetof(Local, returns), RCX);
    err = add(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $%d, RCX", offsetof(Local, returns));
    }

    /* RDX = returnsarraybuf */
    retsoff = localsoff + (sizeof(Value) * nlocals);
    indreg(&arg, retsoff, RSP, RDX);
    err = lea(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "lea $%d, RDX", localsoff);
    }

    /* localbuf.returns = returnsarraybuf */
    regind(&arg, RDX, RCX, 0);
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov RDX, (RCX)");
    }

    immind(&arg, 0, RDX, offsetof(Array, len));
    err = movl(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $%d, %d(RDX)", 0, offsetof(Array, len));
    }

    immind(&arg, nrets, RDX, offsetof(Array, nalloc));
    err = movl(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $%d, %d(RDX)", nlocals, offsetof(Array, nalloc));
    }

    immind(&arg, sizeof(Value), RDX, offsetof(Array, size));
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $%d, %d(RDX)", sizeof(Value), offsetof(Array, size));
    }

    immind(&arg, 0, RDX, offsetof(Array, heap));
    err = movb(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq $0, %d(RDX)", offsetof(Array, heap));
    }

    retsoff += sizeof(Array);
    indreg(&arg, localsoff, RSP, RCX);
    err = lea(&jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    regind(&arg, RCX, RDX, offsetof(Array, items));
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq RCX, %d(RDX)", offsetof(Array, items));
    }

    indreg(&arg, 0, RSP, RAX);
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq 0(RSP), RAX");
    }

    indreg(&arg, 0, RAX, RAX);
    err = movq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "movq 0(RAX), RAX");
    }

    immreg(&arg, 0, RDI);
    movq(&jit, &arg);

    indreg(&arg, 0x18, RSP, RSI);
    lea(&jit, &arg);

    relreg(&arg, RAX);
    err = callq(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "callq");
    }

    err = epilogue(&jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "exec test: ret encode");
    }

    err = mkexec(&jit);
    if (slow(err != NULL)) {
        return error(err, "making executable memory");
    }

    fncall = NULL;

    copyptr((ptr) &fncall, (ptr) &jit.data);

    func.fn = callback;

    err = fncall(&func, &locals);
    if (slow(err != NULL)) {
        return error(err, "exec fn");
    }

    freejit(&jit);

    return NULL;
}


static Error *
test(const Testcase *tc)
{
    File     file;
    Error    *err;
    Jitfn    jit;
    Binbuf   got, want;

    err = openfile(&file, tc->want);
    if (slow(err != NULL)) {
        return err;
    }

    err = readbinary(&file, &want);
    if (slow(err != NULL)) {
        return error(err, "parsing file %s", tc->want);
    }

    err = allocrw(&jit, 4096);
    if (slow(err != NULL)) {
        return error(err, "allocating jit data");
    }

    jit.begin = jit.data;
    jit.end = (jit.data + jit.size);

    err = tc->testfn(&jit);
    if (slow(err != NULL)) {
        return error(err, "while generating code for %s", tc->want);
    }

    got.code = jit.data;
    got.size = (jit.begin - jit.data);

    if (fast(got.size == want.size
             && memcmp(got.code, want.code, got.size) == 0)) {
        closefile(&file);
        return NULL;
    }

#ifndef CS_MODE_64
    u32      i, j;
    csh      handle1, handle2;
    size_t   count1, count2;
    cs_insn  *insn1, *insn2;

	if (slow(cs_open(CS_ARCH_X86, CS_MODE_64, &handle1) != CS_ERR_OK)) {
		return newerror("failed to initialize capstone");
    }

    if (slow(cs_open(CS_ARCH_X86, CS_MODE_64, &handle2) != CS_ERR_OK)) {
		return newerror("failed to initialize capstone");
    }

    cs_option(handle1, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
    cs_option(handle2, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);

	count1 = cs_disasm(handle1, got.code, got.size, 0x1000, 0, &insn1);
    count2 = cs_disasm(handle2, want.code, want.size, 0x1000, 0, &insn2);

	if (count1 > 0 && count2 > 0) {
        printf("\n\tgot asm\t\twant asm:\n");
		for (j = 0; j < min(count1, count2); j++) {
			printf("%d: %s %s\t%s %s\t", j, insn1[j].mnemonic, insn1[j].op_str,
                   insn2[j].mnemonic, insn2[j].op_str);

            for (i = 0; i < insn1[j].size; i++) {
                printf("%x ", insn1[j].bytes[i]);
            }

            printf("\n");

            if (slow(memcmp(&insn1[j].bytes, &insn2[j].bytes, insn1[j].size) != 0)) {
                printf("got:\t");
                for (i = 0; i < insn1[j].size; i++) {
                    printf("%x ", insn1[j].bytes[i]);
                }

                printf("\nwant:\t");
                for (i = 0; i < insn2[j].size; i++) {
                    printf("%x ", insn2[j].bytes[i]);
                }

                puts("");
                break;
            }
		}

        if (count1 > count2) {
            printf("Got surplus instructions (%ld)\n", (count1 - count2));
            for (j = count2; j < count1; j++) {
			    printf("%d: %s %s\t", j, insn1[j].mnemonic, insn1[j].op_str);

                for (i = 0; i < insn1[j].size; i++) {
                    printf("%x ", insn1[j].bytes[i]);
                }

                printf("\n");
            }
        }

        if (count2 > count1) {
            printf("Want more those instructions (%ld)\n", (count2 - count1));
            for (j = count1; j < count2; j++) {
			    printf("%d: %s %s\t", j, insn2[j].mnemonic, insn2[j].op_str);

                for (i = 0; i < insn2[j].size; i++) {
                    printf("%x ", insn2[j].bytes[i]);
                }

                printf("\n");
            }
        }

		cs_free(insn1, count1);
        cs_free(insn2, count2);
	} else {
		return newerror("failed to disassemble buffer: %J", &got);
    }

	cs_close(&handle1);
    cs_close(&handle2);

#endif

    Bincmp data;

    data.a = got;
    data.b = want;

    err = newerror("jit generated mismatch [%d / %d] bytes: %C",
                   got.size, want.size, &data);

    closefile(&file);

    return err;
}


static Error *
test_add(Jitfn *jit)
{
    u32       i, j;
    Error     *err;
    Jitvalue  arg;

    memset(&arg, 0, sizeof(Jitvalue));

    for (i = AL; i <= BH; i++) {
        for (j = AL; j <= BH; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r8, r8");
            }
        }
    }

    for (i = AX; i <= DI; i++) {
        for (j = AX; j <= DI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r16, r16");
            }
        }
    }

    for (i = EAX; i <= EDI; i++) {
        for (j = EAX; j <= EDI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r32, r32");
            }
        }
    }

    for (i = RAX; i <= RDI; i++) {
        for (j = RAX; j <= RDI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r64, r64");
            }
        }
    }

    for (i = R8; i <= R15; i++) {
        for (j = R8; j <= R15; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r64_r, r64_r");
            }
        }
    }

    for (i = R8W; i <= R15W; i++) {
        for (j = R8W; j <= R15W; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r16_r, r16_r");
            }
        }
    }

    for (i = R8; i <= R15; i++) {
        for (j = RAX; j <= RDI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r64_2, r64_2");
            }
        }
    }

    for (i = AX; i <= DI; i++) {
        for (j = R8W; j <= R15W; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r16_2, r16_2");
            }
        }
    }

    for (i = R8W; i <= R15W; i++) {
        for (j = AX; j <= DI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "add r16_3, r16_3");
            }
        }
    }

    /* MemReg */

    for (i = AL; i < LASTREG; i++) {
        memreg(&arg, 0x1, i);
        err = add(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "add 0x1, r16");
        }
    }

    for (i = AL; i < LASTREG; i++) {
        memreg(&arg, 0x1337, i);
        err = add(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "add 0x1337, r16");
        }
    }

    memreg(&arg, 0x7fffffff, EAX);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add 0x7fffffff, r32");
    }

    for (i = AL; i < LASTREG; i++) {
        memreg(&arg, 0x80000000, i);
        err = add(jit, &arg);
        if (slow(err == NULL)) {
            return newerror("should overflow: add 0x80000000, reg");
        }

        errorfree(err);
    }

    /* ImmReg */

    immreg(&arg, 1, AL);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $0x1, al");
    }

    immreg(&arg, 1, AX);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $0x1, ax");
    }

    immreg(&arg, 1, EAX);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $0x1, eax");
    }

    immreg(&arg, 1, RAX);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $0x1, rax");
    }

    immreg(&arg, 1, RDI);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $0x1, rdi");
    }

    immreg(&arg, 1, R8);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "add $0x1, r8");
    }

    return NULL;
}


static Error *
test_mov(Jitfn *jit)
{
    u32              i, j, k;
    Error            *err;
    Jitvalue         arg;
    const Testrange  *range;

    memset(&arg, 0, sizeof(Jitvalue));

    /* RegReg */

    for (i = 0; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            for (k = range->start; k <= range->end; k++) {
                regreg(&arg, j, k);
                err = range->enc(jit, &arg);
                if (slow(err != NULL)) {
                    return error(err, "mov r%d, r%d", j, k);
                }
            }
        }
    }

    /* ImmReg */

    immreg(&arg, INT8_MAX, AL);
    err = movb(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $%d, r8", INT8_MAX);
    }

    arg.src.i64val++;
    err = movb(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (0x%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val++;
    err = movb(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val = 255;
    err = movb(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val++;
    err = movb(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val = -127;
    err = movb(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $-127, al");
    }

    arg.src.i64val--;
    err = movb(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $-128, al");
    }

    arg.src.i64val--;
    err = movb(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    for (i = 0; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            immreg(&arg, 0x69, j);
            err = range->enc(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov $0x69, r%d", j);
            }
        }
    }

    immreg(&arg, 65535, AX);
    err = movw(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val++;
    err = movw(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    immreg(&arg, 32767, AX);
    err = movw(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $32767, ax");
    }

    immreg(&arg, 32767, EAX);
    err = movw(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $32767, eax");
    }

    immreg(&arg, INT64_MAX, EAX);
    err = movl(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    immreg(&arg, (i64) INT32_MAX + 1, EAX);
    err = movl(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    immreg(&arg, INT32_MAX, EAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $INT32_MAX, eax");
    }

    immreg(&arg, INT32_MIN, EAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $INT32_MIN, eax");
    }

    immreg(&arg, INT64_MIN, RAX);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $INT64_MIN, rax");
    }

    immreg(&arg, INT64_MAX, RAX);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $INT64_MAX, rax");
    }

    immreg(&arg, INT64_MIN, R8);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $INT64_MIN, r8");
    }

    immreg(&arg, INT64_MAX, R8);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov $INT64_MAX, r8");
    }

    /* MemReg */

    for (i = 0; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            memreg(&arg, 1, j);
            err = range->enc(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 0x1, r%d", j);
            }
        }
    }

    /* special cases, we choose smaller instructions for small addresses */

    memreg(&arg, 0xdeadbeef, EAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov 0xdeadbeef, eax");
    }

    memreg(&arg, 0xdeadbeef, RAX);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov 0xdeadbeef, rax");
    }

    memreg(&arg, 0x7fffffff, EAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov 0x7fffffff, eax");
    }

    memreg(&arg, UINT64_MAX, RAX);
    err = movq(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("mov UINT64_MAX, rax should overflow: use movabsl");
    }

    errorfree(err);

    memreg(&arg, 0x1000, RAX);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0x7fffffff, R10);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, UINT64_MAX, R10);
    err = movq(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("mov $UINT64_MAX, r10 should fail");
    }

    errorfree(err);

    memreg(&arg, 0x1000, R10);
    err = movq(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov 0x1000, r10");
    }

    memreg(&arg, 0xffff, R10W);
    err = movw(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov 0xffff, r10w");
    }

    /* IndRegReg */

    range = &movranges[2];

    for (j = range->start; j <= range->end; j++) {
        for (k = range->start; k <= range->end; k++) {
            indreg(&arg, 0, k, j);
            err = range->enc(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 0(r%d), r%d", j, k);
            }
        }
    }

    range = &movranges[3];

    for (j = range->start; j <= range->end; j++) {
        for (k = range->start; k <= range->end; k++) {
            indreg(&arg, 0, k, j);
            err = range->enc(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 0(r%d), r%d", j, k);
            }
        }
    }

    range = &movranges[1];

    for (j = range->start; j < LASTREG; j++) {
        indreg(&arg, 0, EAX, j);
        err = range->enc(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "mov (eax), r%d", j);
        }
    }

    for (i = 1 /* AX...BH */; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            indreg(&arg, 8, EAX, j);
            err = range->enc(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 8(eax), r%d", j);
            }
        }
    }

    for (i = 1; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            indreg(&arg, 16, EAX, j);
            err = movl(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 16(eax), r%d", j);
            }
        }
    }

    for (i = 1; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            indreg(&arg, 2000, EAX, j);
            err = movl(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 2000(eax), r%d", j);
            }
        }
    }

    for (i = AX; i <= CX; i++) {
        for (j = EAX; j <= R15; j++) {
            if (j >= R8B && j <= R15W) {
                continue;
            }

            indreg(&arg, 16, j, i);
            err = movl(jit, &arg);
            if (slow(err != NULL)) {
                return error(err, "mov 16(r%d), r%d", j, i);
            }
        }
    }

    indreg(&arg, 0, EBP, CX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (ebp), cx");
    }

    indreg(&arg, 0, ESP, CX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (esp), cx");
    }

    indreg(&arg, 0, ESP, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (esp), ax");
    }

    indreg(&arg, 0, RSP, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (rsp), ax");
    }

    indreg(&arg, 0, RBP, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (rbp), ax");
    }

    indreg(&arg, 0, R8, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
       return error(err, "mov (r8), ax");
    }

    indreg(&arg, 0, R9, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r9), ax");
    }

    indreg(&arg, 0, R10, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r10), ax");
    }

    indreg(&arg, 0, R11, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r11), ax");
    }

    indreg(&arg, 0, R12, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r12), ax");
    }

    indreg(&arg, 0, R13, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r13), ax");
    }

    indreg(&arg, 0, R14, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r14), ax");
    }

    indreg(&arg, 0, R15, AX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r15), ax");
    }

    indreg(&arg, 0, R8, EAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return error(err, "mov (r8), ax");
    }

    /* IndReg + displacement*/
    indreg(&arg, 8, RSP, RAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 8, RSP, RCX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 16, RSP, RCX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 1000, ESP, EAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 8, RAX, RCX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 16, RAX, RAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 16, RCX, RSP);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 127, RAX, RAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp++;
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp = 1000;
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp = -100;
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp = -1000;
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 8, RBP, RAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 9, RBP, RAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, -8, RBP, RAX);
    err = movl(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    /* RegInd */

    for (i = 0; i < LASTREG; i++) {
        regind(&arg, i, RAX, 0);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = 0; i < LASTREG; i++) {
        regind(&arg, i, RAX, 8);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = 0; i < LASTREG; i++) {
        regind(&arg, i, RAX, -8);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = 0; i < LASTREG; i++) {
        regind(&arg, i, RAX, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = 0; i < LASTREG; i++) {
        regind(&arg, i, RBP, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = 0; i < LASTREG; i++) {
        regind(&arg, i, RSP, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    /* RegMem */

    for (i = AL; i < LASTREG; i++) {
        regmem(&arg, i, 0x69);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = 0; i < nitems(movranges); i++) {
        range = &movranges[i];

        for (j = range->start; j <= range->end; j++) {
            regmem(&arg, j, 0x7fffffff);
            err = range->enc(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    /* TODO(i4k): test address bigger than INT32_MAX */

    for (i = EAX; i <= EDI; i++) {
        immind(&arg, 1, i, 0);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = RAX; i <= RDI; i++) {
        immind(&arg, 1, i, 0);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8D; i <= R15D; i++) {
        immind(&arg, 1, i, 0);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8; i <= R15; i++) {
        immind(&arg, 1, i, 0);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = EAX; i <= EDI; i++) {
        immind(&arg, 1, i, 8);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = RAX; i <= RDI; i++) {
        immind(&arg, 1, i, 8);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8D; i <= R15D; i++) {
        immind(&arg, 1, i, 8);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8; i <= R15; i++) {
        immind(&arg, 1, i, 8);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = EAX; i <= EDI; i++) {
        immind(&arg, 1, i, 16);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = RAX; i <= RDI; i++) {
        immind(&arg, 1, i, 16);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8D; i <= R15D; i++) {
        immind(&arg, 1, i, 16);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8; i <= R15; i++) {
        immind(&arg, 1, i, 16);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = EAX; i <= EDI; i++) {
        immind(&arg, 1, i, 1000);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = RAX; i <= RDI; i++) {
        immind(&arg, 1, i, 1000);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8D; i <= R15D; i++) {
        immind(&arg, 1, i, 1000);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8; i <= R15; i++) {
        immind(&arg, 1, i, 1000);
        err = movb(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = EAX; i <= EDI; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movl(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = RAX; i <= RDI; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movl(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8D; i <= R15D; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movl(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8; i <= R15; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movl(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = EAX; i <= EDI; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = RAX; i <= RDI; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8D; i <= R15D; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = R8; i <= R15; i++) {
        immind(&arg, 0xffffff, i, 1000);
        err = movq(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    return NULL;
}


static Error *
test_lea(Jitfn *jit)
{
    Reg       i;
    Error     *err;
    Jitvalue  arg;

    memset(&arg, 0, sizeof(Jitvalue));

    for (i = AX; i <= RDI; i++) {
        indreg(&arg, 0, RAX, i);
        err = lea(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "lea");
        }
    }

    for (i = R8; i <= R15; i++) {
        indreg(&arg, 0, RAX, i);
        err = lea(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "lea");
        }
    }

    for (i = R8D; i <= R15D; i++) {
        indreg(&arg, 0, RAX, i);
        err = lea(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "lea");
        }
    }

    for (i = EAX; i <= RDI; i++) {
        indreg(&arg, 0, i, RAX);
        err = lea(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "lea");
        }
    }

    for (i = EAX; i <= RDI; i++) {
        indreg(&arg, 8, i, RAX);
        err = lea(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "lea");
        }
    }

    for (i = EAX; i <= RDI; i++) {
        indreg(&arg, 1000, i, RAX);
        err = lea(jit, &arg);
        if (slow(err != NULL)) {
            return error(err, "lea");
        }
    }

    return NULL;
}


static Error *
test_call(Jitfn *jit)
{
    Error     *err;
    Jitvalue  arg;

    memset(&arg, 0, sizeof(Jitvalue));

    relreg(&arg, RAX);
    err = callq(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    return NULL;
}
