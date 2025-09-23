#ifndef COMPLEXITY_CALCULATOR_HPP
#define COMPLEXITY_CALCULATOR_HPP

#include <string>
#include <vector>
#include "ast_visitor.hpp"

// Forward declaration
struct ComplexityResult;

enum class ComplexityClass {
    O_1,
    O_LOG_N,
    O_SQRT_N,
    O_N,
    O_N_LOG_N,
    O_N_SQUARED,
    O_N_CUBED,
    O_2_N,
    O_N_FACTORIAL
};

class ComplexityCalculator {
public:
    ComplexityResult calculate(const AnalysisData& data, 
                               const std::vector<std::string>& patterns);
    
private:
    std::string complexityToString(ComplexityClass complexity);
    ComplexityClass calculateTimeComplexity(const AnalysisData& data,
                                           const std::vector<std::string>& patterns);
    ComplexityClass calculateSpaceComplexity(const AnalysisData& data,
                                            const std::vector<std::string>& patterns);
    double calculateConfidence(const AnalysisData& data);
    std::string generateExplanation(const AnalysisData& data,
                                   const std::vector<std::string>& patterns);
};

#endif // COMPLEXITY_CALCULATOR_HPP