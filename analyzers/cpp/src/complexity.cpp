#include "complexity.hpp"
#include <sstream>
#include <algorithm>

namespace cpp_analyzer {

ComplexityAnalyzer::ComplexityAnalyzer() {}

Complexity ComplexityAnalyzer::analyzeTime(const CodeFeatures& features) {
    // Priority 1: Exponential recursion
    if (features.hasMultipleRecursiveCalls) {
        return Complexity::O_2_N;
    }
    
    // Priority 2: Sorting algorithms
    if (features.hasSorting || features.hasSTLSort) {
        if (features.loopDepth == 0) {
            return Complexity::O_N_LOG_N;
        } else if (features.loopDepth == 1) {
            return Complexity::O_N_2;
        } else {
            return Complexity::O_N_3;
        }
    }
    
    // Priority 3: Binary search
    if (features.hasBinarySearch) {
        if (features.loopDepth == 0) {
            return Complexity::O_LOG_N;
        } else if (features.loopDepth == 1) {
            return Complexity::O_N_LOG_N;
        }
    }
    
    // Priority 4: Logarithmic loops
    if (features.hasLogarithmicLoop) {
        if (features.maxNestedLoops == 1) {
            return Complexity::O_LOG_N;
        } else if (features.maxNestedLoops == 2) {
            return Complexity::O_N_LOG_N;
        }
    }
    
    // Priority 5: Simple recursion
    if (features.hasRecursion && !features.hasMultipleRecursiveCalls) {
        if (features.loopDepth == 0) {
            return Complexity::O_N;
        } else if (features.loopDepth == 1) {
            return Complexity::O_N_2;
        }
    }
    
    // Priority 6: Loop-based complexity
    switch (features.maxNestedLoops) {
        case 0: return Complexity::O_1;
        case 1: return Complexity::O_N;
        case 2: return Complexity::O_N_2;
        case 3: return Complexity::O_N_3;
        default: return Complexity::O_N_3;
    }
}

Complexity ComplexityAnalyzer::analyzeSpace(const CodeFeatures& features) {
    // Recursion uses stack space
    if (features.hasRecursion) {
        return Complexity::O_N;
    }
    
    // STL containers
    if (features.hasSTLContainers) {
        return Complexity::O_N;
    }
    
    // Dynamic programming
    if (features.hasDynamicProgramming) {
        if (features.maxNestedLoops >= 2) {
            return Complexity::O_N_2;
        }
        return Complexity::O_N;
    }
    
    // Default to constant space
    return Complexity::O_1;
}

std::string ComplexityAnalyzer::toString(Complexity c) {
    switch (c) {
        case Complexity::O_1: return "O(1)";
        case Complexity::O_LOG_N: return "O(log n)";
        case Complexity::O_SQRT_N: return "O(√n)";
        case Complexity::O_N: return "O(n)";
        case Complexity::O_N_LOG_N: return "O(n log n)";
        case Complexity::O_N_2: return "O(n²)";
        case Complexity::O_N_3: return "O(n³)";
        case Complexity::O_2_N: return "O(2^n)";
        case Complexity::O_N_FACT: return "O(n!)";
        default: return "O(?)";
    }
}

double ComplexityAnalyzer::calculateConfidence(const CodeFeatures& features) {
    double confidence = 0.7; // Base confidence
    
    if (features.loopDepth > 0) confidence += 0.1;
    if (features.hasRecursion) confidence += 0.1;
    if (features.hasLogarithmicLoop) confidence += 0.1;
    if (features.hasSorting || features.hasSTLSort) confidence += 0.05;
    
    return std::min(confidence, 0.95);
}

std::string ComplexityAnalyzer::generateExplanation(const CodeFeatures& features, Complexity timeComplexity) {
    std::stringstream ss;
    
    if (features.hasMultipleRecursiveCalls) {
        ss << "Detected exponential recursion with " << features.recursiveCallCount << " recursive calls. ";
    } else if (features.hasRecursion) {
        ss << "Detected linear recursion. ";
    }
    
    if (features.hasLogarithmicLoop) {
        ss << "Detected logarithmic loop pattern. ";
    }
    
    if (features.maxNestedLoops > 0) {
        ss << "Found " << features.maxNestedLoops << " nested loop(s). ";
    }
    
    if (features.hasSorting || features.hasSTLSort) {
        ss << "Uses sorting algorithm. ";
    }
    
    if (features.hasBinarySearch) {
        ss << "Uses binary search. ";
    }
    
    if (ss.str().empty()) {
        ss << "Constant time operations detected.";
    }
    
    return ss.str();
}

} // namespace cpp_analyzer