#include "complexity_calculator.hpp"
#include "analyzer.hpp"  // Include for ComplexityResult definition
#include <algorithm>
#include <sstream>

ComplexityResult ComplexityCalculator::calculate(const AnalysisData& data, 
                                                const std::vector<std::string>& patterns) {
    ComplexityResult result;
    
    // Calculate complexities
    ComplexityClass timeComplexity = calculateTimeComplexity(data, patterns);
    ComplexityClass spaceComplexity = calculateSpaceComplexity(data, patterns);
    
    // Set main complexities
    result.timeComplexity = complexityToString(timeComplexity);
    result.spaceComplexity = complexityToString(spaceComplexity);
    
    // Set all cases (can be refined for specific algorithms)
    result.bestCaseTime = result.timeComplexity;
    result.averageCaseTime = result.timeComplexity;
    result.worstCaseTime = result.timeComplexity;
    result.bestCaseSpace = result.spaceComplexity;
    result.averageCaseSpace = result.spaceComplexity;
    result.worstCaseSpace = result.spaceComplexity;
    
    // Calculate confidence
    result.confidence = calculateConfidence(data);
    
    // Generate explanation
    result.explanation = generateExplanation(data, patterns);
    
    // Set patterns
    result.patterns = patterns;
    
    // Set metadata
    result.recursive = data.hasRecursion;
    result.hasLoops = data.maxLoopDepth > 0;
    result.loopDepth = data.maxLoopDepth;
    
    // Generate suggestions
    if (timeComplexity == ComplexityClass::O_N_SQUARED || 
        timeComplexity == ComplexityClass::O_N_CUBED) {
        result.suggestions.push_back("Consider optimizing nested loops");
    }
    if (data.hasRecursion && timeComplexity == ComplexityClass::O_2_N) {
        result.suggestions.push_back("Consider using memoization or dynamic programming");
    }
    
    return result;
}

std::string ComplexityCalculator::complexityToString(ComplexityClass complexity) {
    switch (complexity) {
        case ComplexityClass::O_1: return "O(1)";
        case ComplexityClass::O_LOG_N: return "O(log n)";
        case ComplexityClass::O_SQRT_N: return "O(√n)";
        case ComplexityClass::O_N: return "O(n)";
        case ComplexityClass::O_N_LOG_N: return "O(n log n)";
        case ComplexityClass::O_N_SQUARED: return "O(n²)";
        case ComplexityClass::O_N_CUBED: return "O(n³)";
        case ComplexityClass::O_2_N: return "O(2^n)";
        case ComplexityClass::O_N_FACTORIAL: return "O(n!)";
        default: return "O(?)";
    }
}

ComplexityClass ComplexityCalculator::calculateTimeComplexity(const AnalysisData& data,
                                                             const std::vector<std::string>& patterns) {
    // Check for STL sorting
    auto it = std::find_if(data.stlAlgorithms.begin(), data.stlAlgorithms.end(),
                           [](const std::string& s) { 
                               return s == "sort" || s == "stable_sort" || s == "partial_sort"; 
                           });
    if (it != data.stlAlgorithms.end()) {
        return data.maxLoopDepth > 0 ? ComplexityClass::O_N_SQUARED : ComplexityClass::O_N_LOG_N;
    }
    
    // Check for binary search
    if (std::find(patterns.begin(), patterns.end(), "binary_search") != patterns.end()) {
        return ComplexityClass::O_LOG_N;
    }
    
    // Recursion
    if (data.hasRecursion) {
        if (data.maxLoopDepth == 0) {
            if (data.recursiveFunctions.size() > 1) {
                return ComplexityClass::O_2_N;
            }
            return ComplexityClass::O_N;
        }
        return ComplexityClass::O_N_SQUARED;
    }
    
    // Based on loop depth
    switch (data.maxLoopDepth) {
        case 0: return ComplexityClass::O_1;
        case 1: return ComplexityClass::O_N;
        case 2: return ComplexityClass::O_N_SQUARED;
        case 3: return ComplexityClass::O_N_CUBED;
        default: return ComplexityClass::O_2_N;
    }
}

ComplexityClass ComplexityCalculator::calculateSpaceComplexity(const AnalysisData& data,
                                                              const std::vector<std::string>& patterns) {
    // Recursive calls use stack space
    if (data.hasRecursion) {
        return ComplexityClass::O_N;
    }
    
    // Dynamic memory allocation
    if (data.hasDynamicMemory && data.maxLoopDepth > 0) {
        if (data.maxLoopDepth >= 2) {
            return ComplexityClass::O_N_SQUARED;
        }
        return ComplexityClass::O_N;
    }
    
    // STL containers
    if (!data.stlContainers.empty()) {
        return ComplexityClass::O_N;
    }
    
    return ComplexityClass::O_1;
}

double ComplexityCalculator::calculateConfidence(const AnalysisData& data) {
    double confidence = 0.7; // Base confidence for AST analysis
    
    if (data.maxLoopDepth > 0) confidence += 0.1;
    if (data.hasRecursion) confidence += 0.05;
    if (!data.stlAlgorithms.empty()) confidence += 0.1;
    if (data.hasTemplates) confidence += 0.05;
    
    return std::min(confidence, 0.95);
}

std::string ComplexityCalculator::generateExplanation(const AnalysisData& data,
                                                     const std::vector<std::string>& patterns) {
    std::stringstream ss;
    
    if (data.maxLoopDepth > 0) {
        ss << "Found " << data.maxLoopDepth << " level(s) of nested loops. ";
    }
    
    if (data.hasRecursion) {
        ss << "Detected recursive functions. ";
    }
    
    if (!data.stlAlgorithms.empty()) {
        ss << "Uses STL algorithms: ";
        for (const auto& algo : data.stlAlgorithms) {
            ss << algo << " ";
        }
        ss << ". ";
    }
    
    if (!data.stlContainers.empty()) {
        ss << "Uses STL containers. ";
    }
    
    if (data.hasTemplates) {
        ss << "Uses C++ templates. ";
    }
    
    if (data.hasDynamicMemory) {
        ss << "Performs dynamic memory allocation. ";
    }
    
    std::string explanation = ss.str();
    if (explanation.empty()) {
        explanation = "Sequential operations detected.";
    }
    
    return explanation;
}