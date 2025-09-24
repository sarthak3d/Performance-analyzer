#include "analyzer.hpp"
#include "patterns.hpp"
#include "complexity.hpp"
#include <sstream>

namespace cpp_analyzer {

struct Analyzer::Impl {
    PatternDetector patternDetector;
    ComplexityAnalyzer complexityAnalyzer;
};

Analyzer::Analyzer() : pImpl(std::make_unique<Impl>()) {}

Analyzer::~Analyzer() = default;

AnalysisResult Analyzer::analyze(const std::string& code) {
    AnalysisResult result;
    
    // Detect features in the code
    CodeFeatures features = pImpl->patternDetector.detectFeatures(code);
    
    // Analyze complexity
    Complexity timeComplexity = pImpl->complexityAnalyzer.analyzeTime(features);
    Complexity spaceComplexity = pImpl->complexityAnalyzer.analyzeSpace(features);
    
    // Convert to string
    result.timeComplexity = pImpl->complexityAnalyzer.toString(timeComplexity);
    result.spaceComplexity = pImpl->complexityAnalyzer.toString(spaceComplexity);
    
    // Set best/average/worst cases
    if (features.hasSorting || features.hasSTLSort) {
        result.bestCaseTime = "O(n)";
        result.averageCaseTime = "O(n log n)";
        result.worstCaseTime = "O(n log n)";
    } else {
        result.bestCaseTime = result.timeComplexity;
        result.averageCaseTime = result.timeComplexity;
        result.worstCaseTime = result.timeComplexity;
    }
    
    result.bestCaseSpace = result.spaceComplexity;
    result.averageCaseSpace = result.spaceComplexity;
    result.worstCaseSpace = result.spaceComplexity;
    
    // Calculate confidence
    result.confidence = pImpl->complexityAnalyzer.calculateConfidence(features);
    
    // Generate explanation
    result.explanation = pImpl->complexityAnalyzer.generateExplanation(features, timeComplexity);
    
    // Set patterns
    result.patterns = features.detectedPatterns;
    
    // Set metadata
    result.recursive = features.hasRecursion;
    result.hasLoops = features.loopDepth > 0;
    result.loopDepth = features.loopDepth;
    
    // Generate suggestions
    if (timeComplexity == Complexity::O_N_2 || timeComplexity == Complexity::O_N_3) {
        result.suggestions.push_back("Consider optimizing nested loops");
    }
    if (features.hasMultipleRecursiveCalls) {
        result.suggestions.push_back("Consider using memoization or dynamic programming");
    }
    
    return result;
}

} // namespace cpp_analyzer