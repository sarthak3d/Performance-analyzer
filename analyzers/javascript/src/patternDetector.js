class PatternDetector {
    detectPatterns(code, astData) {
        const patterns = [];
        
        // Algorithm patterns - structure-based detection
        if (this.detectSortingPattern(code, astData)) patterns.push('sorting');
        if (this.detectBinarySearchPattern(code, astData)) patterns.push('binary_search');
        if (this.detectDivideAndConquerPattern(astData)) patterns.push('divide_and_conquer');
        if (this.detectExponentialPattern(code, astData)) patterns.push('exponential');
        
        // Specific sorting algorithms
        if (this.detectQuickSortPattern(astData)) patterns.push('quicksort');
        if (this.detectMergeSortPattern(astData)) patterns.push('mergesort');
        if (this.detectBubbleSortPattern(astData)) patterns.push('bubble_sort');
        
        // Recursion patterns
        if (astData.hasRecursion) {
            patterns.push('recursion');
            if (this.detectTailRecursion(code, astData)) patterns.push('tail_recursion');
            if (this.detectMemoization(code, astData)) patterns.push('memoization');
        }
        
        // Array patterns
        if (astData.arrayMethods.length > 0) {
            patterns.push('array_operations');
            if (this.detectMethodChaining(astData)) patterns.push('method_chaining');
        }
        
        // Async patterns
        if (astData.hasAsync) patterns.push('async_await');
        if (astData.asyncPatterns.includes('promises')) patterns.push('promises');
        
        // DOM patterns
        if (this.detectDOMManipulation(code)) {
            patterns.push('dom_manipulation');
            if (this.detectDOMInLoop(code, astData)) patterns.push('dom_in_loop');
        }
        
        // Optimization patterns
        if (this.detectDynamicProgramming(code, astData)) patterns.push('dynamic_programming');
        
        return patterns;
    }

    detectSortingPattern(code, astData) {
        // Structure-based detection
        if (astData.arrayMethods.includes('sort')) return true;
        
        // Nested loops with swap operations
        if (astData.maxLoopDepth >= 2) {
            const hasComparison = astData.binaryOperations.some(op => 
                ['<', '>', '<=', '>='].includes(op.operator) && op.context === 'in_loop'
            );
            
            // Check for array access patterns
            const hasArrayAccess = code.includes('[') && code.includes(']');
            const hasIndexAccess = /\[\s*\w+\s*\]/.test(code) && 
                                  /\[\s*\w+\s*[+-]\s*\d+\s*\]/.test(code);
            
            if (hasComparison && (hasArrayAccess || hasIndexAccess)) return true;
        }
        
        return astData.divideAndConquerSignals.includes('sorting_pattern');
    }

    detectBinarySearchPattern(code, astData) {
        // Structure-based detection
        const indicators = [
            astData.divideAndConquerSignals.includes('binary_search_pattern'),
            astData.divideAndConquerSignals.includes('division_by_2'),
            astData.loopPatterns.some(p => p.complexity === 'logarithmic')
        ];
        
        // Check for mid calculation pattern
        const hasMidCalc = /\w+[\s]*=[\s]*.*[\/>>].*2/.test(code) ||
                          /Math\.floor\s*\(.*[\/].*2\s*\)/.test(code);
        
        // Check for range updates
        const hasRangeUpdate = /\w+[\s]*=[\s]*\w+[\s]*[+-][\s]*1/.test(code);
        
        return indicators.filter(Boolean).length >= 2 || 
               (hasMidCalc && hasRangeUpdate);
    }

    detectDivideAndConquerPattern(astData) {
        const signals = astData.divideAndConquerSignals;
        
        return signals.includes('divide_and_conquer_recursion') ||
               signals.includes('range_parameters') ||
               (astData.hasRecursion && signals.includes('division_by_2'));
    }

    detectQuickSortPattern(astData) {
        // QuickSort has specific patterns: pivot selection, partitioning
        if (!astData.hasRecursion) return false;
        
        const hasPartitioning = astData.functionCalls.some(call => 
            /partition/i.test(call)
        );
        
        const hasPivot = astData.variableUpdates && 
                         Object.keys(astData.variableUpdates).some(v => /pivot/i.test(v));
        
        // Structure: recursive with multiple calls and range parameters
        const hasRangeParams = astData.divideAndConquerSignals.includes('range_parameters');
        const hasMultipleRecursion = astData.recursionPatterns.some(p => p.hasMultipleCalls);
        
        return (hasPartitioning || hasPivot) || 
               (hasRangeParams && hasMultipleRecursion && astData.maxLoopDepth >= 1);
    }

    detectMergeSortPattern(astData) {
        // MergeSort pattern: divide in half, recursive calls, merging
        if (!astData.hasRecursion) return false;
        
        const hasMerge = astData.functionCalls.some(call => 
            /merge/i.test(call)
        );
        
        const hasDivision = astData.recursionPatterns.some(p => p.hasDivision);
        const hasMultipleCalls = astData.recursionPatterns.some(p => p.hasMultipleCalls);
        
        return hasMerge || (hasDivision && hasMultipleCalls);
    }

    detectBubbleSortPattern(astData) {
        // Bubble sort: nested loops with adjacent element comparison
        if (astData.maxLoopDepth < 2) return false;
        
        // Check for swap pattern in nested loops
        const hasNestedLoops = astData.maxLoopDepth >= 2;
        const hasComparison = astData.binaryOperations.some(op => 
            ['<', '>', '<=', '>='].includes(op.operator) && op.context === 'in_loop'
        );
        
        return hasNestedLoops && hasComparison;
    }

    detectTailRecursion(code, astData) {
        // Tail recursion: return statement directly calls the function
        if (!astData.hasRecursion) return false;
        
        // Check if recursive call is in tail position
        const recursiveFuncs = astData.recursiveFunctions;
        for (const func of recursiveFuncs) {
            const pattern = new RegExp(`return\\s+${func}\\s*\\(`);  // Match return followed by function name and opening parenthesis
            if (pattern.test(code)) return true;
        }
        
        return false;
    }

    detectMemoization(code, astData) {
        // Look for caching patterns
        const cacheIndicators = [
            /memo|cache|dp/i.test(code),
            /Map|Set|Object\.create/.test(code),
            code.includes('[') && code.includes(']') && astData.hasRecursion
        ];
        
        // Check for object/array used as cache
        const hasCacheStructure = code.includes('{}') || code.includes('[]') || 
                                  code.includes('new Map') || code.includes('new Set');
        
        return cacheIndicators.filter(Boolean).length >= 2 || 
               (hasCacheStructure && astData.hasRecursion);
    }

    detectMethodChaining(astData) {
        // Check for consecutive array method calls
        const chainableMethods = ['map', 'filter', 'reduce', 'sort', 'forEach'];
        let chainCount = 0;
        
        for (let i = 0; i < astData.arrayMethods.length - 1; i++) {
            if (chainableMethods.includes(astData.arrayMethods[i]) &&
                chainableMethods.includes(astData.arrayMethods[i + 1])) {
                chainCount++;
            }
        }
        
        return chainCount >= 1;
    }

    detectDOMManipulation(code) {
        const domKeywords = [
            'document.', 'getElementById', 'querySelector', 
            'getElementsBy', 'createElement', 'appendChild',
            'innerHTML', 'textContent', 'setAttribute',
            'addEventListener', 'removeChild', 'replaceChild'
        ];
        
        return domKeywords.some(keyword => code.includes(keyword));
    }

    detectDOMInLoop(code, astData) {
        if (!this.detectDOMManipulation(code) || astData.maxLoopDepth === 0) {
            return false;
        }
        
        // More sophisticated pattern matching for DOM in loops
        const domPatterns = [
            /(for|while|forEach|map)[\s\S]{0,200}(document\.|getElementById|querySelector)/,
            /\.(forEach|map|filter|reduce)[\s\S]{0,100}(appendChild|innerHTML|textContent)/
        ];
        
        return domPatterns.some(pattern => pattern.test(code));
    }

    detectDynamicProgramming(code, astData) {
        // DP patterns: memoization, tabulation, optimal substructure
        const dpIndicators = [
            /(dp|memo|table|cache)/i.test(code),
            /Math\.(min|max)/.test(code) && astData.hasRecursion,
            astData.variableUpdates && Object.keys(astData.variableUpdates).some(v => 
                /dp|memo|table/i.test(v)
            ),
            // Check for 2D array pattern (common in DP)
            /\[\s*\w+\s*\]\s*\[\s*\w+\s*\]/.test(code)
        ];
        
        return dpIndicators.filter(Boolean).length >= 2;
    }
    // Add these methods to PatternDetector class

    detectExponentialPattern(code, astData) {
        // Check for classic exponential patterns
        
        // Fibonacci-like pattern
        if (astData.hasRecursion && astData.recursionPatterns.some(p => p.callCount >= 2)) {
            // Check if it's making multiple recursive calls with n-1, n-2 style arguments
            const hasFibPattern = /KATEX_INLINE_OPEN\s*\w+\s*-\s*1\s*KATEX_INLINE_CLOSE.*KATEX_INLINE_OPEN\s*\w+\s*-\s*2\s*KATEX_INLINE_CLOSE/.test(code) ||
                                /KATEX_INLINE_OPEN\s*\w+\s*-\s*2\s*KATEX_INLINE_CLOSE.*KATEX_INLINE_OPEN\s*\w+\s*-\s*1\s*KATEX_INLINE_CLOSE/.test(code);
            if (hasFibPattern) return true;
        }
        
        // Subset/powerset generation pattern
        if (astData.hasRecursion) {
            const hasSubsetPattern = /include|exclude|with|without/i.test(code) &&
                                    astData.recursionPatterns.some(p => p.callCount >= 2);
            if (hasSubsetPattern) return true;
        }
        
        // General exponential recursion: multiple calls without division
        return astData.recursionPatterns.some(p => p.callCount >= 2 && !p.hasDivision);
    }
}

module.exports = { PatternDetector };