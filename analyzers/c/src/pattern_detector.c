#include "pattern_detector.h"
#include "ast_visitor.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static bool contains_pattern(const char* code, const char* pattern) {
    return strstr(code, pattern) != NULL;
}

static bool contains_pattern_case_insensitive(const char* code, const char* pattern) {
    size_t code_len = strlen(code);
    size_t pattern_len = strlen(pattern);
    
    if (pattern_len > code_len) return false;
    
    for (size_t i = 0; i <= code_len - pattern_len; i++) {
        bool match = true;
        for (size_t j = 0; j < pattern_len; j++) {
            if (tolower(code[i + j]) != tolower(pattern[j])) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    
    return false;
}

bool detect_sorting_pattern(const char* code, AnalysisData* data) {
    // Bubble sort
    if ((contains_pattern_case_insensitive(code, "bubble") && 
         contains_pattern_case_insensitive(code, "sort")) ||
        (contains_pattern(code, "swap") && data->has_nested_loops)) {
        return true;
    }
    
    // Quick sort
    if ((contains_pattern_case_insensitive(code, "quick") && 
         contains_pattern_case_insensitive(code, "sort")) ||
        (contains_pattern(code, "pivot") && contains_pattern(code, "partition"))) {
        return true;
    }
    
    // Merge sort
    if ((contains_pattern_case_insensitive(code, "merge") && 
         contains_pattern_case_insensitive(code, "sort")) ||
        (contains_pattern(code, "merge") && contains_pattern(code, "divide"))) {
        return true;
    }
    
    // Heap sort
    if (contains_pattern_case_insensitive(code, "heap") && 
        contains_pattern_case_insensitive(code, "sort")) {
        return true;
    }
    
    // Check for qsort
    if (data->has_sorting) {
        return true;
    }
    
    return false;
}

bool detect_searching_pattern(const char* code, AnalysisData* data) {
    // Binary search
    bool has_mid = contains_pattern(code, "mid") || 
                   contains_pattern(code, "middle") ||
                   contains_pattern(code, "(low + high) / 2") ||
                   contains_pattern(code, "(left + right) / 2");
    
    bool has_bounds = (contains_pattern(code, "left") && contains_pattern(code, "right")) ||
                      (contains_pattern(code, "low") && contains_pattern(code, "high"));
    
    if ((contains_pattern_case_insensitive(code, "binary") && 
         contains_pattern_case_insensitive(code, "search")) ||
        (has_mid && has_bounds) ||
        (data->has_logarithmic_loop && has_bounds)) {
        return true;
    }
    
    // Check for bsearch
    if (data->has_searching) {
        return true;
    }
    
    // Linear search
    if (data->has_loops && contains_pattern(code, "==") && 
        contains_pattern(code, "return")) {
        return true;
    }
    
    return false;
}

bool detect_dynamic_programming(const char* code) {
    return contains_pattern(code, "dp[") ||
           contains_pattern(code, "memo[") ||
           contains_pattern(code, "cache[") ||
           contains_pattern(code, "table[") ||
           contains_pattern_case_insensitive(code, "memoization") ||
           contains_pattern_case_insensitive(code, "tabulation");
}

bool detect_graph_algorithm(const char* code) {
    return contains_pattern_case_insensitive(code, "dfs") ||
           contains_pattern_case_insensitive(code, "bfs") ||
           contains_pattern_case_insensitive(code, "dijkstra") ||
           contains_pattern_case_insensitive(code, "graph") ||
           contains_pattern_case_insensitive(code, "vertex") ||
           contains_pattern_case_insensitive(code, "edge") ||
           contains_pattern_case_insensitive(code, "adjacency");
}

StringArray* detect_patterns(const char* code, AnalysisData* data) {
    StringArray* patterns = create_string_array();
    if (!patterns) return NULL;
    
    // Detect algorithmic patterns
    if (detect_sorting_pattern(code, data)) {
        // Try to identify specific sorting algorithm
        if (contains_pattern_case_insensitive(code, "bubble")) {
            string_array_add(patterns, "bubblesort");
        } else if (contains_pattern_case_insensitive(code, "quick") ||
                   contains_pattern(code, "pivot")) {
            string_array_add(patterns, "quicksort");
        } else if (contains_pattern_case_insensitive(code, "merge")) {
            string_array_add(patterns, "mergesort");
        } else if (contains_pattern_case_insensitive(code, "heap")) {
            string_array_add(patterns, "heapsort");
        } else {
            string_array_add(patterns, "sorting");
        }
    }
    
    if (detect_searching_pattern(code, data)) {
        if (data->has_logarithmic_loop || contains_pattern(code, "mid") ||
            contains_pattern_case_insensitive(code, "binary")) {
            string_array_add(patterns, "binary_search");
        } else {
            string_array_add(patterns, "linear_search");
        }
    }
    
    if (detect_dynamic_programming(code)) {
        string_array_add(patterns, "dynamic_programming");
    }
    
    if (detect_graph_algorithm(code)) {
        string_array_add(patterns, "graph_algorithm");
    }
    
    // Add structural patterns from AST analysis
    if (data->has_recursion) {
        string_array_add(patterns, "recursion");
    }
    
    if (data->has_logarithmic_loop) {
        string_array_add(patterns, "logarithmic");
    }
    
    if (data->has_nested_loops) {
        string_array_add(patterns, "nested_loops");
    } else if (data->has_loops) {
        string_array_add(patterns, "single_loop");
    }
    
    if (data->has_dynamic_alloc) {
        string_array_add(patterns, "dynamic_allocation");
    }
    
    if (data->has_arrays) {
        string_array_add(patterns, "arrays");
    }
    
    // Add qsort if detected
    if (data->function_calls && string_array_contains(data->function_calls, "stdlib_sort")) {
        string_array_add(patterns, "stdlib_sort");
    }
    
    return patterns;
}