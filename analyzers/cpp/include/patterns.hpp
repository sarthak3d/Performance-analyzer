#ifndef PATTERNS_HPP
#define PATTERNS_HPP

#include <string>
#include <vector>
#include <regex>
#include "complexity.hpp"

namespace cpp_analyzer {

class PatternDetector {
public:
    PatternDetector();
    
    CodeFeatures detectFeatures(const std::string& code);
    
private:
    // Loop detection
    int countLoopDepth(const std::string& code);
    bool hasLogarithmicPattern(const std::string& code);
    
    // Recursion detection
    bool detectRecursion(const std::string& code);
    int countRecursiveCalls(const std::string& code, const std::string& functionName);
    
    // Algorithm detection
    bool detectSorting(const std::string& code);
    bool detectBinarySearch(const std::string& code);
    bool detectDynamicProgramming(const std::string& code);
    
    // STL detection
    bool detectSTLSort(const std::string& code);
    bool detectSTLContainers(const std::string& code);
    
    // Helper methods
    std::vector<std::string> extractFunctionNames(const std::string& code);
    std::string removeComments(const std::string& code);
    std::string removeStrings(const std::string& code);
};

} // namespace cpp_analyzer

#endif // PATTERNS_HPP