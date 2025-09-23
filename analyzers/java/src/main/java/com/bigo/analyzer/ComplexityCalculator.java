package com.bigo.analyzer;

import com.bigo.models.ComplexityClass;
import com.bigo.models.ComplexityResult;

import java.util.List;

public class ComplexityCalculator {
    
    public ComplexityResult calculate(ASTAnalyzer astData) {
        ComplexityResult result = new ComplexityResult();
        
        // Calculate time complexity
        ComplexityClass timeComplexity = calculateTimeComplexity(astData);
        result.setTimeComplexity(timeComplexity.toString());
        
        // Calculate space complexity
        ComplexityClass spaceComplexity = calculateSpaceComplexity(astData);
        result.setSpaceComplexity(spaceComplexity.toString());
        
        // Determine best/average/worst cases
        determineCases(result, astData);
        
        // Calculate confidence
        double confidence = calculateConfidence(astData);
        result.setConfidence(confidence);
        
        // Generate explanation
        String explanation = generateExplanation(astData);
        result.setExplanation(explanation);
        
        // Add metadata
        result.setRecursive(astData.hasRecursion());
        result.setLoopDepth(astData.getMaxLoopDepth());
        result.setHasLoops(astData.getMaxLoopDepth() > 0);
        
        return result;
    }
    
    private ComplexityClass calculateTimeComplexity(ASTAnalyzer astData) {
        int loopDepth = astData.getMaxLoopDepth();
        boolean hasRecursion = astData.hasRecursion();
        List<String> patterns = astData.getAlgorithmPatterns();
        
        // Check for sorting
        if (patterns.contains("sorting") || patterns.contains("arrays_sort") || 
            patterns.contains("collections_sort") || patterns.contains("stream_sort")) {
            if (loopDepth > 0) {
                return ComplexityClass.O_N_SQUARED;
            }
            return ComplexityClass.O_N_LOG_N;
        }
        
        // Check for logarithmic patterns
        if (patterns.contains("logarithmic_loop") || patterns.contains("logarithmic_while")) {
            return ComplexityClass.O_LOG_N;
        }
        
        // Recursion
        if (hasRecursion) {
            if (loopDepth == 0) {
                // Check for multiple recursive calls (exponential)
                if (astData.getRecursiveMethods().size() > 1) {
                    return ComplexityClass.O_2_N;
                }
                return ComplexityClass.O_N;
            } else {
                return ComplexityClass.O_N_SQUARED;
            }
        }
        
        // Stream API
        if (patterns.contains("stream_api")) {
            // Multiple stream operations are still O(n)
            return ComplexityClass.O_N;
        }
        
        // Based on loop depth
        switch (loopDepth) {
            case 0: return ComplexityClass.O_1;
            case 1: return ComplexityClass.O_N;
            case 2: return ComplexityClass.O_N_SQUARED;
            case 3: return ComplexityClass.O_N_CUBED;
            default: return ComplexityClass.O_2_N;
        }
    }
    
    private ComplexityClass calculateSpaceComplexity(ASTAnalyzer astData) {
        boolean hasRecursion = astData.hasRecursion();
        int loopDepth = astData.getMaxLoopDepth();
        List<String> patterns = astData.getAlgorithmPatterns();
        
        // Recursion uses call stack
        if (hasRecursion) {
            return ComplexityClass.O_N;
        }
        
        // Stream operations may create intermediate collections
        if (patterns.contains("stream_api")) {
            return ComplexityClass.O_N;
        }
        
        // Default to constant space
        return ComplexityClass.O_1;
    }
    
    private void determineCases(ComplexityResult result, ASTAnalyzer astData) {
        String timeComplexity = result.getTimeComplexity();
        
        // For sorting algorithms
        if (astData.getAlgorithmPatterns().contains("sorting")) {
            result.setBestCaseTime("O(n)");
            result.setAverageCaseTime("O(n log n)");
            result.setWorstCaseTime("O(n log n)");
        } else {
            // Default: all cases are the same
            result.setBestCaseTime(timeComplexity);
            result.setAverageCaseTime(timeComplexity);
            result.setWorstCaseTime(timeComplexity);
        }
        
        // Space complexity usually doesn't vary
        String spaceComplexity = result.getSpaceComplexity();
        result.setBestCaseSpace(spaceComplexity);
        result.setAverageCaseSpace(spaceComplexity);
        result.setWorstCaseSpace(spaceComplexity);
    }
    
    private double calculateConfidence(ASTAnalyzer astData) {
        double confidence = 0.7; // Base confidence for AST analysis
        
        if (astData.getAlgorithmPatterns().size() > 0) {
            confidence += 0.1;
        }
        
        if (astData.hasRecursion()) {
            confidence += 0.05;
        }
        
        if (astData.getMaxLoopDepth() > 0) {
            confidence += 0.1;
        }
        
        return Math.min(confidence, 0.95);
    }
    
    private String generateExplanation(ASTAnalyzer astData) {
        StringBuilder explanation = new StringBuilder();
        
        if (astData.getMaxLoopDepth() > 0) {
            explanation.append("Found ").append(astData.getMaxLoopDepth())
                      .append(" level(s) of nested loops. ");
        }
        
        if (astData.hasRecursion()) {
            explanation.append("Detected recursive methods: ")
                      .append(String.join(", ", astData.getRecursiveMethods()))
                      .append(". ");
        }
        
        if (astData.getAlgorithmPatterns().contains("sorting")) {
            explanation.append("Uses sorting algorithm. ");
        }
        
        if (astData.getAlgorithmPatterns().contains("stream_api")) {
            explanation.append("Uses Java Stream API. ");
        }
        
        if (explanation.length() == 0) {
            explanation.append("Sequential operations detected.");
        }
        
        return explanation.toString().trim();
    }
}