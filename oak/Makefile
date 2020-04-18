# Copyright (C) Madlambda Authors.


include ../common.mk


OAK_OBJDIR=$(OBJDIR)/oak
OAK_TESTDIR=$(OAK_OBJDIR)/tests
DIRS=$(OAK_OBJDIR) $(OAK_TESTDIR)
INCLUDES=-I.
LIBS=$(OBJDIR)/lib/libacorn.a


SOURCES=oak_test.c	\
		oak_bin.c	\


TEST_SOURCES=oak_bin_test.c


# <file>.c => $OAK_OBJDIR/<file>.o
OBJECTS=$(patsubst %,$(OAK_OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))
TEST_OBJECTS=$(patsubst %,$(OAK_TESTDIR)/%,$(patsubst %.c,%.o,$(TEST_SOURCES)))

# <file>_test.o => $OAK_TESTDIR/<file>_test
TEST_PROGRAMS=$(patsubst %test.o,%test,$(TEST_OBJECTS))


all: $(DIRS) $(TEST_PROGRAMS) tests
	@echo done


tests: $(DIRS) $(TEST_PROGRAMS)
	@for x in $(OAK_TESTDIR)/*_test; do \
		$$x; 							\
		if [ $$? = "0" ]; then 			\
			echo "ok"; 					\
		fi; 							\
	done


$(OAK_OBJDIR):
	@mkdir -p $(OAK_OBJDIR)


$(OAK_TESTDIR):
	@mkdir -p $(OAK_TESTDIR)


$(OAK_OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@


$(OAK_TESTDIR)/%_test.o: %_test.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@


$(OAK_TESTDIR)/%_test: $(OAK_TESTDIR)/%_test.o $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) $< -o $@