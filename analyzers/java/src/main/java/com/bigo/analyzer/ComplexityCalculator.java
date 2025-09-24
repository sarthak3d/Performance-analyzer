package com.bigo.analyzer;

import com.bigo.models.ComplexityClass;
import com.bigo.models.ComplexityResult;

import java.util.List;
import java.util.Map;

public class ComplexityCalculator {
    
    public ComplexityResult calculate(ASTAnalyzer astData) {
        ComplexityResult result = new ComplexityResult();
        
        // Calculate time complexity with improved logic
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
        String explanation = generateExplanation(astData, timeComplexity);
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
        boolean hasLogarithmicLoop = astData.hasLogarithmicLoop();
        boolean hasExponentialRecursion = astData.hasExponentialRecursion();
        List<String> patterns = astData.getAlgorithmPatterns();
        Map<String, Integer> recursiveCallCount = astData.getMethodRecursiveCallCount();
        
        // HIGHEST PRIORITY: Exponential complexity
        if (hasExponentialRecursion || 
            patterns.contains("exponential_recursion") ||
            patterns.contains("multiple_recursive_calls_in_return")) {
            return ComplexityClass.O_2_N;
        }
        
        // Check recursive call counts for exponential pattern
        for (Integer count : recursiveCallCount.values()) {
            if (count >= 2) {
                return ComplexityClass.O_2_N;
            }
        }
        
        // PRIORITY 2: Sorting algorithms (but not in nested loops)
        if (patterns.contains("sorting") || patterns.contains("arrays_sort") || 
            patterns.contains("collections_sort") || patterns.contains("stream_sort")) {
            if (loopDepth > 1) {  // Changed from > 0 to > 1
                // Sorting inside nested loops
                return ComplexityClass.O_N_CUBED;
            } else if (loopDepth == 1) {
                // Sorting inside a single loop
                return ComplexityClass.O_N_SQUARED;
            }
            return ComplexityClass.O_N_LOG_N;
        }
        
        // PRIORITY 3: Logarithmic patterns with correct nesting handling
        if (hasLogarithmicLoop || 
            patterns.contains("logarithmic_for_loop") ||
            patterns.contains("logarithmic_while_loop") ||
            patterns.contains("logarithmic_do_while_loop")) {
            
            // Pure logarithmic (loop with logarithmic increment)
            if (loopDepth <= 1) {
                return ComplexityClass.O_LOG_N;
            }
            // Logarithmic nested with linear
            else if (loopDepth == 2) {
                return ComplexityClass.O_N_LOG_N;
            }
            // More complex nesting
            else {
                return ComplexityClass.O_N_SQUARED;
            }
        }
        
        // PRIORITY 4: Handle Stream API separately
        if (patterns.contains("stream_api") || patterns.contains("enhanced_for")) {
            // Check if stream/enhanced-for is inside another loop
            // But don't double count if it's the only loop
            if (loopDepth > 1) {
                return ComplexityClass.O_N_SQUARED;
            } else if (loopDepth == 1) {
                return ComplexityClass.O_N;
            }
        }
        
        // PRIORITY 5: Simple recursion (not exponential)
        if (hasRecursion && !hasExponentialRecursion) {
            // Linear recursion without loops
            if (loopDepth == 0) {
                return ComplexityClass.O_N;
            } 
            // Be careful with recursion + loops
            // Most tail recursion with simple loops is still O(n)
            else if (loopDepth == 1) {
                // Check if it's tail recursion (usually linear)
                if (recursiveCallCount.size() == 1 && 
                    recursiveCallCount.values().stream().allMatch(c -> c == 1)) {
                    return ComplexityClass.O_N;  // Tail recursion with loop is still O(n)
                }
                return ComplexityClass.O_N_SQUARED;
            } 
            else {
                return ComplexityClass.O_N_CUBED;
            }
        }
        
        // PRIORITY 6: Standard loop-based complexity
        switch (loopDepth) {
            case 0: 
                return ComplexityClass.O_1;
            case 1: 
                return ComplexityClass.O_N;
            case 2: 
                return ComplexityClass.O_N_SQUARED;
            case 3: 
                return ComplexityClass.O_N_CUBED;
            default: 
                // More than 3 nested loops
                return ComplexityClass.O_N_CUBED;  // Conservative estimate
        }
    }
    
    private ComplexityClass calculateSpaceComplexity(ASTAnalyzer astData) {
        boolean hasRecursion = astData.hasRecursion();
        boolean hasExponentialRecursion = astData.hasExponentialRecursion();
        List<String> patterns = astData.getAlgorithmPatterns();
        
        // Check for array/list creation patterns
        if (patterns.contains("array_creation") || patterns.contains("list_creation")) {
            return ComplexityClass.O_N;
        }
        
        // Exponential recursion typically uses O(n) call stack space
        if (hasExponentialRecursion || 
            patterns.contains("exponential_recursion") ||
            patterns.contains("multiple_recursive_calls_in_return")) {
            return ComplexityClass.O_N;
        }
        
        // Simple recursion uses O(n) stack space
        if (hasRecursion) {
            return ComplexityClass.O_N;
        }
        
        // Stream operations may create intermediate collections
        if (patterns.contains("stream_api")) {
            return ComplexityClass.O_N;
        }
        
        // Sorting typically requires O(1) to O(n) space
        if (patterns.contains("sorting") || patterns.contains("arrays_sort")) {
            return ComplexityClass.O_N;  // Conservative for merge sort
        }
        
        // Default to constant space
        return ComplexityClass.O_1;
    }
    
    private void determineCases(ComplexityResult result, ASTAnalyzer astData) {
        String timeComplexity = result.getTimeComplexity();
        
        // Sorting algorithms have different best/average/worst cases
        if (astData.getAlgorithmPatterns().contains("sorting") ||
            astData.getAlgorithmPatterns().contains("arrays_sort") ||
            astData.getAlgorithmPatterns().contains("collections_sort")) {
            result.setBestCaseTime("O(n)");
            result.setAverageCaseTime("O(n log n)");
            result.setWorstCaseTime("O(n log n)");
        } else {
            // Default: all cases are the same
            result.setBestCaseTime(timeComplexity);
            result.setAverageCaseTime(timeComplexity);
            result.setWorstCaseTime(timeComplexity);
        }
        
        // Space complexity usually doesn't vary by case
        String spaceComplexity = result.getSpaceComplexity();
        result.setBestCaseSpace(spaceComplexity);
        result.setAverageCaseSpace(spaceComplexity);
        result.setWorstCaseSpace(spaceComplexity);
    }
    
    private double calculateConfidence(ASTAnalyzer astData) {
        double confidence = 0.7; // Base confidence
        
        // Specific detections increase confidence
        if (astData.hasLogarithmicLoop()) {
            confidence += 0.15;
        }
        
        if (astData.hasExponentialRecursion()) {
            confidence += 0.15;
        }
        
        if (astData.getAlgorithmPatterns().size() > 0) {
            confidence += 0.1;
        }
        
        if (astData.hasRecursion()) {
            confidence += 0.05;
        }
        
        if (astData.getMaxLoopDepth() > 0) {
            confidence += 0.1;
        }
        
        // Pattern-specific confidence boosts
        if (astData.getAlgorithmPatterns().contains("logarithmic_for_loop") ||
            astData.getAlgorithmPatterns().contains("logarithmic_while_loop")) {
            confidence += 0.05;
        }
        
        if (astData.getAlgorithmPatterns().contains("multiple_recursive_calls_in_return")) {
            confidence += 0.05;
        }
        
        return Math.min(confidence, 0.95);
    }
    
    private String generateExplanation(ASTAnalyzer astData, ComplexityClass complexity) {
        StringBuilder explanation = new StringBuilder();
        
        // Exponential recursion
        if (astData.hasExponentialRecursion()) {
            explanation.append("Detected exponential recursion pattern. ");
            Map<String, Integer> callCounts = astData.getMethodRecursiveCallCount();
            for (Map.Entry<String, Integer> entry : callCounts.entrySet()) {
                if (entry.getValue() >= 2) {
                    explanation.append("Method '").append(entry.getKey())
                              .append("' makes ").append(entry.getValue())
                              .append(" recursive calls. ");
                }
            }
        }
        
        // Logarithmic patterns
        if (astData.hasLogarithmicLoop()) {
            explanation.append("Detected logarithmic loop pattern (variable multiplied/divided by constant in each iteration). ");
        }
        
        // Loop depth (but be more nuanced)
        if (astData.getMaxLoopDepth() > 0 && !astData.hasLogarithmicLoop()) {
            if (astData.getMaxLoopDepth() == 1) {
                explanation.append("Found single loop iterating over collection. ");
            } else {
                explanation.append("Found ").append(astData.getMaxLoopDepth())
                          .append(" level(s) of nested loops. ");
            }
        }
        
        // Simple recursion
        if (astData.hasRecursion() && !astData.hasExponentialRecursion()) {
            explanation.append("Detected recursive method(s): ")
                      .append(String.join(", ", astData.getRecursiveMethods()))
                      .append(". ");
        }
        
        // Sorting
        if (astData.getAlgorithmPatterns().contains("sorting") ||
            astData.getAlgorithmPatterns().contains("arrays_sort")) {
            explanation.append("Uses sorting algorithm. ");
        }
        
        // Stream API
        if (astData.getAlgorithmPatterns().contains("stream_api")) {
            explanation.append("Uses Java Stream API. ");
        }
        
        // Enhanced for loop
        if (astData.getAlgorithmPatterns().contains("enhanced_for")) {
            explanation.append("Uses enhanced for loop. ");
        }
        
        // Default explanations
        if (explanation.length() == 0) {
            if (complexity == ComplexityClass.O_1) {
                explanation.append("Constant time operations - no loops or recursion detected.");
            } else if (complexity == ComplexityClass.O_N) {
                explanation.append("Linear time complexity - single loop or linear recursion.");
            } else {
                explanation.append("Complexity determined by code structure.");
            }
        }
        
        return explanation.toString().trim();
    }
}