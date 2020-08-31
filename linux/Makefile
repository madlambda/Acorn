# Copyright (C) Madlambda Authors.


include $(PWD)/common.mk


DEBUG=1
LINUX_TESTDIR = "$(OBJDIR)/linux/tests"

TEST_SOURCES  = acorn_test.c
TEST_OBJECTS  = $(patsubst %,$(LINUX_TESTDIR)/%,$(patsubst %.c,%.o,$(TEST_SOURCES)))
TEST_PROGRAMS = $(patsubst %test.o,%test,$(TEST_OBJECTS))

obj-m += acorn.o

acorn-objs = init.o fs.o

CFLAGS_init.o 	= -DDEBUG=$(DEBUG) $(ACORN_INCLUDES)
CFLAGS_fs.o 	= -DDEBUG=$(DEBUG) $(ACORN_INCLUDES)

TARGET = acorn.ko


all: clean $(TARGET)
	@echo "ok"

$(TARGET):
	make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) modules

tests: $(LINUX_TESTDIR) clean $(TARGET) insmod  $(TEST_PROGRAMS) test rmmod

test:
	@for x in $(LINUX_TESTDIR)/*_test; do        \
			testname=`basename $$x`;             \
            $$x;                                 \
            if [ "$$?" = "0" ]; then             \
                echo "$$testname: ok";           \
            else                                 \
                echo "$$testname: fail";         \
				dmesg | tail -n5;                \
				exit 1;                          \
            fi;                                  \
    done

.PHONY: insmod
insmod: rmmod
	sudo insmod $(TARGET) && (dmesg | tail -n2);
	@sudo chmod 666 /dev/acorn

.PHONY: rmmod
rmmod:
	@sudo rmmod acorn 2>/dev/null || true

$(LINUX_TESTDIR)/%_test.o: %_test.c
	$(CC) -c $(ACORN_CFLAGS) \
	-DACORN_MODULE_MAJOR_NR=$(shell cat /proc/devices | grep acorn | cut -d' ' -f1) \
	$< -o $@

$(LINUX_TESTDIR)/%_test: $(LINUX_TESTDIR)/%_test.o
	$(CC) $(ACORN_LDFLAGS) $< -o $@


$(LINUX_TESTDIR):
	@mkdir -p $(LINUX_TESTDIR)

.PHONY: clean
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) clean
