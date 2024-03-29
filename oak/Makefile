# Copyright (C) Madlambda Authors.


include ../common.mk


OAK_OBJDIR=$(OBJDIR)/oak
OAK_TESTDIR=$(OAK_OBJDIR)/tests
DIRS=$(OAK_OBJDIR) $(OAK_TESTDIR)
LIBS=$(OBJDIR)/lib/libacorn.a


SOURCES=test.c   \
        bin.c    \
        file.c   \
        module.c \
        fmt.c    \


TEST_SOURCES=   bin_test.c    \
                module_test.c


LIBOAK=$(OBJDIR)/lib/liboak.a


# <file>.c => $OAK_OBJDIR/<file>.o
OBJECTS=$(patsubst %,$(OAK_OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))
TEST_OBJECTS=$(patsubst %,$(OAK_TESTDIR)/%,$(patsubst %.c,%.o,$(TEST_SOURCES)))

# <file>_test.o => $OAK_TESTDIR/<file>_test
TEST_PROGRAMS=$(patsubst %test.o,%test,$(TEST_OBJECTS))


all: $(DIRS) $(TEST_PROGRAMS) tests $(LIBOAK)
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


$(OAK_TESTDIR)/%_test.o: %_test.c
	$(CC) -c $(CFLAGS) $< -o $@


$(OAK_TESTDIR)/%_test: $(OAK_TESTDIR)/%_test.o $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) $< -o $@


$(LIBOAK): $(OBJECTS)
	$(AR) rcs $(LIBOAK) $(OBJECTS)
