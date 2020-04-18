include ../common.mk

SOURCES=mem.c \
		print.c \

OBJECTS=$(patsubst %,$(OBJDIR)/acorn/%,$(patsubst %.c,%.o,$(SOURCES)))
TARGET=$(OBJDIR)/lib/libacorn.a

all: $(OBJDIR)/acorn $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $(TARGET) $(OBJECTS)

$(OBJDIR)/acorn/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/acorn:
	mkdir -p "$(OBJDIR)/acorn"


