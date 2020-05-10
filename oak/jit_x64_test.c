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


typedef Error *(*testencoder)(Jitfn *j);


typedef struct {
    testencoder  testfn;
    const char   *want;
    const char   *err;
} Testcase;


static Error *test(const Testcase *tc);
static Error *test_add(Jitfn *j);
static Error *test_mov(Jitfn *j);


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
};


int
main()
{
    u32             i;
    Error           *err;

    fmtadd('e', errorfmt);
    fmtadd('J', fmtjit);
    fmtadd('C', fmtjitcmp);

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
                return err;
            }
        }
    }

    for (i = AX; i <= DI; i++) {
        for (j = AX; j <= DI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = EAX; i <= EDI; i++) {
        for (j = EAX; j <= EDI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = RAX; i <= RDI; i++) {
        for (j = RAX; j <= RDI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8; i <= R15; i++) {
        for (j = R8; j <= R15; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8W; i <= R15W; i++) {
        for (j = R8W; j <= R15W; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8; i <= R15; i++) {
        for (j = RAX; j <= RDI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = AX; i <= DI; i++) {
        for (j = R8W; j <= R15W; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8W; i <= R15W; i++) {
        for (j = AX; j <= DI; j++) {
            regreg(&arg, i, j);
            err = add(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = AL; i <= R15W; i++) {
        immreg(&arg, 0x1, i);
        err = add(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = AL; i <= R15W; i++) {
        immreg(&arg, -0x1337, i);
        err = add(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    immreg(&arg, 0x7fffffff, EAX);
    err = add(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    for (i = AL; i < LASTREG; i++) {
        immreg(&arg, 0x80000000, i);
        err = add(jit, &arg);
        if (slow(err == NULL)) {
            return newerror("should overflow");
        }

        errorfree(err);
    }

    return NULL;
}


static Error *
test_mov(Jitfn *jit)
{
    u32       i, j;
    Error     *err;
    Jitvalue  arg;

    memset(&arg, 0, sizeof(Jitvalue));

    /* RegReg */

    for (i = AL; i <= BH; i++) {
        for (j = AL; j <= BH; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = AX; i <= DI; i++) {
        for (j = AX; j <= DI; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = EAX; i <= EDI; i++) {
        for (j = EAX; j <= EDI; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = RAX; i <= RDI; i++) {
        for (j = RAX; j <= RDI; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8; i <= R15; i++) {
        for (j = R8; j <= R15; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8D; i <= R15D; i++) {
        for (j = R8D; j <= R15D; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = R8W; i <= R15W; i++) {
        for (j = R8W; j <= R15W; j++) {
            regreg(&arg, i, j);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    /* ImmReg */

    immreg(&arg, INT8_MAX, AL);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.i64val++;
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (0x%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val++;
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val = 255;
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val++;
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val = -127;
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.i64val--;
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.i64val--;
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    for (i = 0; i < LASTREG; i++) {
        immreg(&arg, 0x69, i);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    immreg(&arg, 65535, AX);
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    arg.src.i64val++;
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    immreg(&arg, 32767, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, 32767, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, INT64_MAX, EAX);
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    immreg(&arg, (i64) INT32_MAX + 1, EAX);
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.i64val);
    }

    errorfree(err);

    immreg(&arg, INT32_MAX, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, INT32_MIN, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, INT64_MIN, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, INT64_MAX, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, INT64_MIN, R8);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    immreg(&arg, INT64_MAX, R8);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    /* MemReg */

    for (i = 0; i < LASTREG; i++) {
        memreg(&arg, 1, i);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    /* special cases, we choose smaller instructions for small addresses */

    memreg(&arg, 0xdeadbeef, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0xdeadbeef, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0x7fffffff, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, UINT64_MAX, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0x1000, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0x7fffffff, R10);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0xdeadbeef, R10);
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return error(err, "only rax can be used with 64bit immediate address");
    }

    errorfree(err);

    memreg(&arg, UINT64_MAX, R10);
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return error(err, "only rax can be used with 64bit immediate address");
    }

    errorfree(err);

    memreg(&arg, 0x1000, R10);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    memreg(&arg, 0x1000, R10D);
    err = mov(jit, &arg);
    if (slow(err == NULL)) {
        return newerror("should fail (%x): overflow", arg.src.u64val);
    }

    errorfree(err);

    memreg(&arg, 0xffff, R10W);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    /* IndRegReg */

    for (i = EAX; i <= EDI; i++) {
        for (j = EAX; j <= EDI; j++) {
            indreg(&arg, 0, j, i);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = RAX; i <= RDI; i++) {
        for (j = RAX; j <= RDI; j++) {
            indreg(&arg, 0, j, i);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    for (i = AX; i < LASTREG; i++) {
        indreg(&arg, 0, EAX, i);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = AX; i < LASTREG; i++) {
        indreg(&arg, 8, EAX, i);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = AX; i < LASTREG; i++) {
        indreg(&arg, 16, EAX, i);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = AX; i < LASTREG; i++) {
        indreg(&arg, 2000, EAX, i);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = AX; i <= CX; i++) {
        for (j = EAX; j <= R15; j++) {
            indreg(&arg, 16, j, i);
            err = mov(jit, &arg);
            if (slow(err != NULL)) {
                return err;
            }
        }
    }

    indreg(&arg, 0, EBP, CX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, ESP, CX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, ESP, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, RSP, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, RBP, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R8, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R9, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R10, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R11, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R12, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R13, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R14, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R15, AX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 0, R8, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    /* IndReg + displacement*/
    indreg(&arg, 8, RSP, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 8, RSP, RCX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 16, RSP, RCX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 1000, ESP, EAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 8, RAX, RCX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 16, RAX, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 16, RCX, RSP);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 127, RAX, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp++;
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp = 1000;
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp = -100;
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    arg.src.disp = -1000;
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 8, RBP, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, 9, RBP, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    indreg(&arg, -8, RBP, RAX);
    err = mov(jit, &arg);
    if (slow(err != NULL)) {
        return err;
    }

    /* RegMem */

    for (i = AL; i < LASTREG; i++) {
        regmem(&arg, i, 0x69);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    for (i = AL; i < LASTREG; i++) {
        regmem(&arg, i, 0x7fffffff);
        err = mov(jit, &arg);
        if (slow(err != NULL)) {
            return err;
        }
    }

    /* TODO(i4k): test address bigger than INT32_MAX */

    return NULL;
}
