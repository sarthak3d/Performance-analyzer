#ifndef COMPLEXITY_HPP
#define COMPLEXITY_HPP

#include <string>
#include <vector>

namespace cpp_analyzer {

enum class Complexity {
    O_1,        // O(1) - Constant
    O_LOG_N,    // O(log n) - Logarithmic
    O_SQRT_N,   // O(√n) - Square root
    O_N,        // O(n) - Linear
    O_N_LOG_N,  // O(n log n) - Linearithmic
    O_N_2,      // O(n²) - Quadratic
    O_N_3,      // O(n³) - Cubic
    O_2_N,      // O(2^n) - Exponential
    O_N_FACT    // O(n!) - Factorial
};

struct CodeFeatures {
    int loopDepth = 0;
    int maxNestedLoops = 0;
    bool hasRecursion = false;
    bool hasMultipleRecursiveCalls = false;
    bool hasLogarithmicLoop = false;
    bool hasSorting = false;
    bool hasBinarySearch = false;
    bool hasDynamicProgramming = false;
    bool hasSTLSort = false;
    bool hasSTLContainers = false;
    std::vector<std::string> recursiveFunctions;
    std::vector<std::string> detectedPatterns;
    int recursiveCallCount = 0;
};

class ComplexityAnalyzer {
public:
    ComplexityAnalyzer();
    
    Complexity analyzeTime(const CodeFeatures& features);
    Complexity analyzeSpace(const CodeFeatures& features);
    std::string toString(Complexity c);
    double calculateConfidence(const CodeFeatures& features);
    std::string generateExplanation(const CodeFeatures& features, Complexity timeComplexity);
    
private:
    Complexity determineLoopComplexity(const CodeFeatures& features);
    Complexity determineRecursiveComplexity(const CodeFeatures& features);
};

} // namespace cpp_analyzer

#endif // COMPLEXITY_HPP