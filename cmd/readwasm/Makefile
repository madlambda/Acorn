# Copyright (C) Madlambda Authors.

include ../../common.mk

READWASM_OBJDIR=$(OBJCMDDIR)/readwasm
DIRS=$(READWASM_OBJDIR)
LIBS=$(OBJDIR)/lib/libacorn.a $(OBJDIR)/lib/liboak.a


SOURCES=main.c
TARGET=$(OBJDIR)/bin/readwasm
OBJECTS=$(patsubst %,$(READWASM_OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))


all: $(DIRS) $(TARGET)
	@echo done


$(READWASM_OBJDIR):
	@mkdir -p $(READWASM_OBJDIR)


$(READWASM_OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@


$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
