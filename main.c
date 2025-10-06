#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "md4c.h"
#include "md4c-html.h"

uint32_t Read_MD(const char * _filePath, char** _bufferPtr);
void process_html_output(const MD_CHAR* text, MD_SIZE size, void* userdata);

const char* FILE_PATH = "./docs/test.md";
const char* OUTPUT_PATH = "./bin/index.html";

// Structure to hold HTML output
typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} html_buffer_t;

int main(){
    printf("Hello, AF_Docs!\n");

    // Read markdown file
    char* mdContent = NULL;
    uint32_t bytesRead = Read_MD(FILE_PATH, &mdContent);
    
    if(bytesRead == 0 || mdContent == NULL){
        printf("Failed to read from %s or file is empty.\n", FILE_PATH);
        return 1;
    }
    
    printf("Read %u bytes from %s\n", bytesRead, FILE_PATH);
    printf("Content:\n%s\n", mdContent);

    // Initialize HTML output buffer
    html_buffer_t html_output = {0};
    html_output.capacity = bytesRead * 2; // Allocate more space for HTML tags
    html_output.data = (char*)malloc(html_output.capacity);
    if(html_output.data == NULL){
        printf("Failed to allocate memory for HTML output.\n");
        free(mdContent);
        return 1;
    }
    html_output.size = 0;

    // Convert markdown to HTML
    printf("Converting markdown to HTML...\n");
    int result = md_html(mdContent, bytesRead, process_html_output, &html_output, 
                        MD_DIALECT_GITHUB, 0);
    
    if(result != 0){
        printf("MD to HTML conversion failed with error code: %d\n", result);
        free(mdContent);
        free(html_output.data);
        return 1;
    }

    // combine the HTML header and footer
    const char* html_header = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Converted Markdown</title>\n<link rel=\"stylesheet\" href=\"./markdown.css\" type=\"text/css\">\n</head>\n<body>\n";
    const char* html_footer = "\n</body>\n</html>";
    size_t header_len = strlen(html_header);
    size_t footer_len = strlen(html_footer);
    size_t total_size = header_len + html_output.size + footer_len;
    char* full_html = (char*)malloc(total_size + 1);
    if(full_html == NULL){
        printf("Failed to allocate memory for full HTML output.\n");
        free(mdContent);
        free(html_output.data);
        return 1;
    }
    // copy in the header, md and footer into the full_html buffer
    full_html[total_size] = '\0'; // null terminate
    memcpy(full_html, html_header, header_len);
    memcpy(full_html + header_len, html_output.data, html_output.size);
    memcpy(full_html + header_len + html_output.size, html_footer, footer_len);
    free(html_output.data);

    printf("MD to HTML conversion succeeded.\n");
    printf("HTML output (%zu bytes):\n%s\n", total_size, full_html);

    // Write HTML to file
    FILE* outputFile = fopen(OUTPUT_PATH, "w");
    if(outputFile != NULL){
        fwrite(full_html, 1, total_size, outputFile);
        fclose(outputFile);
        printf("Output written to %s\n", OUTPUT_PATH);
    } else {
        printf("Failed to open %s for writing.\n", OUTPUT_PATH);
    }

    // Cleanup
    free(mdContent);
    free(html_output.data);

    return 0;
}

// Callback function to collect HTML output
void process_html_output(const MD_CHAR* text, MD_SIZE size, void* userdata){
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

uint32_t Read_MD(const char * _filePath, char** _bufferPtr){
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

