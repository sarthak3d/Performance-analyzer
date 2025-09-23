class PatternDetector {
    detectPatterns(code, astData) {
        const patterns = [];
        
        // Algorithm patterns
        if (this.detectBubbleSort(code)) patterns.push('bubble_sort');
        if (this.detectQuickSort(code)) patterns.push('quicksort');
        if (this.detectMergeSort(code)) patterns.push('mergesort');
        if (this.detectBinarySearch(code)) patterns.push('binary_search');
        
        // Recursion patterns
        if (astData.hasRecursion) {
            patterns.push('recursion');
            if (this.detectTailRecursion(code)) patterns.push('tail_recursion');
            if (this.detectMemoization(code)) patterns.push('memoization');
        }
        
        // Array patterns
        if (astData.arrayMethods.length > 0) {
            patterns.push('array_operations');
            if (this.detectMethodChaining(code)) patterns.push('method_chaining');
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
        if (this.detectDynamicProgramming(code)) patterns.push('dynamic_programming');
        
        return patterns;
    }

    detectBubbleSort(code) {
        return code.includes('bubble') && code.includes('sort') ||
               (code.includes('for') && code.includes('swap') && code.includes('j < i'));
    }

    detectQuickSort(code) {
        return code.includes('quick') && code.includes('sort') ||
               (code.includes('pivot') && code.includes('partition'));
    }

    detectMergeSort(code) {
        return code.includes('merge') && code.includes('sort') ||
               (code.includes('merge') && code.includes('mid') && code.includes('recursive'));
    }

    detectBinarySearch(code) {
        return code.includes('binary') && code.includes('search') ||
               (code.includes('mid') && code.includes('left') && 
                code.includes('right') && code.includes('while'));
    }

    detectTailRecursion(code) {
        // Simple heuristic: return statement directly calls the function
        return /return\s+\w+KATEX_INLINE_OPEN[^)]*KATEX_INLINE_CLOSE/.test(code);
    }

    detectMemoization(code) {
        return code.includes('memo') || code.includes('cache') ||
               code.includes('dp[') || code.includes('dp.get');
    }

    detectMethodChaining(code) {
        return /\.\w+KATEX_INLINE_OPEN[^)]*KATEX_INLINE_CLOSE\.\w+KATEX_INLINE_OPEN[^)]*KATEX_INLINE_CLOSE/.test(code);
    }

    detectDOMManipulation(code) {
        const domKeywords = [
            'document.', 'getElementById', 'querySelector', 
            'getElementsBy', 'createElement', 'appendChild',
            'innerHTML', 'textContent', 'setAttribute'
        ];
        
        return domKeywords.some(keyword => code.includes(keyword));
    }

    detectDOMInLoop(code, astData) {
        if (!this.detectDOMManipulation(code) || astData.maxLoopDepth === 0) {
            return false;
        }
        
        // Simple heuristic: DOM operation keywords appear near loop keywords
        const loopPattern = /(for|while)[\s\S]{0,200}(document\.|getElementById|querySelector)/;
        return loopPattern.test(code);
    }

    detectDynamicProgramming(code) {
        return code.includes('dp[') || code.includes('memo[') ||
               (code.includes('table[') && code.includes('Math.min'));
    }
}

module.exports = { PatternDetector };