#include "complexity_calculator.h"
#include "analyzer.h"
#include "ast_visitor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

const char* complexity_to_string(ComplexityClass c) {
    switch (c) {
        case COMPLEXITY_O_1: return "O(1)";
        case COMPLEXITY_O_LOG_N: return "O(log n)";
        case COMPLEXITY_O_SQRT_N: return "O(√n)";
        case COMPLEXITY_O_N: return "O(n)";
        case COMPLEXITY_O_N_LOG_N: return "O(n log n)";
        case COMPLEXITY_O_N2: return "O(n²)";
        case COMPLEXITY_O_N3: return "O(n³)";
        case COMPLEXITY_O_2_N: return "O(2^n)";
        case COMPLEXITY_O_N_FACT: return "O(n!)";
        default: return "O(?)";
    }
}

ComplexityClass calculate_time_complexity(AnalysisData* data, StringArray* patterns) {
    // Check for specific algorithm patterns first
    if (patterns) {
        for (size_t i = 0; i < patterns->count; i++) {
            if (strcmp(patterns->items[i], "binary_search") == 0) {
                return COMPLEXITY_O_LOG_N;
            }
            if (strcmp(patterns->items[i], "mergesort") == 0 ||
                strcmp(patterns->items[i], "heapsort") == 0 ||
                strcmp(patterns->items[i], "quicksort") == 0 ||
                strcmp(patterns->items[i], "stdlib_sort") == 0) {
                return COMPLEXITY_O_N_LOG_N;
            }
        }
    }
    
    // Check for merge sort pattern (recursive with merge)
    bool has_merge = false;
    bool has_divide = false;
    if (data->function_calls) {
        for (size_t i = 0; i < data->function_calls->count; i++) {
            if (strstr(data->function_calls->items[i], "merge") != NULL) {
                has_merge = true;
            }
            if (strstr(data->function_calls->items[i], "sort") != NULL) {
                has_divide = true;
            }
        }
    }
    
    // Merge sort detection: recursion + merge function
    if (data->has_recursion && has_merge) {
        return COMPLEXITY_O_N_LOG_N;
    }
    
    // Check function calls
    if (data->function_calls) {
        if (string_array_contains(data->function_calls, "binary_search") ||
            string_array_contains(data->function_calls, "bsearch")) {
            return COMPLEXITY_O_LOG_N;
        }
        if (string_array_contains(data->function_calls, "stdlib_sort") ||
            string_array_contains(data->function_calls, "qsort")) {
            return COMPLEXITY_O_N_LOG_N;
        }
    }
    
    // Check for logarithmic loops
    if (data->has_logarithmic_loop && data->max_loop_depth == 1) {
        return COMPLEXITY_O_LOG_N;
    }
    
    if (data->has_logarithmic_loop && data->max_loop_depth == 2) {
        return COMPLEXITY_O_N_LOG_N;
    }
    
    // Handle recursion - IMPROVED DETECTION
    if (data->has_recursion) {
        // Check for exponential recursion (multiple recursive calls per function)
        bool has_exponential = false;
        
        // Check recursion counts
        for (size_t i = 0; i < data->recursive_functions->count && i < 100; i++) {
            if (data->recursion_counts[i] >= 2) {
                // Multiple recursive calls in same function = likely exponential
                has_exponential = true;
                break;
            }
        }
        
        // Also check for fibonacci pattern
        if (data->recursive_functions) {
            for (size_t i = 0; i < data->recursive_functions->count; i++) {
                const char* func_name = data->recursive_functions->items[i];
                if (strstr(func_name, "fib") != NULL || strstr(func_name, "fibonacci") != NULL) {
                    has_exponential = true;
                    break;
                }
            }
        }
        
        if (has_exponential) {
            return COMPLEXITY_O_2_N;
        }
        
        // Single recursion with loops
        if (data->has_loops) {
            if (data->max_loop_depth >= 2) {
                return COMPLEXITY_O_N2;
            }
            return COMPLEXITY_O_N;
        }
        
        // Simple recursion is typically O(n)
        return COMPLEXITY_O_N;
    }
    
    // No loops or recursion - constant time
    if (!data->has_loops && !data->has_recursion && data->has_constant_only) {
        return COMPLEXITY_O_1;
    }
    
    // Based on loop nesting depth
    switch (data->max_loop_depth) {
        case 0: return COMPLEXITY_O_1;
        case 1: return COMPLEXITY_O_N;
        case 2: return COMPLEXITY_O_N2;
        case 3: return COMPLEXITY_O_N3;
        default: return COMPLEXITY_O_2_N;  // Very deep nesting
    }
}

ComplexityClass calculate_space_complexity(AnalysisData* data, StringArray* patterns) {
    // Check for binary search pattern - should be O(1) space
    if (patterns) {
        for (size_t i = 0; i < patterns->count; i++) {
            if (strcmp(patterns->items[i], "binary_search") == 0) {
                // Iterative binary search is O(1) space
                if (!data->has_recursion) {
                    return COMPLEXITY_O_1;
                }
                // Recursive binary search is O(log n) space
                return COMPLEXITY_O_LOG_N;
            }
        }
    }
    
    // Recursion uses stack space
    if (data->has_recursion) {
        // Check for exponential recursion
        bool has_exponential = false;
        for (size_t i = 0; i < data->recursive_functions->count && i < 100; i++) {
            if (data->recursion_counts[i] >= 2) {
                has_exponential = true;
                break;
            }
        }
        
        if (has_exponential) {
            // Exponential recursion can use exponential space (call stack)
            return COMPLEXITY_O_N;  // Usually O(n) for fibonacci due to call stack depth
        }
        
        // Tail recursion or simple recursion is O(n) space
        return COMPLEXITY_O_N;
    }
    
    // Dynamic allocation in loops
    if (data->has_dynamic_alloc && data->has_loops) {
        if (data->max_loop_depth >= 2) {
            return COMPLEXITY_O_N2;
        }
        return COMPLEXITY_O_N;
    }
    
    // Arrays
    if (data->has_arrays) {
        // Arrays typically mean O(n) space
        return COMPLEXITY_O_N;
    }
    
    // Default: constant space
    return COMPLEXITY_O_1;
}

static char* generate_explanation(AnalysisData* data, ComplexityClass time, ComplexityClass space) {
    char* explanation = malloc(1024);
    explanation[0] = '\0';
    
    if (!data->has_loops && !data->has_recursion) {
        strcat(explanation, "Code contains only constant-time operations. ");
    }
    
    if (data->has_logarithmic_loop) {
        strcat(explanation, "Detected logarithmic loop pattern (variable doubling/halving). ");
    }
    
    if (data->max_loop_depth > 0) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Found %d level(s) of nested loops. ", data->max_loop_depth);
        strcat(explanation, buffer);
    }
    
    if (data->has_recursion) {
        strcat(explanation, "Detected recursive function(s). ");
        
        // Check for exponential pattern
        for (size_t i = 0; i < data->recursive_functions->count && i < 100; i++) {
            if (data->recursion_counts[i] >= 2) {
                strcat(explanation, "Multiple recursive calls detected (exponential complexity). ");
                break;
            }
        }
    }
    
    if (data->has_dynamic_alloc) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Uses dynamic memory allocation (%d call(s)). ", data->malloc_count);
        strcat(explanation, buffer);
    }
    
    if (data->has_arrays) {
        strcat(explanation, "Uses arrays. ");
    }
    
    if (strlen(explanation) == 0) {
        strcat(explanation, "Simple sequential code structure detected.");
    }
    
    return explanation;
}

static void generate_suggestions(ComplexityResult* result, AnalysisData* data, ComplexityClass time) {
    if (time == COMPLEXITY_O_N2 || time == COMPLEXITY_O_N3) {
        string_array_add(result->suggestions, "Consider optimizing nested loops");
        string_array_add(result->suggestions, "Look for opportunities to use more efficient algorithms");
        if (data->has_nested_loops) {
            string_array_add(result->suggestions, "Try to reduce loop nesting levels");
        }
    }
    
    if (data->has_recursion && time == COMPLEXITY_O_2_N) {
        string_array_add(result->suggestions, "Consider using memoization or dynamic programming");
        string_array_add(result->suggestions, "Convert recursion to iteration if possible");
    }
    
    if (data->max_loop_depth > 2) {
        string_array_add(result->suggestions, "Deep nesting detected - consider refactoring for better readability");
    }
    
    if (data->malloc_count > 10) {
        string_array_add(result->suggestions, "Multiple dynamic allocations detected - consider using memory pools");
    }
    
    if (data->has_dynamic_alloc && !data->has_loops) {
        string_array_add(result->suggestions, "Remember to free dynamically allocated memory");
    }
}

ComplexityResult* calculate_complexity(AnalysisData* data, StringArray* patterns) {
    ComplexityResult* result = create_complexity_result();
    if (!result) return NULL;
    
    // Calculate time and space complexity
    ComplexityClass time_complexity = calculate_time_complexity(data, patterns);
    ComplexityClass space_complexity = calculate_space_complexity(data, patterns);
    
    // Set complexity strings
    result->time_complexity = strdup(complexity_to_string(time_complexity));
    result->space_complexity = strdup(complexity_to_string(space_complexity));
    
    // Check for specific algorithm patterns for different cases
    bool is_quicksort = patterns && string_array_contains(patterns, "quicksort");
    bool is_bubblesort = patterns && string_array_contains(patterns, "bubblesort");
    bool is_binary_search = patterns && string_array_contains(patterns, "binary_search");
    bool is_mergesort = patterns && string_array_contains(patterns, "mergesort");
    
    if (is_quicksort) {
        result->best_case_time = strdup("O(n log n)");
        result->average_case_time = strdup("O(n log n)");
        result->worst_case_time = strdup("O(n²)");
    } else if (is_bubblesort) {
        result->best_case_time = strdup("O(n)");
        result->average_case_time = strdup("O(n²)");
        result->worst_case_time = strdup("O(n²)");
    } else if (is_binary_search) {
        result->best_case_time = strdup("O(1)");
        result->average_case_time = strdup("O(log n)");
        result->worst_case_time = strdup("O(log n)");
    } else if (is_mergesort) {
        result->best_case_time = strdup("O(n log n)");
        result->average_case_time = strdup("O(n log n)");
        result->worst_case_time = strdup("O(n log n)");
    } else {
        // Default: all cases are the same
        result->best_case_time = strdup(result->time_complexity);
        result->average_case_time = strdup(result->time_complexity);
        result->worst_case_time = strdup(result->time_complexity);
    }
    
    // Space complexity cases (usually the same)
    result->best_case_space = strdup(result->space_complexity);
    result->average_case_space = strdup(result->space_complexity);
    result->worst_case_space = strdup(result->space_complexity);
    
    // Calculate confidence
    double confidence = 0.75;  // Base confidence
    if (data->has_loops) confidence += 0.05;
    if (data->has_recursion) confidence += 0.05;
    if (patterns && patterns->count > 0) confidence += 0.1;
    if (data->has_logarithmic_loop) confidence += 0.05;
    if (confidence > 0.95) confidence = 0.95;
    result->confidence = confidence;
    
    // Generate explanation
    result->explanation = generate_explanation(data, time_complexity, space_complexity);
    
    // Copy patterns
    if (patterns) {
        for (size_t i = 0; i < patterns->count; i++) {
            string_array_add(result->patterns, patterns->items[i]);
        }
    }
    
    // Generate suggestions
    generate_suggestions(result, data, time_complexity);
    
    // Set metadata
    result->recursive = data->has_recursion;
    result->has_loops = data->has_loops;
    result->loop_depth = data->max_loop_depth;
    
    return result;
}