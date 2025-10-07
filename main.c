/*
===============================
AF_Docs - Markdown to HTML Tool
Author: jhall.develop
License: MIT

This tool converts all .md files in a specified directory to .html files using the MD4C library.
It preserves the directory structure and creates necessary directories in the output path.
It also adds a simple HTML header and footer to each converted file.
Its default input directory is ./docs and default output directory is ./bin/public.

Usage:
  ./AF_Docs [input_directory] [output_directory]
If no arguments are provided, it uses the default directories.
For help, run:
  ./AF_Docs -h
===============================
*/
#include "AF_Docs.h"
#include <string.h>


int main(int argc, char* argv[]){
    printf("=== AF_Docs - Markdown to HTML Converter ===\n");

    // Parse command-line arguments
    const char* input_dir = DEFAULT_INPUT_DIR;
    const char* output_dir = DEFAULT_OUTPUT_DIR;

    // Check for help flag or too many arguments
    if(argc >= 2){
        if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
            AF_DOCS_Print_Usage(argv[0]);
            return 0;
        }
        input_dir = argv[1];
    }

    // Optional output directory
    if(argc >= 3){
        output_dir = argv[2];
    }

    // Too many arguments
    if(argc > 3){
        printf("Error: Too many arguments.\n\n");
        AF_DOCS_Print_Usage(argv[0]);
        return 1;
    }

    printf("Input directory:  %s\n", input_dir);
    printf("Output directory: %s\n\n", output_dir);

    // Create output directory if it doesn't exist
    if(AF_DOCS_Create_Directory_Recursive(output_dir) != 0){
        printf("Failed to create output directory: %s\n", output_dir);
        return 1;
    }

    // Process all markdown files in the directory
    int result = AF_DOCS_Process_Directory(input_dir, output_dir);
    
    if(result == 0){
        printf("\n=== Conversion completed successfully! ===\n");
    } else {
        printf("\n=== Conversion completed with errors ===\n");
    }

    return result;
}


