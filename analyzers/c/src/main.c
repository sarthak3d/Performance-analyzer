#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyzer.h"
#include "json_handler.h"

#define MAX_INPUT_SIZE (10 * 1024 * 1024)  // 10MB max input

static char* read_stdin(void) {
    char* buffer = malloc(MAX_INPUT_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for input\n");
        return NULL;
    }
    
    size_t total_read = 0;
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer + total_read, 1, 
                               MAX_INPUT_SIZE - total_read - 1, stdin)) > 0) {
        total_read += bytes_read;
        if (total_read >= MAX_INPUT_SIZE - 1) {
            fprintf(stderr, "Warning: Input truncated at %zu bytes\n", total_read);
            break;
        }
    }
    
    buffer[total_read] = '\0';
    return buffer;
}

int main(int argc, char* argv[]) {
    // Read JSON input from stdin
    char* input = read_stdin();
    if (!input) {
        return 1;
    }
    
    // Parse JSON to extract code
    char* code = parse_json_request(input);
    free(input);
    
    if (!code) {
        fprintf(stderr, "Error: Failed to parse JSON input\n");
        return 1;
    }
    
    // Analyze the code
    ComplexityResult* result = analyze_code(code);
    free(code);
    
    if (!result) {
        fprintf(stderr, "Error: Failed to analyze code\n");
        return 1;
    }
    
    // Convert result to JSON
    char* json_output = result_to_json(result);
    free_complexity_result(result);
    
    if (!json_output) {
        fprintf(stderr, "Error: Failed to generate JSON output\n");
        return 1;
    }
    
    // Output the JSON result
    printf("%s\n", json_output);
    free(json_output);
    
    return 0;
}