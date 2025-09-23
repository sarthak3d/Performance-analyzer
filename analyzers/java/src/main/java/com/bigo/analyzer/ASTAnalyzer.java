package com.bigo.analyzer;

import com.github.javaparser.ast.body.MethodDeclaration;
import com.github.javaparser.ast.expr.MethodCallExpr;
import com.github.javaparser.ast.stmt.*;
import com.github.javaparser.ast.visitor.VoidVisitorAdapter;

import java.util.*;

public class ASTAnalyzer extends VoidVisitorAdapter<Void> {
    private int currentLoopDepth = 0;
    private int maxLoopDepth = 0;
    private List<String> recursiveMethods = new ArrayList<>();
    private List<String> methodCalls = new ArrayList<>();
    private Set<String> currentMethodNames = new HashSet<>();
    private Map<String, Integer> methodComplexity = new HashMap<>();
    private boolean hasRecursion = false;
    private List<String> algorithmPatterns = new ArrayList<>();
    
    @Override
    public void visit(MethodDeclaration method, Void arg) {
        String methodName = method.getNameAsString();
        currentMethodNames.add(methodName);
        
        // Visit method body
        super.visit(method, arg);
        
        // Store method complexity
        methodComplexity.put(methodName, maxLoopDepth);
        currentMethodNames.remove(methodName);
    }
    
    @Override
    public void visit(ForStmt forStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        
        // Check for logarithmic patterns
        if (isLogarithmicLoop(forStmt)) {
            algorithmPatterns.add("logarithmic_loop");
        }
        
        super.visit(forStmt, arg);
        currentLoopDepth--;
    }
    
    @Override
    public void visit(ForEachStmt forEachStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        algorithmPatterns.add("enhanced_for");
        super.visit(forEachStmt, arg);
        currentLoopDepth--;
    }
    
    @Override
    public void visit(WhileStmt whileStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        
        // Check condition for patterns
        String condition = whileStmt.getCondition().toString();
        if (condition.contains("/=") || condition.contains(">>")) {
            algorithmPatterns.add("logarithmic_while");
        }
        
        super.visit(whileStmt, arg);
        currentLoopDepth--;
    }
    
    @Override
    public void visit(DoStmt doStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        super.visit(doStmt, arg);
        currentLoopDepth--;
    }
    
    @Override
    public void visit(MethodCallExpr methodCall, Void arg) {
        String methodName = methodCall.getNameAsString();
        methodCalls.add(methodName);
        
        // Check for recursive calls
        if (currentMethodNames.contains(methodName)) {
            hasRecursion = true;
            recursiveMethods.add(methodName);
        }
        
        // Check for known complexity methods
        checkBuiltInMethodComplexity(methodCall);
        
        super.visit(methodCall, arg);
    }
    
    private boolean isLogarithmicLoop(ForStmt forStmt) {
        String stmt = forStmt.toString();
        return stmt.contains("*=") || stmt.contains("/=") || 
               stmt.contains("<<") || stmt.contains(">>");
    }
    
    private void checkBuiltInMethodComplexity(MethodCallExpr methodCall) {
        String methodName = methodCall.getNameAsString();
        
        // Check for sorting methods
        if (methodName.equals("sort") || methodName.equals("parallelSort")) {
            algorithmPatterns.add("sorting");
        }
        
        // Check for collection operations
        if (methodCall.getScope().isPresent()) {
            String scope = methodCall.getScope().get().toString();
            
            if (scope.contains("Arrays") && methodName.equals("sort")) {
                algorithmPatterns.add("arrays_sort");
            }
            
            if (scope.contains("Collections") && methodName.equals("sort")) {
                algorithmPatterns.add("collections_sort");
            }
            
            // Stream operations
            if (scope.contains("stream()")) {
                algorithmPatterns.add("stream_api");
                
                if (methodName.equals("sorted")) {
                    algorithmPatterns.add("stream_sort");
                }
            }
        }
    }
    
    // Getters
    public int getMaxLoopDepth() { return maxLoopDepth; }
    public boolean hasRecursion() { return hasRecursion; }
    public List<String> getRecursiveMethods() { return recursiveMethods; }
    public List<String> getAlgorithmPatterns() { return algorithmPatterns; }
    public Map<String, Integer> getMethodComplexity() { return methodComplexity; }
}