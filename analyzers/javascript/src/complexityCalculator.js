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
        // First check for exponential patterns (highest complexity)
        if (this.hasExponentialPattern(astData, patterns)) {
            return 'O_2_N';
        }
        
        // Check for logarithmic patterns
        if (this.hasLogarithmicPattern(astData, patterns)) {
            if (astData.maxLoopDepth === 0 && !astData.hasRecursion) {
                return 'O_LOG_N';
            } else if (astData.maxLoopDepth === 1) {
                // Check if the loop itself is logarithmic
                const hasLogLoop = astData.loopPatterns.some(p => p.complexity === 'logarithmic');
                if (hasLogLoop) {
                    return 'O_LOG_N';
                }
                return 'O_N_LOG_N';
            } else {
                return 'O_N_LOG_N';
            }
        }
        
        // Check for sorting patterns
        if (this.hasSortingPattern(astData, patterns)) {
            // Check for efficient sorting
            if (patterns.includes('divide_and_conquer_sorting') ||
                astData.divideAndConquerSignals.includes('divide_and_conquer_recursion')) {
                return 'O_N_LOG_N';
            }
            // Nested loops with sorting typically indicate O(n²)
            if (astData.maxLoopDepth >= 2) {
                return 'O_N_SQUARED';
            }
            return 'O_N_LOG_N';
        }
        
        // Check for recursion patterns
        if (astData.hasRecursion) {
            return this.calculateRecursiveComplexity(astData, patterns);
        }
        
        // Check for nested array operations
        const nestedArrayOps = this.countNestedArrayOperations(astData);
        if (nestedArrayOps >= 2) {
            return 'O_N_SQUARED';
        }
        
        // Special case: array methods that are O(n)
        if (astData.arrayMethods.length > 0 && astData.maxLoopDepth === 0) {
            const linearMethods = ['map', 'filter', 'forEach', 'reduce', 'find', 'some', 'every'];
            if (astData.arrayMethods.some(m => linearMethods.includes(m))) {
                return 'O_N';
            }
        }
        
        // Based on loop depth and patterns
        if (astData.maxLoopDepth === 0) {
            return 'O_1';
        }
        
        // Check loop patterns for more accurate analysis
        const complexities = astData.loopPatterns.map(p => p.complexity);
        const hasLogLoop = complexities.includes('logarithmic');
        const hasExpLoop = complexities.includes('exponential_growth');
        
        if (hasExpLoop) {
            return 'O_2_N';
        }
        
        if (hasLogLoop && astData.maxLoopDepth === 1) {
            return 'O_LOG_N';
        }
        
        // Standard loop depth analysis
        switch (astData.maxLoopDepth) {
            case 1: return 'O_N';
            case 2: return 'O_N_SQUARED';
            case 3: return 'O_N_CUBED';
            default: return 'O_2_N';
        }
    }

    hasExponentialPattern(astData, patterns) {
        // Direct indicators of exponential complexity
        if (astData.divideAndConquerSignals.includes('multiple_recursion_exponential') ||
            astData.divideAndConquerSignals.includes('fibonacci_pattern')) {
            return true;
        }
        
        // Check recursive call counts
        if (astData.recursiveCallCounts) {
            for (const [func, count] of Object.entries(astData.recursiveCallCounts)) {
                // If a function has 2 or more recursive calls and no divide-and-conquer signal
                if (count >= 2 && !astData.divideAndConquerSignals.includes('divide_and_conquer_recursion')) {
                    return true;
                }
            }
        }
        
        // Check recursion patterns directly
        if (astData.recursionPatterns && astData.recursionPatterns.length > 0) {
            const hasMultipleCallsNoDivision = astData.recursionPatterns.some(p => 
                p.callCount >= 2 && !p.hasDivision
            );
            
            if (hasMultipleCallsNoDivision) {
                return true;
            }
        }
        
        // Pattern-based detection
        if (patterns.includes('exponential') || 
            patterns.includes('fibonacci') ||
            patterns.includes('subset_generation')) {
            return true;
        }
        
        // Very deep loops (4+) might indicate exponential
        if (astData.maxLoopDepth >= 4) {
            return true;
        }
        
        return false;
    }

    hasLogarithmicPattern(astData, patterns) {
        // Check multiple indicators for logarithmic complexity
        const indicators = [
            astData.divideAndConquerSignals.includes('division_by_2'),
            astData.divideAndConquerSignals.includes('bit_shift'),
            astData.divideAndConquerSignals.includes('logarithmic_operation'),
            astData.divideAndConquerSignals.includes('logarithmic_loops'),
            astData.divideAndConquerSignals.includes('binary_search_pattern'),
            astData.divideAndConquerSignals.includes('math_division'),
            astData.loopPatterns.some(p => p.complexity === 'logarithmic'),
            patterns.includes('binary_search'),
            patterns.includes('divide_and_conquer')
        ];
        
        // Need at least 2 indicators for confidence
        return indicators.filter(Boolean).length >= 1;
    }

    hasSortingPattern(astData, patterns) {
        return astData.arrayMethods.includes('sort') ||
               patterns.includes('sorting') ||
               patterns.includes('quicksort') ||
               patterns.includes('mergesort') ||
               patterns.includes('heapsort') ||
               astData.divideAndConquerSignals.includes('sorting_pattern');
    }

    calculateRecursiveComplexity(astData, patterns) {
        // Check for memoization or dynamic programming (optimized recursion)
        if (patterns.includes('memoization') || patterns.includes('dynamic_programming')) {
            return 'O_N';
        }
        
        // Check for exponential indicators
        if (astData.divideAndConquerSignals.includes('multiple_recursion_exponential') ||
            astData.divideAndConquerSignals.includes('fibonacci_pattern')) {
            return 'O_2_N';
        }
        
        // Check recursive call counts
        if (astData.recursiveCallCounts) {
            const maxCalls = Math.max(...Object.values(astData.recursiveCallCounts));
            
            if (maxCalls >= 2) {
                // Multiple calls - check if it's divide-and-conquer or exponential
                const hasDivision = astData.recursionPatterns.some(p => p.hasDivision);
                
                if (!hasDivision) {
                    return 'O_2_N';  // Exponential
                } else {
                    return 'O_N_LOG_N';  // Divide and conquer
                }
            }
        }
        
        // Check for logarithmic recursion
        if (astData.divideAndConquerSignals.includes('logarithmic_recursion')) {
            return 'O_LOG_N';
        }
        
        // Linear recursion
        if (astData.divideAndConquerSignals.includes('linear_recursion')) {
            return 'O_N';
        }
        
        // Default
        return 'O_N';
    }

    countNestedArrayOperations(astData) {
        const iterativeMethods = ['map', 'filter', 'forEach', 'reduce', 'flatMap'];
        let nestingLevel = 0;
        
        // Count how many iterative array methods are used
        iterativeMethods.forEach(method => {
            const count = astData.arrayMethods.filter(m => m === method).length;
            nestingLevel = Math.max(nestingLevel, count);
        });
        
        // Also consider loop depth with array operations
        if (astData.maxLoopDepth > 0 && astData.arrayMethods.length > 0) {
            nestingLevel += astData.maxLoopDepth;
        }
        
        return nestingLevel;
    }

    calculateSpaceComplexity(astData, patterns) {
        // Recursion uses call stack
        if (astData.hasRecursion) {
            // Exponential recursion typically has O(n) space due to call stack
            if (astData.divideAndConquerSignals.includes('multiple_recursion_exponential') ||
                astData.recursionPatterns.some(p => p.callCount >= 2 && !p.hasDivision)) {
                return 'O_N';
            }
            
            if (astData.divideAndConquerSignals.includes('divide_and_conquer_recursion')) {
                // Divide and conquer typically has O(log n) call stack
                return 'O_LOG_N';
            }
            
            return 'O_N';
        }
        
        // Array methods that create new arrays
        const creatingMethods = ['map', 'filter', 'concat', 'slice', 'flat', 'flatMap'];
        const hasCreatingMethods = creatingMethods.some(m => astData.arrayMethods.includes(m));
        
        if (hasCreatingMethods) {
            const count = this.countNestedArrayOperations(astData);
            if (count >= 2) {
                return 'O_N_SQUARED';
            }
            return 'O_N';
        }
        
        // Generators are memory efficient
        if (astData.hasGenerators) {
            return 'O_1';
        }
        
        // Check for data structures that grow with input
        if (patterns.includes('dynamic_programming') || patterns.includes('memoization')) {
            return 'O_N';
        }
        
        // Check if we're using multiple arrays in nested loops
        if (astData.maxLoopDepth >= 2 && hasCreatingMethods) {
            return 'O_N_SQUARED';
        }
        
        return 'O_1';
    }
    adjustForSpecificCases(result, astData, patterns) {
        // Adjust for sorting algorithms
        if (patterns.includes('quicksort') || 
            (this.hasSortingPattern(astData, patterns) && 
             astData.divideAndConquerSignals.includes('divide_and_conquer_recursion'))) {
            result.bestCaseTime = 'O(n log n)';
            result.averageCaseTime = 'O(n log n)';
            result.worstCaseTime = 'O(n²)';
            result.averageCaseSpace = 'O(log n)';
        } else if (patterns.includes('mergesort')) {
            result.bestCaseTime = 'O(n log n)';
            result.averageCaseTime = 'O(n log n)';
            result.worstCaseTime = 'O(n log n)';
            result.worstCaseSpace = 'O(n)';
        } else if (astData.arrayMethods.includes('sort')) {
            // JavaScript's sort is typically TimSort
            result.bestCaseTime = 'O(n)';
            result.averageCaseTime = 'O(n log n)';
            result.worstCaseTime = 'O(n log n)';
        }
        
        // Adjust for binary search pattern
        if (patterns.includes('binary_search') || 
            astData.divideAndConquerSignals.includes('binary_search_pattern')) {
            result.bestCaseTime = 'O(1)';
            result.averageCaseTime = 'O(log n)';
            result.worstCaseTime = 'O(log n)';
        }
    }

    calculateConfidence(astData, patterns) {
        let confidence = 0.5; // Base confidence
        
        // Increase confidence based on clear indicators
        if (astData.maxLoopDepth > 0) confidence += 0.15;
        if (astData.hasRecursion) confidence += 0.1;
        if (patterns.length > 0) confidence += Math.min(0.2, patterns.length * 0.05);
        if (astData.divideAndConquerSignals.length > 0) {
            confidence += Math.min(0.15, astData.divideAndConquerSignals.length * 0.03);
        }
        if (astData.loopPatterns.length > 0) confidence += 0.1;
        
        // Reduce confidence for ambiguous patterns
        if (astData.conditionalComplexity > 10) confidence -= 0.05;
        if (astData.functionComplexity.size > 5) confidence -= 0.05;
        
        return Math.max(0.3, Math.min(confidence, 0.95));
    }

    generateExplanation(astData, patterns) {
        const explanations = [];
        
        // Loop analysis
        if (astData.maxLoopDepth > 0) {
            explanations.push(`Found ${astData.maxLoopDepth} level(s) of nested loops`);
            
            const logLoops = astData.loopPatterns.filter(p => p.complexity === 'logarithmic');
            if (logLoops.length > 0) {
                explanations.push('Detected logarithmic loop pattern(s)');
            }
        }
        
        // Recursion analysis
        if (astData.hasRecursion) {
            const recType = astData.divideAndConquerSignals.includes('multiple_recursion_exponential') 
                ? 'multiple recursive calls' 
                : astData.divideAndConquerSignals.includes('divide_and_conquer_recursion')
                    ? 'divide-and-conquer recursion'
                    : 'recursive function';
            explanations.push(`Detected ${recType}`);
        }
        
        // Algorithm patterns
        if (astData.divideAndConquerSignals.includes('binary_search_pattern')) {
            explanations.push('Binary search pattern detected');
        }
        
        if (this.hasSortingPattern(astData, patterns)) {
            explanations.push('Sorting algorithm detected');
        }
        
        // Array operations
        if (astData.arrayMethods.length > 0) {
            const uniqueMethods = [...new Set(astData.arrayMethods)];
            explanations.push(`Uses array methods: ${uniqueMethods.slice(0, 3).join(', ')}`);
        }
        
        if (astData.hasAsync) {
            explanations.push('Contains asynchronous operations');
        }
        
        if (astData.hasGenerators) {
            explanations.push('Uses generator functions for memory efficiency');
        }
        
        return explanations.length > 0 
            ? explanations.join('. ') + '.'
            : 'Sequential operations detected.';
    }

    generateSuggestions(astData, patterns, timeComplexity) {
        const suggestions = [];
        
        if (timeComplexity === 'O_N_SQUARED' || timeComplexity === 'O_N_CUBED') {
            suggestions.push('Consider optimizing nested loops or using more efficient algorithms');
            
            if (this.hasSortingPattern(astData, patterns) && astData.maxLoopDepth >= 2) {
                suggestions.push('Consider using built-in sort() or a more efficient sorting algorithm');
            }
        }
        
        if (timeComplexity === 'O_2_N' && astData.hasRecursion) {
            suggestions.push('Consider using memoization or dynamic programming to optimize recursive calls');
        }
        
        if (patterns.includes('dom_in_loop')) {
            suggestions.push('Batch DOM operations outside loops for better performance');
        }
        
        const chainedMethods = astData.arrayMethods.filter(m => 
            ['map', 'filter', 'reduce'].includes(m)
        );
        if (chainedMethods.length > 2) {
            suggestions.push('Consider combining multiple array operations into a single pass');
        }
        
        if (astData.maxLoopDepth > 2) {
            suggestions.push('Deep nesting detected - consider refactoring for better readability and performance');
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