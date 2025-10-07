CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRCDIR = src
INCDIR = include
LIBDIR = libs/md4c/src
BUILDDIR = build
BINDIR = bin
PUBLICDIR = public
DOCSDIR = docs
HTTPDIR = libs/AF_HTTP

SOURCES = main.c $(wildcard $(SRCDIR)/*.c) $(wildcard $(LIBDIR)/*.c)
HEADERS = $(wildcard $(INCDIR)/*.h) $(wildcard $(LIBDIR)/*.h)

# Create object file paths in build directory
MAIN_OBJ = $(BUILDDIR)/main.o
SRC_OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(wildcard $(SRCDIR)/*.c))
LIB_OBJS = $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(wildcard $(LIBDIR)/*.c))
OBJECTS = $(MAIN_OBJ) $(SRC_OBJS) $(LIB_OBJS)

TARGET = $(BINDIR)/AF_Docs
HTTP_TARGET = $(BINDIR)/AF_HTTP

all: $(TARGET) $(HTTP_TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET)

# Build AF_HTTP and copy to bin directory
$(HTTP_TARGET): | $(BINDIR)
	@echo "Building AF_HTTP..."
	$(MAKE) -C $(HTTPDIR)
	cp $(HTTPDIR)/bin/AF_HTTP $(HTTP_TARGET)
	@echo "AF_HTTP copied to $(BINDIR)/"

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET)

$(BUILDDIR)/main.o: main.c $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBDIR) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBDIR) -c $< -o $@

$(BUILDDIR)/%.o: $(LIBDIR)/%.c $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBDIR) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
	$(MAKE) -C $(HTTPDIR) clean

.PHONY: all clean