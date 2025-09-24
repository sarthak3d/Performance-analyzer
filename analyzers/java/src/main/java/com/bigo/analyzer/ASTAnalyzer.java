package com.bigo.analyzer;

import com.github.javaparser.ast.body.MethodDeclaration;
import com.github.javaparser.ast.expr.*;
import com.github.javaparser.ast.stmt.*;
import com.github.javaparser.ast.visitor.VoidVisitorAdapter;

import java.util.*;

public class ASTAnalyzer extends VoidVisitorAdapter<Void> {
    private int currentLoopDepth = 0;
    private int maxLoopDepth = 0;
    private List<String> recursiveMethods = new ArrayList<>();
    private Set<String> currentMethodNames = new HashSet<>();
    private Map<String, Integer> methodComplexity = new HashMap<>();
    private Map<String, Integer> methodRecursiveCallCount = new HashMap<>();
    private boolean hasRecursion = false;
    private List<String> algorithmPatterns = new ArrayList<>();
    private String currentMethodName = "";
    private boolean hasLogarithmicLoop = false;
    private boolean hasExponentialRecursion = false;
    private int totalRecursiveCallsInMethod = 0;
    
    @Override
    public void visit(MethodDeclaration method, Void arg) {
        String methodName = method.getNameAsString();
        currentMethodNames.add(methodName);
        String previousMethodName = currentMethodName;
        currentMethodName = methodName;
        totalRecursiveCallsInMethod = 0;
        
        // Visit method body
        super.visit(method, arg);
        
        // Store recursive call count for this method
        if (totalRecursiveCallsInMethod > 0) {
            methodRecursiveCallCount.put(methodName, totalRecursiveCallsInMethod);
            if (totalRecursiveCallsInMethod >= 2) {
                hasExponentialRecursion = true;
                algorithmPatterns.add("exponential_recursion");
            }
        }
        
        // Store method complexity
        methodComplexity.put(methodName, maxLoopDepth);
        currentMethodNames.remove(methodName);
        currentMethodName = previousMethodName;
    }
    
    @Override
    public void visit(ForStmt forStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        
        // Analyze the for loop for logarithmic patterns
        boolean isLogarithmic = analyzeForLoopComplexity(forStmt);
        if (isLogarithmic) {
            hasLogarithmicLoop = true;
            algorithmPatterns.add("logarithmic_for_loop");
            // Don't count this as a regular loop for complexity calculation
            currentLoopDepth--;
            maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
            currentLoopDepth++;
        }
        
        super.visit(forStmt, arg);
        currentLoopDepth--;
    }
    
    private boolean analyzeForLoopComplexity(ForStmt forStmt) {
        // Check the update expression
        if (!forStmt.getUpdate().isEmpty()) {
            for (Expression update : forStmt.getUpdate()) {
                // Direct check for logarithmic patterns
                String updateStr = update.toString();
                
                // Look for *= /= <<= >>= patterns
                if (updateStr.contains("*=") && !updateStr.contains("+=")) {
                    return true;
                }
                if (updateStr.contains("/=") && !updateStr.contains("-=")) {
                    return true;
                }
                if (updateStr.contains("<<=") || updateStr.contains(">>=")) {
                    return true;
                }
                
                // Check AssignExpr specifically
                if (update instanceof AssignExpr) {
                    AssignExpr assign = (AssignExpr) update;
                    
                    // Check for compound assignment operators
                    if (assign.getOperator() == AssignExpr.Operator.MULTIPLY ||
                        assign.getOperator() == AssignExpr.Operator.DIVIDE ||
                        assign.getOperator() == AssignExpr.Operator.LEFT_SHIFT ||
                        assign.getOperator() == AssignExpr.Operator.SIGNED_RIGHT_SHIFT ||
                        assign.getOperator() == AssignExpr.Operator.UNSIGNED_RIGHT_SHIFT) {
                        return true;
                    }
                    
                    // Check for i = i * 2 pattern
                    if (assign.getOperator() == AssignExpr.Operator.ASSIGN) {
                        Expression value = assign.getValue();
                        if (value instanceof BinaryExpr) {
                            BinaryExpr binary = (BinaryExpr) value;
                            BinaryExpr.Operator op = binary.getOperator();
                            
                            // Check if left side matches the target variable
                            String target = assign.getTarget().toString();
                            String left = binary.getLeft().toString();
                            
                            if (target.equals(left) && 
                                (op == BinaryExpr.Operator.MULTIPLY ||
                                 op == BinaryExpr.Operator.DIVIDE ||
                                 op == BinaryExpr.Operator.LEFT_SHIFT ||
                                 op == BinaryExpr.Operator.SIGNED_RIGHT_SHIFT ||
                                 op == BinaryExpr.Operator.UNSIGNED_RIGHT_SHIFT)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        
        return false;
    }
    
    @Override
    public void visit(WhileStmt whileStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        
        // Check for logarithmic patterns in while loop body
        LogarithmicPatternDetector detector = new LogarithmicPatternDetector();
        whileStmt.getBody().accept(detector, null);
        
        if (detector.isLogarithmic()) {
            hasLogarithmicLoop = true;
            algorithmPatterns.add("logarithmic_while_loop");
            // Adjust loop depth for logarithmic complexity
            currentLoopDepth--;
            maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
            currentLoopDepth++;
        }
        
        super.visit(whileStmt, arg);
        currentLoopDepth--;
    }
    
    @Override
    public void visit(DoStmt doStmt, Void arg) {
        currentLoopDepth++;
        maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
        
        // Check for logarithmic patterns in do-while loop body
        LogarithmicPatternDetector detector = new LogarithmicPatternDetector();
        doStmt.getBody().accept(detector, null);
        
        if (detector.isLogarithmic()) {
            hasLogarithmicLoop = true;
            algorithmPatterns.add("logarithmic_do_while_loop");
            // Adjust loop depth for logarithmic complexity
            currentLoopDepth--;
            maxLoopDepth = Math.max(maxLoopDepth, currentLoopDepth);
            currentLoopDepth++;
        }
        
        super.visit(doStmt, arg);
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
    public void visit(MethodCallExpr methodCall, Void arg) {
        String methodName = methodCall.getNameAsString();
        
        // Check for recursive calls
        if (currentMethodName.equals(methodName)) {
            hasRecursion = true;
            totalRecursiveCallsInMethod++;
            if (!recursiveMethods.contains(methodName)) {
                recursiveMethods.add(methodName);
            }
        }
        
        // Check for known complexity methods
        checkBuiltInMethodComplexity(methodCall);
        
        super.visit(methodCall, arg);
    }
    
    @Override
    public void visit(ReturnStmt returnStmt, Void arg) {
        if (returnStmt.getExpression().isPresent()) {
            Expression expr = returnStmt.getExpression().get();
            
            // Count recursive calls in the return statement
            RecursiveCallCounter counter = new RecursiveCallCounter(currentMethodName);
            expr.accept(counter, null);
            
            int callsInReturn = counter.getCount();
            if (callsInReturn >= 2) {
                hasExponentialRecursion = true;
                algorithmPatterns.add("multiple_recursive_calls_in_return");
                // Add to total count (but don't double count if already visited)
                totalRecursiveCallsInMethod = Math.max(totalRecursiveCallsInMethod, callsInReturn);
            }
        }
        
        super.visit(returnStmt, arg);
    }
    
    @Override
    public void visit(BinaryExpr binaryExpr, Void arg) {
        // Count recursive calls in binary expressions (like fib(n-1) + fib(n-2))
        if (currentMethodName != null && !currentMethodName.isEmpty()) {
            RecursiveCallCounter counter = new RecursiveCallCounter(currentMethodName);
            binaryExpr.accept(counter, null);
            
            int calls = counter.getCount();
            if (calls >= 2) {
                hasExponentialRecursion = true;
                totalRecursiveCallsInMethod = Math.max(totalRecursiveCallsInMethod, calls);
            }
        }
        
        super.visit(binaryExpr, arg);
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
    
    // Helper class to detect logarithmic patterns
    private static class LogarithmicPatternDetector extends VoidVisitorAdapter<Void> {
        private boolean logarithmic = false;
        
        @Override
        public void visit(AssignExpr n, Void arg) {
            // Check for logarithmic assignment patterns
            AssignExpr.Operator op = n.getOperator();
            
            if (op == AssignExpr.Operator.MULTIPLY ||
                op == AssignExpr.Operator.DIVIDE ||
                op == AssignExpr.Operator.LEFT_SHIFT ||
                op == AssignExpr.Operator.SIGNED_RIGHT_SHIFT ||
                op == AssignExpr.Operator.UNSIGNED_RIGHT_SHIFT) {
                logarithmic = true;
            }
            
            // Check for x = x * 2 or x = x / 2 patterns
            if (op == AssignExpr.Operator.ASSIGN) {
                Expression value = n.getValue();
                String target = n.getTarget().toString();
                
                if (value instanceof BinaryExpr) {
                    BinaryExpr binary = (BinaryExpr) value;
                    String left = binary.getLeft().toString();
                    
                    if (target.equals(left)) {
                        BinaryExpr.Operator binOp = binary.getOperator();
                        if (binOp == BinaryExpr.Operator.MULTIPLY ||
                            binOp == BinaryExpr.Operator.DIVIDE ||
                            binOp == BinaryExpr.Operator.LEFT_SHIFT ||
                            binOp == BinaryExpr.Operator.SIGNED_RIGHT_SHIFT ||
                            binOp == BinaryExpr.Operator.UNSIGNED_RIGHT_SHIFT) {
                            logarithmic = true;
                        }
                    }
                }
            }
            
            super.visit(n, arg);
        }
        
        public boolean isLogarithmic() {
            return logarithmic;
        }
    }
    
    // Helper class to count recursive calls
    private static class RecursiveCallCounter extends VoidVisitorAdapter<Void> {
        private final String methodName;
        private int count = 0;
        
        public RecursiveCallCounter(String methodName) {
            this.methodName = methodName;
        }
        
        @Override
        public void visit(MethodCallExpr n, Void arg) {
            if (n.getNameAsString().equals(methodName)) {
                count++;
            }
            super.visit(n, arg);
        }
        
        public int getCount() {
            return count;
        }
    }
    
    // Getters
    public int getMaxLoopDepth() { return maxLoopDepth; }
    public boolean hasRecursion() { return hasRecursion; }
    public boolean hasLogarithmicLoop() { return hasLogarithmicLoop; }
    public boolean hasExponentialRecursion() { return hasExponentialRecursion; }
    public List<String> getRecursiveMethods() { return recursiveMethods; }
    public List<String> getAlgorithmPatterns() { return algorithmPatterns; }
    public Map<String, Integer> getMethodComplexity() { return methodComplexity; }
    public Map<String, Integer> getMethodRecursiveCallCount() { return methodRecursiveCallCount; }
}