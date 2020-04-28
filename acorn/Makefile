include ../common.mk

ACORN_OBJDIR=$(OBJDIR)/acorn
ACORN_TESTDIR=$(ACORN_OBJDIR)/tests
DIRS=$(ACORN_OBJDIR) $(ACORN_TESTDIR)

SOURCES=mem.c       \
        string.c    \
        error.c     \
        array.c     \
        fmt.c       \
        print.c     \

TEST_SOURCES=string_test.c  \
             fmt_test.c     \
             array_test.c

OBJECTS=$(patsubst %,$(ACORN_OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))
TEST_OBJECTS=$(patsubst %,$(ACORN_TESTDIR)/%,$(patsubst %.c,%.o,$(TEST_SOURCES)))

# <file>_test.o => $OAK_TESTDIR/<file>_test
TEST_PROGRAMS=$(patsubst %test.o,%test,$(TEST_OBJECTS))
TARGET=$(OBJDIR)/lib/libacorn.a


all: $(DIRS) $(TEST_PROGRAMS) $(TARGET) tests
	@echo done


tests: $(DIRS) $(TEST_PROGRAMS)
	@for x in $(ACORN_TESTDIR)/*_test; do       \
                $$x;                                \
                if [ $$? = "0" ]; then              \
                        echo "ok";                  \
                else                                \
                        echo "fail"; exit 1;        \
                fi;                                 \
        done


$(ACORN_OBJDIR):
	@mkdir -p $(ACORN_OBJDIR)


$(ACORN_TESTDIR):
	@mkdir -p $(ACORN_TESTDIR)


$(ACORN_OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@


$(ACORN_TESTDIR)/%_test.o: %_test.c
	$(CC) -c $(CFLAGS) $< -o $@


$(ACORN_TESTDIR)/%_test: $(ACORN_TESTDIR)/%_test.o $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) $< -o $@


$(TARGET): $(OBJECTS)
	$(AR) rcs $(TARGET) $(OBJECTS)
