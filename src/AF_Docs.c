/*
===============================
AF_Docs - Implementation File
Author: jhall.develop
License: MIT

Core implementation for the AF_Docs markdown to HTML converter.
Provides functions for:
  - Reading markdown files
  - Converting markdown to HTML using md4c library
  - Recursive directory processing
  - File path manipulation
  - Directory creation
===============================
*/
#include "AF_Docs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include "md4c.h"
#include "md4c-html.h"

// Define default paths
const char* DEFAULT_INPUT_DIR = "./docs";
const char* DEFAULT_OUTPUT_DIR = "./bin/public";

// Forward declaration of internal functions
void AF_DOCS_Process_Html_Output(const MD_CHAR* text, MD_SIZE size, void* userdata);


/*
===============
AF_DOCS_CreateDocs
Create HTML documentation from a markdown file
Params:
  input_path  - Path to the input .md file
  output_path - Path to save the output .html file
Returns:
  0 on success, 1 on error
===============
*/
uint32_t AF_DOCS_CreateDocs(const char* input_path, const char* output_path){
    // Read markdown file
    char* mdContent = NULL;
    uint32_t bytesRead = AF_DOCS_Read_MD(input_path, &mdContent);

    if(bytesRead == 0 || mdContent == NULL){
        fprintf(stderr, "  Failed to read: %s\n", input_path);
        return 1;
    }

    // Initialize HTML output buffer
    html_buffer_t html_output = {0};
    html_output.capacity = bytesRead * 2;
    html_output.data = (char*)malloc(html_output.capacity);
    if(html_output.data == NULL){
        fprintf(stderr, "  Failed to allocate memory for: %s\n", input_path);
        free(mdContent);
        return 1;
    }
    html_output.size = 0;

    // Convert markdown to HTML
    int result = md_html(mdContent, bytesRead, AF_DOCS_Process_Html_Output, &html_output, 
                        MD_DIALECT_GITHUB, 0);
    
    if(result != 0){
        fprintf(stderr, "  MD conversion failed for: %s\n", input_path);
        free(mdContent);
        free(html_output.data);
        return 1;
    }

    // Combine HTML header and footer
    const char* html_header = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Converted Markdown</title>\n<link rel=\"stylesheet\" href=\"../markdown.css\" type=\"text/css\">\n</head>\n<body>\n";
    const char* html_footer = "\n</body>\n</html>";
    size_t header_len = strlen(html_header);
    size_t footer_len = strlen(html_footer);
    size_t total_size = header_len + html_output.size + footer_len;
    
    char* full_html = (char*)malloc(total_size + 1);
    if(full_html == NULL){
        fprintf(stderr, "  Failed to allocate memory for full HTML\n");
        free(mdContent);
        free(html_output.data);
        return 1;
    }
    
    memcpy(full_html, html_header, header_len);
    memcpy(full_html + header_len, html_output.data, html_output.size);
    memcpy(full_html + header_len + html_output.size, html_footer, footer_len);
    full_html[total_size] = '\0';

    // Write HTML to file
    FILE* outputFile = fopen(output_path, "w");
    if(outputFile != NULL){
        fwrite(full_html, 1, total_size, outputFile);
        fclose(outputFile);
        printf("  ✓ %s -> %s\n", input_path, output_path);
    } else {
        fprintf(stderr, "  Failed to write: %s\n", output_path);
        free(mdContent);
        free(html_output.data);
        free(full_html);
        return 1;
    }

    // Cleanup
    free(mdContent);
    free(html_output.data);
    free(full_html);

    return 0;
}

/*
===============
AF_DOCS_Process_Directory
Recursively process all .md files in a directory
Params:
  input_dir  - Source directory containing .md files
  output_dir - Destination directory for .html files
Returns:
  0 on success, 1 if any errors occurred
===============
*/
int AF_DOCS_Process_Directory(const char* input_dir, const char* output_dir){
    DIR* dir = opendir(input_dir);
    if(dir == NULL){
        fprintf(stderr, "Error: Cannot open directory: %s\n", input_dir);
        return 1;
    }

    struct dirent* entry;
    int error_count = 0;
    int file_count = 0;

    while((entry = readdir(dir)) != NULL){
        // Skip . and ..
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        // Build full input path
        char input_path[1024];
        snprintf(input_path, sizeof(input_path), "%s/%s", input_dir, entry->d_name);

        struct stat path_stat;
        if(stat(input_path, &path_stat) != 0){
            continue;
        }

        // If it's a directory, recurse
        if(S_ISDIR(path_stat.st_mode)){
            char output_subdir[1024];
            snprintf(output_subdir, sizeof(output_subdir), "%s/%s", output_dir, entry->d_name);
            
            // Create subdirectory in output
            AF_DOCS_Create_Directory_Recursive(output_subdir);
            
            // Recursively process subdirectory
            error_count += AF_DOCS_Process_Directory(input_path, output_subdir);
        }
        // If it's a .md file, convert it
        else if(S_ISREG(path_stat.st_mode)){
            const char* ext = strrchr(entry->d_name, '.');
            if(ext != NULL && strcmp(ext, ".md") == 0){
                // Replace .md extension with .html
                char* html_filename = AF_DOCS_Replace_Extension(entry->d_name, ".html");
                if(html_filename != NULL){
                    char output_path[1024];
                    snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, html_filename);
                    
                    // Convert the file
                    if(AF_DOCS_CreateDocs(input_path, output_path) == 0){
                        file_count++;
                    } else {
                        error_count++;
                    }
                    
                    free(html_filename);
                }
            }
        }
    }

    closedir(dir);
    
    if(file_count > 0){
        printf("  Processed %d file(s) from %s\n", file_count, input_dir);
    }

    return error_count > 0 ? 1 : 0;
}

/*
===============
AF_DOCS_Replace_Extension
Replace file extension (e.g., .md -> .html)
Params:
  filename - Original filename with extension
  new_ext  - New extension to apply (e.g., ".html")
Returns:
  Newly allocated string with new extension (caller must free)
  NULL on error
===============
*/
char* AF_DOCS_Replace_Extension(const char* filename, const char* new_ext){
    if(filename == NULL || new_ext == NULL){
        return NULL;
    }

    const char* dot = strrchr(filename, '.');
    size_t base_len;
    
    if(dot != NULL){
        base_len = dot - filename;
    } else {
        base_len = strlen(filename);
    }

    size_t new_len = base_len + strlen(new_ext) + 1;
    char* new_filename = (char*)malloc(new_len);
    if(new_filename == NULL){
        return NULL;
    }

    strncpy(new_filename, filename, base_len);
    new_filename[base_len] = '\0';
    strcat(new_filename, new_ext);

    return new_filename;
}

/*
===============
AF_DOCS_Create_Directory_Recursive
Create directory and all parent directories if they don't exist
Params:
  path - Full path to directory to create
Returns:
  0 on success
===============
*/
int AF_DOCS_Create_Directory_Recursive(const char* path){
    char tmp[1024];
    char* p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if(tmp[len - 1] == '/'){
        tmp[len - 1] = 0;
    }

    for(p = tmp + 1; *p; p++){
        if(*p == '/'){
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);

    return 0;
}

/*
===============
AF_DOCS_Process_Html_Output
Callback function to collect HTML output from md4c parser
Params:
  text     - HTML text chunk to append
  size     - Size of the text chunk
  userdata - Pointer to html_buffer_t structure
===============
*/
void AF_DOCS_Process_Html_Output(const MD_CHAR* text, MD_SIZE size, void* userdata){
    html_buffer_t* buffer = (html_buffer_t*)userdata;
    
    // Reallocate if needed
    while(buffer->size + size >= buffer->capacity){
        buffer->capacity *= 2;
        char* new_data = (char*)realloc(buffer->data, buffer->capacity);
        if(new_data == NULL){
            printf("Failed to reallocate HTML buffer.\n");
            return;
        }
        buffer->data = new_data;
    }
    
    // Append new data
    memcpy(buffer->data + buffer->size, text, size);
    buffer->size += size;
    buffer->data[buffer->size] = '\0'; // Null terminate
}

/*
===============
AF_DOCS_Read_MD
Read markdown file into a dynamically allocated buffer
Params:
  _filePath   - Path to the markdown file
  _bufferPtr  - Pointer to receive allocated buffer (caller must free)
Returns:
  Number of bytes read, or 0 on error
===============
*/
uint32_t AF_DOCS_Read_MD(const char * _filePath, char** _bufferPtr){
    FILE* file = fopen(_filePath, "rb");
    if(file == NULL){
        printf("Error opening file: %s\n", _filePath);
        return 0;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if(fileSize <= 0){
        printf("File is empty or error getting size: %s\n", _filePath);
        fclose(file);
        return 0;
    }

    // Allocate buffer
    *_bufferPtr = (char*)malloc(fileSize + 1);
    if(*_bufferPtr == NULL){
        printf("Failed to allocate memory for file buffer.\n");
        fclose(file);
        return 0;
    }

    // Read file
    size_t bytesRead = fread(*_bufferPtr, 1, fileSize, file);
    (*_bufferPtr)[bytesRead] = '\0'; // Null terminate
    
    if(bytesRead != (size_t)fileSize){
        if(feof(file)){
            printf("End of file reached after reading %zu bytes.\n", bytesRead);
        } else if(ferror(file)){
            printf("Error reading file: %s\n", _filePath);
        }
    }

    fclose(file);
    return (uint32_t)bytesRead;
}



/*
===============
AF_DOCS_Print_Usage
Print usage information and examples to stdout
Params:
  program_name - Name of the executable (from argv[0])
===============
*/
void AF_DOCS_Print_Usage(const char *program_name)
{
    printf("Usage: %s [input_directory] [output_directory]\n\n", program_name);
    printf("Arguments:\n");
    printf("  input_directory  - Directory containing .md files (default: %s)\n", DEFAULT_INPUT_DIR);
    printf("  output_directory - Directory to save .html files (default: %s)\n\n", DEFAULT_OUTPUT_DIR);
    printf("Options:\n");
    printf("  -h, --help       - Show this help message\n\n");
    printf("Description:\n");
    printf("  Recursively converts all .md files to .html files.\n");
    printf("  Each .md file is converted to an .html file with the same base name.\n");
    printf("  Directory structure is preserved in the output.\n\n");
    printf("Examples:\n");
    printf("  %s                    # Use default directories\n", program_name);
    printf("  %s ./docs ./bin       # Convert docs/*.md to bin/*.html\n", program_name);
    printf("  %s ./markdown ./web   # Convert markdown/*.md to web/*.html\n", program_name);
}