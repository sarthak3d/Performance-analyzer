#ifndef PATTERN_DETECTOR_H
#define PATTERN_DETECTOR_H

#include "types.h"

// Detect patterns in code
StringArray* detect_patterns(const char* code, AnalysisData* data);

// Individual pattern detection functions
bool detect_sorting_pattern(const char* code, AnalysisData* data);
bool detect_searching_pattern(const char* code, AnalysisData* data);
bool detect_dynamic_programming(const char* code);
bool detect_graph_algorithm(const char* code);

#endif // PATTERN_DETECTOR_H