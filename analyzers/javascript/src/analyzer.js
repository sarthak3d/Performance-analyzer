const { ASTAnalyzer } = require('./astAnalyzer');
const { ComplexityCalculator } = require('./complexityCalculator');
const { PatternDetector } = require('./patternDetector');

class JavaScriptAnalyzer {
    constructor() {
        this.astAnalyzer = new ASTAnalyzer();
        this.calculator = new ComplexityCalculator();
        this.patternDetector = new PatternDetector();
    }

    async analyze(code) {
        try {
            // Parse and analyze AST
            const astData = this.astAnalyzer.analyze(code);
            
            // Detect patterns
            const patterns = this.patternDetector.detectPatterns(code, astData);
            
            // Calculate complexity
            const result = this.calculator.calculate(astData, patterns);
            
            return result;
            
        } catch (error) {
            throw new Error(`Analysis failed: ${error.message}`);
        }
    }
}

module.exports = { JavaScriptAnalyzer };