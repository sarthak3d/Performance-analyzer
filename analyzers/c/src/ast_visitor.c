#include "ast_visitor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// String array implementation
StringArray* create_string_array(void) {
    StringArray* array = calloc(1, sizeof(StringArray));
    if (array) {
        array->capacity = 8;
        array->items = calloc(array->capacity, sizeof(char*));
    }
    return array;
}

void free_string_array(StringArray* array) {
    if (!array) return;
    
    for (size_t i = 0; i < array->count; i++) {
        free(array->items[i]);
    }
    free(array->items);
    free(array);
}

void string_array_add(StringArray* array, const char* str) {
    if (!array || !str) return;
    
    if (array->count >= array->capacity) {
        array->capacity *= 2;
        array->items = realloc(array->items, array->capacity * sizeof(char*));
    }
    
    array->items[array->count++] = strdup(str);
}

bool string_array_contains(StringArray* array, const char* str) {
    if (!array || !str) return false;
    
    for (size_t i = 0; i < array->count; i++) {
        if (strcmp(array->items[i], str) == 0) {
            return true;
        }
    }
    return false;
}

// Analysis data implementation
AnalysisData* create_analysis_data(void) {
    AnalysisData* data = calloc(1, sizeof(AnalysisData));
    if (data) {
        data->function_calls = create_string_array();
        data->current_functions = create_string_array();
        data->recursive_functions = create_string_array();
        data->has_constant_only = true;
        data->recursion_counts = calloc(100, sizeof(int));
    }
    return data;
}

void free_analysis_data(AnalysisData* data) {
    if (!data) return;
    
    free_string_array(data->function_calls);
    free_string_array(data->current_functions);
    free_string_array(data->recursive_functions);
    free(data->recursion_counts);
    free(data);
}

// Helper to track recursion depth and multiple calls
typedef struct {
    const char* function_name;
    int call_count_in_body;
    int max_depth;
} RecursionInfo;

static RecursionInfo* current_recursion_info = NULL;

// AST visitor callback
static enum CXChildVisitResult visitor_callback(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    AnalysisData* data = (AnalysisData*)client_data;
    enum CXCursorKind kind = clang_getCursorKind(cursor);
    
    // Get cursor name
    CXString spelling = clang_getCursorSpelling(cursor);
    const char* name = clang_getCString(spelling);
    
    switch (kind) {
        case CXCursor_FunctionDecl: {
            // Check if function has a body
            CXCursor definition = clang_getCursorDefinition(cursor);
            if (!clang_Cursor_isNull(definition)) {
                // Enter function
                string_array_add(data->current_functions, name);
                
                // Save and reset loop depth
                int saved_depth = data->current_loop_depth;
                data->current_loop_depth = 0;
                
                // Set up recursion tracking for this function
                RecursionInfo rec_info = {name, 0, 0};
                RecursionInfo* saved_rec_info = current_recursion_info;
                current_recursion_info = &rec_info;
                
                // Visit function body
                clang_visitChildren(definition, visitor_callback, data);
                
                // Check if this function had multiple recursive calls
                if (rec_info.call_count_in_body > 0) {
                    data->has_recursion = true;
                    
                    // Find or add to recursive functions
                    size_t func_index = data->recursive_functions->count;
                    for (size_t i = 0; i < data->recursive_functions->count; i++) {
                        if (strcmp(data->recursive_functions->items[i], name) == 0) {
                            func_index = i;
                            break;
                        }
                    }
                    
                    if (func_index == data->recursive_functions->count) {
                        string_array_add(data->recursive_functions, name);
                    }
                    
                    // Store recursion count (multiple calls = exponential)
                    if (func_index < 100) {
                        data->recursion_counts[func_index] = rec_info.call_count_in_body;
                    }
                }
                
                // Restore state
                current_recursion_info = saved_rec_info;
                data->current_loop_depth = saved_depth;
                if (data->current_functions->count > 0) {
                    data->current_functions->count--;
                }
            }
            break;
        }
        
        case CXCursor_ForStmt: {
            data->has_loops = true;
            data->has_constant_only = false;
            data->current_loop_depth++;
            
            if (data->current_loop_depth > data->max_loop_depth) {
                data->max_loop_depth = data->current_loop_depth;
            }
            
            if (data->current_loop_depth >= 2) {
                data->has_nested_loops = true;
            }
            
            // Check for logarithmic patterns
            CXSourceRange range = clang_getCursorExtent(cursor);
            CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
            
            CXToken* tokens = NULL;
            unsigned num_tokens = 0;
            clang_tokenize(tu, range, &tokens, &num_tokens);
            
            // Look for logarithmic patterns
            for (unsigned i = 0; i < num_tokens - 1; i++) {
                CXString token_str = clang_getTokenSpelling(tu, tokens[i]);
                const char* token = clang_getCString(token_str);
                
                // Check for *= 2, /= 2, <<= 1, >>= 1
                if (strcmp(token, "*=") == 0 || strcmp(token, "/=") == 0 ||
                    strcmp(token, "<<=") == 0 || strcmp(token, ">>=") == 0) {
                    if (i + 1 < num_tokens) {
                        CXString next_str = clang_getTokenSpelling(tu, tokens[i + 1]);
                        const char* next = clang_getCString(next_str);
                        char* endptr;
                        long val = strtol(next, &endptr, 10);
                        
                        // Check if it's multiplying/dividing by a power of 2
                        if (*endptr == '\0' && val > 1 && (val & (val - 1)) == 0) {
                            data->has_logarithmic_loop = true;
                        }
                        clang_disposeString(next_str);
                    }
                }
                
                // Also check for i = i * 2, i = i / 2 patterns
                if (strcmp(token, "*") == 0 || strcmp(token, "/") == 0) {
                    if (i + 1 < num_tokens) {
                        CXString next_str = clang_getTokenSpelling(tu, tokens[i + 1]);
                        const char* next = clang_getCString(next_str);
                        if (strcmp(next, "2") == 0) {
                            // Check if it's in an assignment context
                            if (i > 0) {
                                CXString prev_str = clang_getTokenSpelling(tu, tokens[i - 1]);
                                const char* prev = clang_getCString(prev_str);
                                // Simple heuristic - if variable name before operator
                                if (isalpha(prev[0]) || prev[0] == '_') {
                                    data->has_logarithmic_loop = true;
                                }
                                clang_disposeString(prev_str);
                            }
                        }
                        clang_disposeString(next_str);
                    }
                }
                
                clang_disposeString(token_str);
            }
            
            if (tokens) {
                clang_disposeTokens(tu, tokens, num_tokens);
            }
            
            // Visit loop body
            clang_visitChildren(cursor, visitor_callback, data);
            data->current_loop_depth--;
            
            clang_disposeString(spelling);
            return CXChildVisit_Continue;
        }
        
        case CXCursor_WhileStmt:
        case CXCursor_DoStmt:
            data->has_loops = true;
            data->has_constant_only = false;
            data->current_loop_depth++;
            
            if (data->current_loop_depth > data->max_loop_depth) {
                data->max_loop_depth = data->current_loop_depth;
            }
            
            if (data->current_loop_depth >= 2) {
                data->has_nested_loops = true;
            }
            
            // Check for logarithmic patterns in while loops
            if (kind == CXCursor_WhileStmt) {
                CXSourceRange range = clang_getCursorExtent(cursor);
                CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
                
                CXToken* tokens = NULL;
                unsigned num_tokens = 0;
                clang_tokenize(tu, range, &tokens, &num_tokens);
                
                // Look for n /= 2 or similar patterns
                for (unsigned i = 0; i < num_tokens - 1; i++) {
                    CXString token_str = clang_getTokenSpelling(tu, tokens[i]);
                    const char* token = clang_getCString(token_str);
                    
                    if (strcmp(token, "/=") == 0 || strcmp(token, ">>=") == 0) {
                        if (i + 1 < num_tokens) {
                            CXString next_str = clang_getTokenSpelling(tu, tokens[i + 1]);
                            const char* next = clang_getCString(next_str);
                            if (strcmp(next, "2") == 0) {
                                data->has_logarithmic_loop = true;
                            }
                            clang_disposeString(next_str);
                        }
                    }
                    
                    clang_disposeString(token_str);
                }
                
                if (tokens) {
                    clang_disposeTokens(tu, tokens, num_tokens);
                }
            }
            
            // Visit loop body
            clang_visitChildren(cursor, visitor_callback, data);
            data->current_loop_depth--;
            
            clang_disposeString(spelling);
            return CXChildVisit_Continue;
            
        case CXCursor_CallExpr: {
            // Get the actual function name being called
            CXCursor referenced = clang_getCursorReferenced(cursor);
            CXString ref_spelling = clang_getCursorSpelling(referenced);
            const char* func_name = clang_getCString(ref_spelling);
            
            // Add to function calls
            string_array_add(data->function_calls, func_name);
            
            // Check for recursion
            if (data->current_functions->count > 0) {
                const char* current_func = data->current_functions->items[data->current_functions->count - 1];
                if (strcmp(func_name, current_func) == 0) {
                    // This is a recursive call
                    if (current_recursion_info && 
                        strcmp(current_recursion_info->function_name, current_func) == 0) {
                        current_recursion_info->call_count_in_body++;
                    }
                }
            }
            
            // Check for memory allocation functions
            if (strcmp(func_name, "malloc") == 0) {
                data->has_dynamic_alloc = true;
                data->malloc_count++;
                
                // Check if malloc is in a loop
                if (data->current_loop_depth > 0) {
                    // malloc in loop - space complexity increases
                    data->has_dynamic_alloc = true;
                }
            } else if (strcmp(func_name, "calloc") == 0) {
                data->has_dynamic_alloc = true;
                data->malloc_count++;
                
                // calloc allocates and zeros memory
                if (data->current_loop_depth > 0) {
                    data->has_dynamic_alloc = true;
                }
            } else if (strcmp(func_name, "realloc") == 0) {
                data->has_dynamic_alloc = true;
                data->malloc_count++;
                
                // realloc can grow arrays dynamically
                if (data->current_loop_depth > 0) {
                    data->has_dynamic_alloc = true;
                }
            } else if (strcmp(func_name, "free") == 0) {
                // Track free calls (good practice)
                // Note: Could check for memory leaks by matching malloc/free
            }
            
            // Check for standard library functions
            else if (strcmp(func_name, "qsort") == 0) {
                data->has_sorting = true;
                string_array_add(data->function_calls, "stdlib_sort");
            } else if (strcmp(func_name, "bsearch") == 0) {
                data->has_searching = true;
                string_array_add(data->function_calls, "binary_search");
            }
            
            // Check for string functions that imply O(n) operations
            else if (strcmp(func_name, "strlen") == 0 ||
                     strcmp(func_name, "strcpy") == 0 ||
                     strcmp(func_name, "strncpy") == 0 ||
                     strcmp(func_name, "strcat") == 0 ||
                     strcmp(func_name, "strcmp") == 0 ||
                     strcmp(func_name, "strncmp") == 0) {
                // These are O(n) operations on strings
                if (data->current_loop_depth > 0) {
                    // String operation in loop - complexity increases
                }
            }
            
            // Check for memory operations
            else if (strcmp(func_name, "memcpy") == 0 ||
                     strcmp(func_name, "memmove") == 0 ||
                     strcmp(func_name, "memset") == 0) {
                // These are O(n) operations on memory
                if (data->current_loop_depth > 0) {
                    // Memory operation in loop
                }
            }
            
            clang_disposeString(ref_spelling);
            break;
        }
        
        case CXCursor_ArraySubscriptExpr:
            data->has_arrays = true;
            data->array_count++;
            break;
            
        case CXCursor_VarDecl: {
            CXType type = clang_getCursorType(cursor);
            
            // Check for arrays
            if (type.kind == CXType_ConstantArray || 
                type.kind == CXType_VariableArray ||
                type.kind == CXType_IncompleteArray) {
                data->has_arrays = true;
                data->array_count++;
                
                // Get array size if constant
                if (type.kind == CXType_ConstantArray) {
                    long long size = clang_getArraySize(type);
                    // Could track array sizes for space complexity
                }
            }
            
            // Check for pointers
            if (type.kind == CXType_Pointer) {
                data->pointer_count++;
            }
            
            // Check for recursive data structures
            CXString type_spelling = clang_getTypeSpelling(type);
            const char* type_name = clang_getCString(type_spelling);
            
            // Check for common recursive structures
            if (strstr(type_name, "node") != NULL ||
                strstr(type_name, "Node") != NULL ||
                strstr(type_name, "tree") != NULL ||
                strstr(type_name, "Tree") != NULL ||
                strstr(type_name, "list") != NULL ||
                strstr(type_name, "List") != NULL) {
                // Likely a recursive data structure
                data->has_arrays = true;  // Treat as dynamic structure
            }
            
            clang_disposeString(type_spelling);
            break;
        }
        
        case CXCursor_BinaryOperator: {
            // Enhanced logarithmic pattern detection
            if (data->current_loop_depth > 0) {
                CXSourceRange range = clang_getCursorExtent(cursor);
                CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
                
                CXToken* tokens = NULL;
                unsigned num_tokens = 0;
                clang_tokenize(tu, range, &tokens, &num_tokens);
                
                for (unsigned i = 0; i < num_tokens; i++) {
                    CXString token_str = clang_getTokenSpelling(tu, tokens[i]);
                    const char* token = clang_getCString(token_str);
                    
                    // Check for logarithmic operators
                    if (strcmp(token, "*=") == 0 || strcmp(token, "/=") == 0 ||
                        strcmp(token, "<<=") == 0 || strcmp(token, ">>=") == 0) {
                        if (i + 1 < num_tokens) {
                            CXString next_str = clang_getTokenSpelling(tu, tokens[i + 1]);
                            const char* next = clang_getCString(next_str);
                            char* endptr;
                            long val = strtol(next, &endptr, 10);
                            if (*endptr == '\0' && val > 1) {
                                // Power of 2 or any multiplication/division
                                data->has_logarithmic_loop = true;
                            }
                            clang_disposeString(next_str);
                        }
                    }
                    clang_disposeString(token_str);
                }
                
                if (tokens) {
                    clang_disposeTokens(tu, tokens, num_tokens);
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    clang_disposeString(spelling);
    
    // Continue visiting children
    clang_visitChildren(cursor, visitor_callback, data);
    
    return CXChildVisit_Continue;
}

void visit_ast(CXCursor cursor, AnalysisData* data) {
    clang_visitChildren(cursor, visitor_callback, data);
}