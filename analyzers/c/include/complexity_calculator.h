#ifndef COMPLEXITY_CALCULATOR_H
#define COMPLEXITY_CALCULATOR_H

#include "types.h"

// Calculate complexity from analysis data
ComplexityResult* calculate_complexity(AnalysisData* data, StringArray* patterns);

// Convert complexity class to string representation
const char* complexity_to_string(ComplexityClass c);

// Calculate specific complexities
ComplexityClass calculate_time_complexity(AnalysisData* data, StringArray* patterns);
ComplexityClass calculate_space_complexity(AnalysisData* data, StringArray* patterns);

#endif // COMPLEXITY_CALCULATOR_H