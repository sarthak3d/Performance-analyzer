#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "types.h"
#include <clang-c/Index.h>

// Create and destroy analysis data
AnalysisData* create_analysis_data(void);
void free_analysis_data(AnalysisData* data);

// Main AST visiting function
void visit_ast(CXCursor cursor, AnalysisData* data);

// String array utilities
StringArray* create_string_array(void);
void free_string_array(StringArray* array);
void string_array_add(StringArray* array, const char* str);
bool string_array_contains(StringArray* array, const char* str);

#endif // AST_VISITOR_H