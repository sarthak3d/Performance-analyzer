#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include <string>
#include <vector>
#include <memory>
#include "complexity.hpp"

namespace cpp_analyzer {

struct AnalysisResult {
    std::string timeComplexity = "O(1)";
    std::string spaceComplexity = "O(1)";
    std::string bestCaseTime = "O(1)";
    std::string averageCaseTime = "O(1)";
    std::string worstCaseTime = "O(1)";
    std::string bestCaseSpace = "O(1)";
    std::string averageCaseSpace = "O(1)";
    std::string worstCaseSpace = "O(1)";
    double confidence = 0.7;
    std::string explanation = "";
    std::vector<std::string> patterns;
    std::vector<std::string> suggestions;
    bool recursive = false;
    bool hasLoops = false;
    int loopDepth = 0;
};

class Analyzer {
public:
    Analyzer();
    ~Analyzer();
    
    AnalysisResult analyze(const std::string& code);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace cpp_analyzer

#endif // ANALYZER_HPP