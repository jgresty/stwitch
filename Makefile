TARGET = stwitch

FFMPEG = "`which ffmpeg`"

CC = gcc
CFLAGS = -Wall -I. -std=c11 -D _XOPEN_SOURCE -D FFMPEG=$(FFMPEG) `pkg-config --cflags gtk+-3.0`

LINKER = gcc -o
LFLAGS = -Wall -I. -lm `pkg-config --libs gtk+-3.0`

SRCDIR = .
OBJDIR = .
BINDIR = .

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm = rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
