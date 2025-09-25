#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct AnalysisData AnalysisData;
typedef struct ComplexityResult ComplexityResult;
typedef struct StringArray StringArray;

// Complexity classes
typedef enum {
    COMPLEXITY_O_1,        // O(1) - Constant
    COMPLEXITY_O_LOG_N,    // O(log n) - Logarithmic
    COMPLEXITY_O_SQRT_N,   // O(√n) - Square root
    COMPLEXITY_O_N,        // O(n) - Linear
    COMPLEXITY_O_N_LOG_N,  // O(n log n) - Linearithmic
    COMPLEXITY_O_N2,       // O(n²) - Quadratic
    COMPLEXITY_O_N3,       // O(n³) - Cubic
    COMPLEXITY_O_2_N,      // O(2^n) - Exponential
    COMPLEXITY_O_N_FACT,   // O(n!) - Factorial
    COMPLEXITY_UNKNOWN     // Unknown complexity
} ComplexityClass;

// Dynamic string array
struct StringArray {
    char** items;
    size_t count;
    size_t capacity;
};

// Analysis data collected from AST
struct AnalysisData {
    // Loop information
    int max_loop_depth;
    int current_loop_depth;
    bool has_loops;
    bool has_logarithmic_loop;
    bool has_nested_loops;
    
    // Recursion information
    bool has_recursion;
    StringArray* recursive_functions;
    int* recursion_counts;  // Count of recursive calls per function
    
    // Memory information
    bool has_dynamic_alloc;
    bool has_arrays;
    int malloc_count;
    int array_count;
    int pointer_count;
    
    // Function tracking
    StringArray* function_calls;
    StringArray* current_functions;  // Stack of current functions being analyzed
    
    // Complexity hints
    bool has_constant_only;
    bool has_sorting;
    bool has_searching;
};

// Complexity analysis result
struct ComplexityResult {
    // Complexity strings
    char* time_complexity;
    char* space_complexity;
    
    // Detailed complexity cases
    char* best_case_time;
    char* average_case_time;
    char* worst_case_time;
    char* best_case_space;
    char* average_case_space;
    char* worst_case_space;
    
    // Analysis metadata
    double confidence;
    char* explanation;
    
    // Detected patterns and suggestions
    StringArray* patterns;
    StringArray* suggestions;
    
    // Code characteristics
    bool recursive;
    bool has_loops;
    int loop_depth;
};

#endif // TYPES_H