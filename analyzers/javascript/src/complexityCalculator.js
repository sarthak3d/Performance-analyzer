class ComplexityCalculator {
    calculate(astData, patterns) {
        const timeComplexity = this.calculateTimeComplexity(astData, patterns);
        const spaceComplexity = this.calculateSpaceComplexity(astData, patterns);
        
        const result = {
            timeComplexity: this.complexityToString(timeComplexity),
            spaceComplexity: this.complexityToString(spaceComplexity),
            bestCaseTime: this.complexityToString(timeComplexity),
            averageCaseTime: this.complexityToString(timeComplexity),
            worstCaseTime: this.complexityToString(timeComplexity),
            bestCaseSpace: this.complexityToString(spaceComplexity),
            averageCaseSpace: this.complexityToString(spaceComplexity),
            worstCaseSpace: this.complexityToString(spaceComplexity),
            confidence: this.calculateConfidence(astData, patterns),
            explanation: this.generateExplanation(astData, patterns),
            patterns: patterns,
            suggestions: this.generateSuggestions(astData, patterns, timeComplexity),
            recursive: astData.hasRecursion,
            hasLoops: astData.maxLoopDepth > 0,
            loopDepth: astData.maxLoopDepth
        };
        
        // Adjust for specific cases
        this.adjustForSpecificCases(result, astData, patterns);
        
        return result;
    }

    calculateTimeComplexity(astData, patterns) {
        // Check for sorting
        if (astData.arrayMethods.includes('sort') || patterns.includes('sorting')) {
            if (astData.maxLoopDepth > 0) {
                return 'O_N_SQUARED';
            }
            return 'O_N_LOG_N';
        }
        
        // Check for nested array methods
        const iterativeMethods = ['map', 'filter', 'forEach', 'reduce'];
        const nestedArrayOps = iterativeMethods.filter(m => 
            astData.arrayMethods.filter(am => am === m).length > 1
        );
        
        if (nestedArrayOps.length > 0 && astData.maxLoopDepth > 0) {
            return 'O_N_SQUARED';
        }
        
        // Check for recursion
        if (astData.hasRecursion) {
            if (patterns.includes('memoization') || patterns.includes('dynamic_programming')) {
                return 'O_N';
            }
            if (astData.recursiveFunctions.length > 1 || patterns.includes('multiple_recursion')) {
                return 'O_2_N';
            }
            if (patterns.includes('tail_recursion')) {
                return 'O_N';
            }
            return astData.maxLoopDepth > 0 ? 'O_N_SQUARED' : 'O_N';
        }
        
        // Check for logarithmic patterns
        if (astData.functionComplexity.logarithmic_pattern || patterns.includes('binary_search')) {
            return 'O_LOG_N';
        }
        
        // Based on loop depth
        switch (astData.maxLoopDepth) {
            case 0: return 'O_1';
            case 1: return 'O_N';
            case 2: return 'O_N_SQUARED';
            case 3: return 'O_N_CUBED';
            default: return 'O_2_N';
        }
    }

    calculateSpaceComplexity(astData, patterns) {
        // Recursion uses call stack
        if (astData.hasRecursion) {
            return 'O_N';
        }
        
        // Array methods that create new arrays
        const creatingMethods = ['map', 'filter', 'concat', 'slice'];
        const hasCreatingMethods = creatingMethods.some(m => astData.arrayMethods.includes(m));
        
        if (hasCreatingMethods) {
            if (astData.maxLoopDepth > 1) {
                return 'O_N_SQUARED';
            }
            return 'O_N';
        }
        
        // Generators are memory efficient
        if (astData.hasGenerators) {
            return 'O_1';
        }
        
        return 'O_1';
    }

    adjustForSpecificCases(result, astData, patterns) {
        // Adjust for sorting algorithms
        if (patterns.includes('quicksort')) {
            result.bestCaseTime = 'O(n log n)';
            result.averageCaseTime = 'O(n log n)';
            result.worstCaseTime = 'O(n²)';
        } else if (patterns.includes('mergesort')) {
            result.bestCaseTime = 'O(n log n)';
            result.averageCaseTime = 'O(n log n)';
            result.worstCaseTime = 'O(n log n)';
        } else if (astData.arrayMethods.includes('sort')) {
            // JavaScript's sort is typically TimSort
            result.bestCaseTime = 'O(n)';
            result.averageCaseTime = 'O(n log n)';
            result.worstCaseTime = 'O(n log n)';
        }
    }

    calculateConfidence(astData, patterns) {
        let confidence = 0.6; // Base confidence
        
        if (astData.maxLoopDepth > 0) confidence += 0.15;
        if (astData.hasRecursion) confidence += 0.1;
        if (patterns.length > 0) confidence += Math.min(0.15, patterns.length * 0.05);
        if (astData.arrayMethods.length > 0) confidence += 0.05;
        
        return Math.min(confidence, 0.95);
    }

    generateExplanation(astData, patterns) {
        const explanations = [];
        
        if (astData.maxLoopDepth > 0) {
            explanations.push(`Found ${astData.maxLoopDepth} level(s) of nested loops`);
        }
        
        if (astData.hasRecursion) {
            explanations.push(`Detected recursive functions: ${astData.recursiveFunctions.join(', ')}`);
        }
        
        if (astData.arrayMethods.includes('sort')) {
            explanations.push('Uses array sort method');
        }
        
        if (astData.hasAsync) {
            explanations.push('Contains asynchronous operations');
        }
        
        if (astData.hasGenerators) {
            explanations.push('Uses generator functions');
        }
        
        if (patterns.includes('dom_manipulation')) {
            explanations.push('Performs DOM manipulation');
        }
        
        return explanations.length > 0 
            ? explanations.join('. ') + '.'
            : 'Sequential operations detected.';
    }

    generateSuggestions(astData, patterns, timeComplexity) {
        const suggestions = [];
        
        if (timeComplexity === 'O_N_SQUARED' || timeComplexity === 'O_N_CUBED') {
            suggestions.push('Consider optimizing nested loops or using more efficient algorithms');
        }
        
        if (timeComplexity === 'O_2_N' && astData.hasRecursion) {
            suggestions.push('Consider using memoization or dynamic programming to optimize recursive calls');
        }
        
        if (patterns.includes('dom_in_loop')) {
            suggestions.push('Avoid DOM manipulation inside loops for better performance');
        }
        
        if (astData.arrayMethods.filter(m => m === 'map').length > 2) {
            suggestions.push('Consider combining multiple map operations into a single pass');
        }
        
        return suggestions;
    }

    complexityToString(complexity) {
        const map = {
            'O_1': 'O(1)',
            'O_LOG_N': 'O(log n)',
            'O_SQRT_N': 'O(√n)',
            'O_N': 'O(n)',
            'O_N_LOG_N': 'O(n log n)',
            'O_N_SQUARED': 'O(n²)',
            'O_N_CUBED': 'O(n³)',
            'O_2_N': 'O(2^n)',
            'O_N_FACTORIAL': 'O(n!)'
        };
        
        return map[complexity] || 'O(?)';
    }
}

module.exports = { ComplexityCalculator };