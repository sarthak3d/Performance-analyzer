#ifndef PATTERN_DETECTOR_HPP
#define PATTERN_DETECTOR_HPP

#include <vector>
#include <string>
#include "ast_visitor.hpp"

class PatternDetector {
public:
    std::vector<std::string> detectPatterns(const std::string& code, 
                                           const AnalysisData& astData);
    
private:
    bool detectBubbleSort(const std::string& code);
    bool detectQuickSort(const std::string& code);
    bool detectMergeSort(const std::string& code);
    bool detectBinarySearch(const std::string& code);
    bool detectDynamicProgramming(const std::string& code);
    bool detectGraphAlgorithm(const std::string& code);
};

#endif // PATTERN_DETECTOR_HPP