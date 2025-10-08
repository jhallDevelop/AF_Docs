# ============================================================================
# AF_Docs Makefile
# Author: jhall.develop
# License: MIT
#
# Builds AF_Docs markdown converter and AF_HTTP preview server
# Usage: make [-j]    Build everything
#        make clean   Remove build artifacts
#        make help    Show available targets
# ============================================================================

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# Directories
SRCDIR = src
INCDIR = include
LIBDIR = libs/md4c/src
BUILDDIR = build
BINDIR = bin
PUBLICDIR = public
DOCSDIR = docs
HTTPDIR = libs/AF_HTTP

# Source files
SOURCES = main.c $(wildcard $(SRCDIR)/*.c) $(wildcard $(LIBDIR)/*.c)
HEADERS = $(wildcard $(INCDIR)/*.h) $(wildcard $(LIBDIR)/*.h)

# Object files
MAIN_OBJ = $(BUILDDIR)/main.o
SRC_OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(wildcard $(SRCDIR)/*.c))
LIB_OBJS = $(patsubst $(LIBDIR)/%.c,$(BUILDDIR)/%.o,$(wildcard $(LIBDIR)/*.c))
OBJECTS = $(MAIN_OBJ) $(SRC_OBJS) $(LIB_OBJS)

# Build targets
TARGET = $(BINDIR)/AF_Docs
HTTP_TARGET = $(BINDIR)/AF_HTTP

# ============================================================================
# Build Rules
# ============================================================================

all: $(TARGET) $(HTTP_TARGET)

# Link AF_Docs binary
$(TARGET): $(OBJECTS) | $(BINDIR)
	@echo "Linking AF_Docs..."
	$(CC) $(OBJECTS) -o $(TARGET)
	@echo "AF_Docs built: $(TARGET)"

# Build AF_HTTP from submodule
$(HTTP_TARGET): | $(BINDIR)
	@echo "Building AF_HTTP..."
	$(MAKE) -C $(HTTPDIR)
	cp $(HTTPDIR)/bin/AF_HTTP $(HTTP_TARGET)
	@echo "AF_HTTP copied to $(BINDIR)/"

# Compile object files
$(BUILDDIR)/main.o: main.c $(HEADERS) | $(BUILDDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBDIR) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILDDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBDIR) -c $< -o $@

$(BUILDDIR)/%.o: $(LIBDIR)/%.c $(HEADERS) | $(BUILDDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBDIR) -c $< -o $@

# Create directories
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILDDIR) $(BINDIR)
	@$(MAKE) -C $(HTTPDIR) clean
	@echo "Clean complete"

# Show help
help:
	@echo "AF_Docs Makefile Targets:"
	@echo "  make          Build AF_Docs and AF_HTTP"
	@echo "  make clean    Remove build artifacts"
	@echo "  make help     Show this help"
	@echo "  make -j       Build with parallel compilation"

.PHONY: all clean help