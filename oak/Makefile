# Copyright (C) Madlambda Authors.


include ../common.mk


OAK_OBJDIR=$(OBJDIR)/oak
OAK_TESTDIR=$(OAK_OBJDIR)/tests
DIRS=$(OAK_OBJDIR) $(OAK_TESTDIR)
LIBS=$(OBJDIR)/lib/libacorn.a


SOURCES=test.c     \
        bin.c      \
        file.c     \
        module.c   \
        instance.c \
        jit.c      \
        x64.c      \
        jit_x64.c  \
        fmt.c      \


TEST_SOURCES=   bin_test.c      \
                module_test.c   \
                jit_x64_test.c  \
#                instance_test.c \
#                jit_test.c      \



ASM_TESTDATA=   testdata/ok/call1/output.s \
                testdata/ok/emptyfunc/output.s \
                testdata/ok/x64/mov.s \


LIBOAK=$(OBJDIR)/lib/liboak.a


# <file>.c => $OAK_OBJDIR/<file>.o
OBJECTS=$(patsubst %,$(OAK_OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))
TEST_OBJECTS=$(patsubst %,$(OAK_TESTDIR)/%,$(patsubst %.c,%.o,$(TEST_SOURCES)))
ASM_TESTOBJECTS=$(patsubst %.s,%.jit,$(ASM_TESTDATA))

# <file>_test.o => $OAK_TESTDIR/<file>_test
TEST_PROGRAMS=$(patsubst %test.o,%test,$(TEST_OBJECTS))


all: $(DIRS) clean-objtest $(ASM_TESTOBJECTS) $(TEST_PROGRAMS) tests $(LIBOAK)
	@echo done


tests: $(DIRS) $(TEST_PROGRAMS)
	@for x in $(OAK_TESTDIR)/*_test; do          \
			testname=`basename $$x`;             \
            $$x;                                 \
            if [ "$$?" = "0" ]; then             \
                echo "$$testname: ok";           \
            else                                 \
                echo "$$testname: fail"; exit 1; \
            fi;                                  \
    done


$(OAK_OBJDIR):
	@mkdir -p $(OAK_OBJDIR)


$(OAK_TESTDIR):
	@mkdir -p $(OAK_TESTDIR)


$(OAK_OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.jit: %.s
	$(AS) $< -o $@


$(OAK_TESTDIR)/%_test.o: %_test.c
	$(CC) -c $(CFLAGS) $< -o $@


$(OAK_TESTDIR)/%_test: $(OAK_TESTDIR)/%_test.o $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) $< -o $@ -Wl,-lcapstone


$(LIBOAK): $(OBJECTS)
	$(AR) rcs $(LIBOAK) $(OBJECTS)


.PHONY: clean-objtest
clean-objtest:
	rm -f $(ASM_TESTOBJECTS)
