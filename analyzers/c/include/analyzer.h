#ifndef ANALYZER_H
#define ANALYZER_H

#include "types.h"

// Main analysis function
ComplexityResult* analyze_code(const char* code);

// Memory management
ComplexityResult* create_complexity_result(void);
void free_complexity_result(ComplexityResult* result);

#endif // ANALYZER_H