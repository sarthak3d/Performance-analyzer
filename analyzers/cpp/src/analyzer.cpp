#include "analyzer.hpp"
#include "ast_visitor.hpp"
#include "pattern_detector.hpp"
#include "complexity_calculator.hpp"  // Now safe to include
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <memory>

CppAnalyzer::CppAnalyzer() 
    : calculator(std::make_unique<ComplexityCalculator>()) {
}

CppAnalyzer::~CppAnalyzer() = default;

ComplexityResult CppAnalyzer::analyze(const std::string& code) {
    // Create AST visitor
    auto visitor = std::make_unique<ASTVisitor>();
    
    // Parse and analyze the code using Clang
    std::vector<std::string> args = {"-std=c++17"};
    
    // Use clang tooling to parse the code
    auto ast = clang::tooling::buildASTFromCodeWithArgs(code, args, "input.cpp");
    
    if (ast) {
        visitor->TraverseAST(ast->getASTContext());
    }
    
    // Detect patterns
    PatternDetector patternDetector;
    auto patterns = patternDetector.detectPatterns(code, visitor->getData());
    
    // Calculate complexity
    ComplexityResult result = calculator->calculate(visitor->getData(), patterns);
    
    return result;
}