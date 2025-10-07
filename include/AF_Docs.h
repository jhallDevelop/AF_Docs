/*
===============================
AF_Docs - Header File
Author: jhall.develop
License: MIT

Public API for the AF_Docs markdown to HTML converter.
===============================
*/
#ifndef AF_DOCS_H
#define AF_DOCS_H

#include <stdint.h>
#include <stdio.h>

// Structure to hold HTML output buffer
typedef struct {
    char* data;       // Dynamically allocated buffer
    size_t size;      // Current size of data
    size_t capacity;  // Total allocated capacity
} html_buffer_t;

// Default paths (defined in AF_Docs.c)
extern const char* DEFAULT_INPUT_DIR;
extern const char* DEFAULT_OUTPUT_DIR;

// ====================
// Public API Functions
// ====================

/**
 * Convert a single markdown file to HTML
 * @param input_path  - Path to input .md file
 * @param output_path - Path to save output .html file
 * @return 0 on success, 1 on error
 */
uint32_t AF_DOCS_CreateDocs(const char* input_path, const char* output_path);

/**
 * Read markdown file into dynamically allocated buffer
 * @param _filePath  - Path to markdown file
 * @param _bufferPtr - Receives pointer to allocated buffer (caller must free)
 * @return Number of bytes read, or 0 on error
 */
uint32_t AF_DOCS_Read_MD(const char* _filePath, char** _bufferPtr);

/**
 * Print usage information and examples
 * @param program_name - Name of the executable (from argv[0])
 */
void AF_DOCS_Print_Usage(const char* program_name);

/**
 * Recursively process all .md files in a directory
 * @param input_dir  - Source directory containing .md files
 * @param output_dir - Destination directory for .html files
 * @return 0 on success, 1 if any errors occurred
 */
int AF_DOCS_Process_Directory(const char* input_dir, const char* output_dir);

/**
 * Replace file extension (e.g., "file.md" -> "file.html")
 * @param filename - Original filename with extension
 * @param new_ext  - New extension (e.g., ".html")
 * @return Newly allocated string (caller must free), or NULL on error
 */
char* AF_DOCS_Replace_Extension(const char* filename, const char* new_ext);

/**
 * Create directory recursively (like mkdir -p)
 * @param path - Full path to directory to create
 * @return 0 on success
 */
int AF_DOCS_Create_Directory_Recursive(const char* path);

#endif // AF_DOCS_H