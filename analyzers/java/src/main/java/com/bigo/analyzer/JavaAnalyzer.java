package com.bigo.analyzer;

import com.bigo.models.ComplexityResult;
import com.bigo.models.ComplexityClass;
import com.github.javaparser.JavaParser;
import com.github.javaparser.ParseResult;
import com.github.javaparser.ast.CompilationUnit;
import com.github.javaparser.ast.visitor.VoidVisitor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

public class JavaAnalyzer {
    private static final Logger logger = LoggerFactory.getLogger(JavaAnalyzer.class);
    private final JavaParser parser;
    
    public JavaAnalyzer() {
        this.parser = new JavaParser();
    }
    
    public ComplexityResult analyze(String code) {
        try {
            // Parse the Java code
            ParseResult<CompilationUnit> parseResult = parser.parse(code);
            
            if (!parseResult.isSuccessful()) {
                throw new RuntimeException("Failed to parse Java code: " + 
                    parseResult.getProblems().toString());
            }
            
            CompilationUnit cu = parseResult.getResult().orElseThrow();
            
            // Create AST analyzer
            ASTAnalyzer astAnalyzer = new ASTAnalyzer();
            cu.accept(astAnalyzer, null);
            
            // Calculate complexity
            ComplexityCalculator calculator = new ComplexityCalculator();
            ComplexityResult result = calculator.calculate(astAnalyzer);
            
            // Detect patterns
            PatternDetector patternDetector = new PatternDetector();
            List<String> patterns = patternDetector.detectPatterns(cu);
            result.setPatterns(patterns);
            
            return result;
            
        } catch (Exception e) {
            logger.error("Error analyzing Java code", e);
            throw new RuntimeException("Analysis failed: " + e.getMessage());
        }
    }
}