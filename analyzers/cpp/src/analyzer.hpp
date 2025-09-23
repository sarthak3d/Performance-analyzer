#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include <string>
#include <vector>
#include <memory>

struct ComplexityResult {
    std::string timeComplexity;
    std::string spaceComplexity;
    std::string bestCaseTime;
    std::string averageCaseTime;
    std::string worstCaseTime;
    std::string bestCaseSpace;
    std::string averageCaseSpace;
    std::string worstCaseSpace;
    double confidence;
    std::string explanation;
    std::vector<std::string> patterns;
    std::vector<std::string> suggestions;
    bool recursive;
    bool hasLoops;
    int loopDepth;
};

// Forward declaration
class ComplexityCalculator;

class CppAnalyzer {
public:
    CppAnalyzer();
    ~CppAnalyzer();
    
    ComplexityResult analyze(const std::string& code);
    
private:
    std::unique_ptr<ComplexityCalculator> calculator;
};

#endif // ANALYZER_HPP