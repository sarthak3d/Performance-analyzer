#include "analyzer.h"
#include "ast_visitor.h"
#include "complexity_calculator.h"
#include "pattern_detector.h"
#include <clang-c/Index.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ComplexityResult* create_complexity_result(void) {
    ComplexityResult* result = calloc(1, sizeof(ComplexityResult));
    if (result) {
        result->patterns = create_string_array();
        result->suggestions = create_string_array();
    }
    return result;
}

void free_complexity_result(ComplexityResult* result) {
    if (!result) return;
    
    free(result->time_complexity);
    free(result->space_complexity);
    free(result->best_case_time);
    free(result->average_case_time);
    free(result->worst_case_time);
    free(result->best_case_space);
    free(result->average_case_space);
    free(result->worst_case_space);
    free(result->explanation);
    
    free_string_array(result->patterns);
    free_string_array(result->suggestions);
    
    free(result);
}

ComplexityResult* analyze_code(const char* code) {
    if (!code || strlen(code) == 0) {
        return NULL;
    }
    
    // Initialize analysis data
    AnalysisData* data = create_analysis_data();
    if (!data) {
        return NULL;
    }
    
    // Create clang index and translation unit
    CXIndex index = clang_createIndex(0, 0);
    
    const char* filename = "input.c";
    struct CXUnsavedFile unsaved_file = {  // Fixed: Added 'struct' keyword
        .Filename = filename,
        .Contents = code,
        .Length = strlen(code)
    };
    
    // Compiler arguments for C11 standard
    const char* args[] = {
        "-std=c11",
        "-xc"
    };
    int num_args = sizeof(args) / sizeof(args[0]);
    
    // Parse the code
    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        filename,
        args, num_args,
        &unsaved_file, 1,
        CXTranslationUnit_DetailedPreprocessingRecord
    );
    
    if (!tu) {
        fprintf(stderr, "Error: Unable to parse translation unit\n");
        free_analysis_data(data);
        clang_disposeIndex(index);
        return NULL;
    }
    
    // Check for diagnostics
    unsigned num_diagnostics = clang_getNumDiagnostics(tu);
    for (unsigned i = 0; i < num_diagnostics; i++) {
        CXDiagnostic diagnostic = clang_getDiagnostic(tu, i);
        enum CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diagnostic);  // Fixed: Added 'enum' keyword
        
        if (severity >= CXDiagnostic_Error) {
            CXString error_string = clang_formatDiagnostic(diagnostic, 
                CXDiagnostic_DisplaySourceLocation | CXDiagnostic_DisplayColumn);
            fprintf(stderr, "Parse error: %s\n", clang_getCString(error_string));
            clang_disposeString(error_string);
        }
        clang_disposeDiagnostic(diagnostic);
    }
    
    // Visit the AST
    CXCursor root_cursor = clang_getTranslationUnitCursor(tu);
    visit_ast(root_cursor, data);
    
    // Detect patterns in the code
    StringArray* patterns = detect_patterns(code, data);
    
    // Calculate complexity
    ComplexityResult* result = calculate_complexity(data, patterns);
    
    // Transfer patterns to result (already done in calculate_complexity)
    
    // Cleanup
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    free_analysis_data(data);
    free_string_array(patterns);
    
    return result;
}