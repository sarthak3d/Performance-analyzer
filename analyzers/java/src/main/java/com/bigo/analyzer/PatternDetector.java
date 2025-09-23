package com.bigo.analyzer;

import com.github.javaparser.ast.CompilationUnit;
import com.github.javaparser.ast.Node;
import com.github.javaparser.ast.body.MethodDeclaration;
import com.github.javaparser.ast.visitor.VoidVisitorAdapter;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class PatternDetector {
    
    public List<String> detectPatterns(CompilationUnit cu) {
        List<String> patterns = new ArrayList<>();
        
        // Get string representation for text-based pattern matching
        String code = cu.toString();
        
        // Algorithm patterns
        if (detectBubbleSort(code)) patterns.add("bubble_sort");
        if (detectQuickSort(code)) patterns.add("quick_sort");
        if (detectMergeSort(code)) patterns.add("merge_sort");
        if (detectBinarySearch(code)) patterns.add("binary_search");
        if (detectDynamicProgramming(code)) patterns.add("dynamic_programming");
        
        // Java-specific patterns
        if (detectStreamAPI(code)) patterns.add("stream_api");
        if (detectLambdas(code)) patterns.add("lambdas");
        if (detectGenerics(code)) patterns.add("generics");
        if (detectCollections(code)) patterns.add("collections");
        
        // Structural patterns
        PatternVisitor visitor = new PatternVisitor();
        cu.accept(visitor, patterns);
        
        return patterns;
    }
    
    private boolean detectBubbleSort(String code) {
        Pattern pattern = Pattern.compile("(bubble|Bubble).*(sort|Sort)", Pattern.CASE_INSENSITIVE);
        if (pattern.matcher(code).find()) return true;
        
        // Look for nested loops with swap
        Pattern swapPattern = Pattern.compile("for.*for.*(swap|temp\\s*=)");
        return swapPattern.matcher(code).find();
    }
    
    private boolean detectQuickSort(String code) {
        Pattern pattern = Pattern.compile("(quick|Quick).*(sort|Sort)", Pattern.CASE_INSENSITIVE);
        if (pattern.matcher(code).find()) return true;
        
        // Look for pivot and partition
        return code.contains("pivot") || code.contains("partition");
    }
    
    private boolean detectMergeSort(String code) {
        Pattern pattern = Pattern.compile("(merge|Merge).*(sort|Sort)", Pattern.CASE_INSENSITIVE);
        if (pattern.matcher(code).find()) return true;
        
        // Look for merge with mid
        return code.contains("merge") && code.contains("mid");
    }
    
    private boolean detectBinarySearch(String code) {
        Pattern pattern = Pattern.compile("(binary|Binary).*(search|Search)", Pattern.CASE_INSENSITIVE);
        if (pattern.matcher(code).find()) return true;
        
        // Look for left, right, mid pattern
        return (code.contains("left") && code.contains("right") && code.contains("mid")) ||
               (code.contains("low") && code.contains("high") && code.contains("mid"));
    }
    
    private boolean detectDynamicProgramming(String code) {
        // Look for DP patterns
        return code.contains("dp[") || code.contains("memo[") || 
               code.contains("cache.") || code.contains("memoization");
    }
    
    private boolean detectStreamAPI(String code) {
        return code.contains(".stream()") || code.contains("Stream<");
    }
    
    private boolean detectLambdas(String code) {
        return code.contains("->") && code.contains("(");
    }
    
    private boolean detectGenerics(String code) {
        Pattern pattern = Pattern.compile("<[A-Z]>");
        return pattern.matcher(code).find();
    }
    
    private boolean detectCollections(String code) {
        String[] collections = {"ArrayList", "LinkedList", "HashMap", "TreeMap", 
                               "HashSet", "TreeSet", "PriorityQueue"};
        for (String collection : collections) {
            if (code.contains(collection)) return true;
        }
        return false;
    }
    
    // Visitor for structural patterns
    private static class PatternVisitor extends VoidVisitorAdapter<List<String>> {
        @Override
        public void visit(MethodDeclaration method, List<String> patterns) {
            super.visit(method, patterns);
            
            // Check for recursion indicator in method name
            String methodName = method.getNameAsString();
            if (methodName.toLowerCase().contains("recursive") || 
                methodName.toLowerCase().contains("recur")) {
                if (!patterns.contains("recursion")) {
                    patterns.add("recursion");
                }
            }
            
            // Check for synchronized methods
            if (method.isSynchronized()) {
                if (!patterns.contains("synchronized")) {
                    patterns.add("synchronized");
                }
            }
        }
    }
}