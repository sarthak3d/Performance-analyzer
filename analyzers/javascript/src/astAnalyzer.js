const esprima = require('esprima');
const estraverse = require('estraverse');

class ASTAnalyzer {
    constructor() {
        this.reset();
    }

    reset() {
        this.currentLoopDepth = 0;
        this.maxLoopDepth = 0;
        this.recursiveFunctions = [];
        this.functionCalls = [];
        this.currentFunctions = [];  // Changed to array to track nesting
        this.functionComplexity = new Map();
        this.arrayMethods = [];
        this.asyncPatterns = [];
        this.hasRecursion = false;
        this.hasGenerators = false;
        this.hasAsync = false;
        this.loopPatterns = [];
        this.binaryOperations = [];
        this.recursionPatterns = [];
        this.divideAndConquerSignals = [];
        this.variableUpdates = new Map();
        this.conditionalComplexity = 0;
        this.functionCallGraph = new Map();
        this.recursiveCallsPerFunction = new Map();
        this.functionBodies = new Map();
    }

    analyze(code) {
        this.reset();
        
        // Parse code into AST
        const ast = esprima.parseScript(code, {
            jsx: true,
            tolerant: true,
            loc: true,
            range: true
        });

        // First pass - collect function bodies
        this.collectFunctionBodies(ast);
        
        // Second pass - detailed analysis
        estraverse.traverse(ast, {
            enter: (node, parent) => {
                node.parent = parent;  // Add parent reference
                this.enterNode(node, parent);
            },
            leave: (node, parent) => this.leaveNode(node, parent)
        });

        // Post-processing analysis
        this.analyzeRecursionPatterns();
        this.analyzeLoopPatterns();

        return this.getAnalysisData();
    }

    collectFunctionBodies(ast) {
        estraverse.traverse(ast, {
            enter: (node, parent) => {
                node.parent = parent;
                if (this.isFunctionNode(node)) {
                    const name = this.getFunctionName(node);
                    if (name && name !== '<anonymous>') {
                        this.functionBodies.set(name, node);
                    }
                }
            }
        });
    }

    isFunctionNode(node) {
        return node.type === 'FunctionDeclaration' ||
               node.type === 'FunctionExpression' ||
               node.type === 'ArrowFunctionExpression';
    }

    getFunctionName(node) {
        if (node.id && node.id.name) {
            return node.id.name;
        }
        
        // Try to get name from variable assignment
        if (node.parent) {
            if (node.parent.type === 'VariableDeclarator' && node.parent.id) {
                return node.parent.id.name;
            }
            if (node.parent.type === 'AssignmentExpression' && node.parent.left) {
                if (node.parent.left.type === 'Identifier') {
                    return node.parent.left.name;
                }
            }
        }
        
        return '<anonymous>';
    }

    enterNode(node, parent) {
        switch (node.type) {
            case 'FunctionDeclaration':
            case 'FunctionExpression':
            case 'ArrowFunctionExpression':
                this.handleFunction(node);
                break;
                
            case 'ForStatement':
                this.handleForLoop(node);
                break;
                
            case 'ForInStatement':
            case 'ForOfStatement':
                this.handleForInOfLoop(node);
                break;
                
            case 'WhileStatement':
            case 'DoWhileStatement':
                this.handleWhileLoop(node);
                break;
                
            case 'CallExpression':
                this.handleCallExpression(node);
                break;
                
            case 'MemberExpression':
                this.handleMemberExpression(node);
                break;
                
            case 'BinaryExpression':
                this.handleBinaryExpression(node);
                break;
                
            case 'UpdateExpression':
                this.handleUpdateExpression(node);
                break;
                
            case 'AssignmentExpression':
                this.handleAssignmentExpression(node);
                break;
                
            case 'IfStatement':
            case 'ConditionalExpression':
                this.conditionalComplexity++;
                break;
                
            case 'AwaitExpression':
                this.hasAsync = true;
                this.asyncPatterns.push('await');
                break;
                
            case 'YieldExpression':
                this.hasGenerators = true;
                break;
        }
    }

    leaveNode(node, parent) {
        switch (node.type) {
            case 'FunctionDeclaration':
            case 'FunctionExpression':
            case 'ArrowFunctionExpression':
                this.exitFunction(node);
                break;
                
            case 'ForStatement':
            case 'ForInStatement':
            case 'ForOfStatement':
            case 'WhileStatement':
            case 'DoWhileStatement':
                this.exitLoop();
                break;
        }
    }

    handleFunction(node) {
        const funcName = this.getFunctionName(node);
        this.currentFunctions.push(funcName);
        
        // Initialize recursive call count for this function
        if (!this.recursiveCallsPerFunction.has(funcName)) {
            this.recursiveCallsPerFunction.set(funcName, 0);
        }
        
        if (node.async) {
            this.hasAsync = true;
            this.asyncPatterns.push('async_function');
        }
        
        if (node.generator) {
            this.hasGenerators = true;
        }
        
        // Check for recursive pattern structure
        if (node.params && node.params.length > 0) {
            this.analyzeParameterPattern(node);
        }
    }

    analyzeParameterPattern(node) {
        // Look for divide-and-conquer patterns (e.g., left, right, mid parameters)
        const params = node.params.map(p => p.name || '').filter(Boolean);
        
        if (params.length >= 2) {
            const hasRangeParams = params.some(p => /start|begin|left|low/i.test(p)) &&
                                   params.some(p => /end|right|high/i.test(p));
            
            if (hasRangeParams) {
                this.divideAndConquerSignals.push('range_parameters');
            }
        }
    }

    exitFunction(node) {
        const funcName = this.getFunctionName(node);
        this.functionComplexity.set(funcName, {
            loopDepth: this.maxLoopDepth,
            conditionalComplexity: this.conditionalComplexity,
            recursiveCalls: this.recursiveCallsPerFunction.get(funcName) || 0
        });
        this.currentFunctions.pop();
    }

    handleForLoop(node) {
        this.currentLoopDepth++;
        this.maxLoopDepth = Math.max(this.maxLoopDepth, this.currentLoopDepth);
        
        const loopPattern = this.analyzeForLoopPattern(node);
        if (loopPattern) {
            this.loopPatterns.push(loopPattern);
        }
    }

    analyzeForLoopPattern(node) {
        const pattern = {
            type: 'for',
            isLogarithmic: false,
            isLinear: false,
            complexity: 'linear'
        };
        
        // Analyze init, test, and update expressions
        if (node.update) {
            const updateStr = this.nodeToString(node.update);
            
            // Check for logarithmic patterns
            if (node.update.type === 'AssignmentExpression') {
                const { operator, right } = node.update;
                
                // Check for i *= 2, i /= 2 patterns
                if (operator === '*=' || operator === '/=') {
                    if (right && right.type === 'Literal' && 
                        (right.value === 2 || right.value === 0.5)) {
                        pattern.isLogarithmic = true;
                        pattern.complexity = 'logarithmic';
                    }
                }
                
                // Check for i = i * 2, i = i / 2 patterns
                if (operator === '=' && right && right.type === 'BinaryExpression') {
                    const { operator: binOp, right: binRight } = right;
                    if ((binOp === '*' || binOp === '/') && 
                        binRight && binRight.type === 'Literal' &&
                        (binRight.value === 2 || binRight.value === 0.5)) {
                        pattern.isLogarithmic = true;
                        pattern.complexity = 'logarithmic';
                    }
                }
            }
            
            // Check for exponential doubling (i = i * i)
            if (updateStr.includes('*') && updateStr.includes(updateStr.split('*')[0].trim())) {
                const varName = updateStr.split(/[*=]/)[0].trim();
                if (updateStr.split('*').filter(part => part.includes(varName)).length > 1) {
                    pattern.complexity = 'exponential_growth';
                }
            }
        }
        
        // Check test condition for logarithmic patterns
        if (node.test && node.test.type === 'BinaryExpression') {
            const { left, operator, right } = node.test;
            
            // Check for patterns like i < n/2, i <= Math.floor(n/2)
            if (right && right.type === 'BinaryExpression' && 
                (right.operator === '/' || right.operator === '>>')) {
                pattern.isLogarithmic = true;
                pattern.complexity = 'logarithmic';
            }
        }
        
        return pattern;
    }

    handleForInOfLoop(node) {
        this.currentLoopDepth++;
        this.maxLoopDepth = Math.max(this.maxLoopDepth, this.currentLoopDepth);
        
        this.loopPatterns.push({
            type: node.type === 'ForInStatement' ? 'for-in' : 'for-of',
            complexity: 'linear'
        });
    }

    handleWhileLoop(node) {
        this.currentLoopDepth++;
        this.maxLoopDepth = Math.max(this.maxLoopDepth, this.currentLoopDepth);
        
        const pattern = this.analyzeWhileLoopPattern(node);
        if (pattern) {
            this.loopPatterns.push(pattern);
        }
    }

    analyzeWhileLoopPattern(node) {
        const pattern = {
            type: 'while',
            complexity: 'linear'
        };
        
        // Check for binary search pattern in while loop
        if (node.test && node.body) {
            const testStr = this.nodeToString(node.test);
            const bodyStr = this.nodeToString(node.body);
            
            // Binary search indicators
            const hasMidCalculation = bodyStr.includes('>>') || 
                                     (bodyStr.includes('/') && bodyStr.includes('2')) ||
                                     bodyStr.includes('Math.floor');
            
            const hasRangeUpdate = /\w+\s*=\s*\w+\s*[+-]\s*1/.test(bodyStr);
            
            const hasComparison = testStr.includes('<') || testStr.includes('<=');
            
            if (hasMidCalculation && hasRangeUpdate && hasComparison) {
                pattern.complexity = 'logarithmic';
                this.divideAndConquerSignals.push('binary_search_pattern');
            }
        }
        
        return pattern;
    }

    exitLoop() {
        this.currentLoopDepth--;
    }

    handleCallExpression(node) {
        const funcName = this.getCallName(node);
        
        if (funcName) {
            this.functionCalls.push(funcName);
            
            // Check if this is a recursive call
            const currentFunc = this.currentFunctions[this.currentFunctions.length - 1];
            
            if (currentFunc && currentFunc !== '<anonymous>' && currentFunc === funcName) {
                this.hasRecursion = true;
                if (!this.recursiveFunctions.includes(funcName)) {
                    this.recursiveFunctions.push(funcName);
                }
                
                // Count recursive calls for this function
                const count = this.recursiveCallsPerFunction.get(currentFunc) || 0;
                this.recursiveCallsPerFunction.set(currentFunc, count + 1);
                
                // Analyze the recursive call
                this.analyzeRecursiveCall(node, currentFunc);
            }
            
            // Check for Promise patterns
            if (funcName === 'Promise' || funcName.endsWith('.then') || 
                funcName.endsWith('.catch') || funcName.endsWith('.finally')) {
                this.asyncPatterns.push('promises');
            }
            
            // Check for Math operations
            if (funcName.startsWith('Math.')) {
                this.handleMathOperation(funcName);
            }
        }
    }

    analyzeRecursiveCall(node, functionName) {
        const pattern = {
            type: 'recursion',
            functionName: functionName,
            hasMultipleCalls: false,
            hasDivision: false,
            callCount: 0,
            isInLoop: this.currentLoopDepth > 0,
            argumentPattern: null
        };
        
        // Analyze arguments to detect pattern
        if (node.arguments && node.arguments.length > 0) {
            const argAnalysis = this.analyzeRecursiveArguments(node.arguments);
            pattern.hasDivision = argAnalysis.hasDivision;
            pattern.argumentPattern = argAnalysis.pattern;
        }
        
        // Count total recursive calls in this function
        const funcBody = this.functionBodies.get(functionName);
        if (funcBody) {
            const callCount = this.countRecursiveCallsInFunction(funcBody, functionName);
            pattern.callCount = callCount;
            pattern.hasMultipleCalls = callCount >= 2;
        }
        
        this.recursionPatterns.push(pattern);
    }

    analyzeRecursiveArguments(args) {
        const analysis = {
            hasDivision: false,
            pattern: null
        };
        
        for (const arg of args) {
            const argStr = this.nodeToString(arg);
            
            // Check for division patterns (divide-and-conquer)
            if (argStr.includes('/2') || argStr.includes('>>1') || 
                argStr.includes('/ 2') || argStr.includes('>> 1')) {
                analysis.hasDivision = true;
                analysis.pattern = 'divide_by_2';
            }
            // Check for subtraction patterns (linear or exponential)
            else if (argStr.includes('- 1') || argStr.includes('-1')) {
                if (analysis.pattern !== 'divide_by_2') {
                    analysis.pattern = 'decrement';
                }
            }
            else if (argStr.includes('- 2') || argStr.includes('-2')) {
                if (analysis.pattern !== 'divide_by_2') {
                    analysis.pattern = 'double_decrement';
                }
            }
        }
        
        return analysis;
    }

    countRecursiveCallsInFunction(funcNode, funcName) {
        let count = 0;
        
        // Traverse the function body to count recursive calls
        estraverse.traverse(funcNode, {
            enter: (node) => {
                if (node.type === 'CallExpression') {
                    const calledName = this.getCallName(node);
                    if (calledName === funcName) {
                        count++;
                    }
                }
            },
            // Don't traverse into nested functions
            fallback: (node) => {
                if (this.isFunctionNode(node) && node !== funcNode) {
                    return estraverse.VisitorOption.Skip;
                }
            }
        });
        
        return count;
    }

    handleMathOperation(funcName) {
        const operation = funcName.split('.')[1];
        
        if (operation === 'floor' || operation === 'ceil' || operation === 'round') {
            this.divideAndConquerSignals.push('math_division');
        }
        
        if (operation === 'log' || operation === 'log2' || operation === 'log10') {
            this.divideAndConquerSignals.push('logarithmic_operation');
        }
        
        if (operation === 'pow' || operation === 'sqrt') {
            this.binaryOperations.push(operation);
        }
    }

    handleMemberExpression(node) {
        if (node.property && node.property.name) {
            const methodName = node.property.name;
            
            // Array methods
            const arrayMethods = [
                'map', 'filter', 'reduce', 'forEach', 'find', 'findIndex',
                'some', 'every', 'sort', 'includes', 'indexOf', 'lastIndexOf',
                'push', 'pop', 'shift', 'unshift', 'splice', 'slice', 'concat',
                'reverse', 'join', 'flat', 'flatMap'
            ];
            
            if (arrayMethods.includes(methodName)) {
                this.arrayMethods.push(methodName);
            }
        }
    }

    handleBinaryExpression(node) {
        this.binaryOperations.push({
            operator: node.operator,
            context: this.currentLoopDepth > 0 ? 'in_loop' : 'outside_loop'
        });
        
        // Check for division by 2 or multiplication by 2 (logarithmic indicators)
        if ((node.operator === '/' || node.operator === '*') && 
            node.right && node.right.type === 'Literal' && 
            node.right.value === 2) {
            this.divideAndConquerSignals.push('division_by_2');
        }
        
        // Check for bit shift operations (also logarithmic indicators)
        if (node.operator === '>>' || node.operator === '<<') {
            this.divideAndConquerSignals.push('bit_shift');
        }
    }

    handleUpdateExpression(node) {
        if (node.argument && node.argument.name) {
            const varName = node.argument.name;
            const updates = this.variableUpdates.get(varName) || [];
            updates.push({
                operator: node.operator,
                inLoop: this.currentLoopDepth > 0
            });
            this.variableUpdates.set(varName, updates);
        }
    }

    handleAssignmentExpression(node) {
        if (node.left && node.left.name) {
            const varName = node.left.name;
            const updates = this.variableUpdates.get(varName) || [];
            updates.push({
                operator: node.operator,
                inLoop: this.currentLoopDepth > 0,
                rightSide: this.nodeToString(node.right)
            });
            this.variableUpdates.set(varName, updates);
        }
    }

    analyzeRecursionPatterns() {
        if (this.hasRecursion && this.recursionPatterns.length > 0) {
            // Group patterns by function
            const functionPatterns = new Map();
            
            for (const pattern of this.recursionPatterns) {
                const funcName = pattern.functionName;
                if (!functionPatterns.has(funcName)) {
                    functionPatterns.set(funcName, []);
                }
                functionPatterns.get(funcName).push(pattern);
            }
            
            // Analyze each function's recursion pattern
            for (const [funcName, patterns] of functionPatterns) {
                const totalCalls = this.recursiveCallsPerFunction.get(funcName) || 0;
                const hasMultipleCalls = totalCalls >= 2;
                const hasDivision = patterns.some(p => p.hasDivision);
                const hasDecrementPattern = patterns.some(p => 
                    p.argumentPattern === 'decrement' || p.argumentPattern === 'double_decrement'
                );
                
                // Exponential: Multiple recursive calls without division
                if (hasMultipleCalls && !hasDivision) {
                    this.divideAndConquerSignals.push('multiple_recursion_exponential');
                    
                    // Check for classic fibonacci pattern (n-1 and n-2)
                    if (hasDecrementPattern) {
                        this.divideAndConquerSignals.push('fibonacci_pattern');
                    }
                }
                // Divide and conquer: Multiple calls with division
                else if (hasMultipleCalls && hasDivision) {
                    this.divideAndConquerSignals.push('divide_and_conquer_recursion');
                }
                // Linear recursion: Single recursive call
                else if (!hasMultipleCalls) {
                    if (hasDivision) {
                        this.divideAndConquerSignals.push('logarithmic_recursion');
                    } else {
                        this.divideAndConquerSignals.push('linear_recursion');
                    }
                }
            }
        }
    }

    analyzeLoopPatterns() {
        // Analyze nested loops and patterns
        const logarithmicLoops = this.loopPatterns.filter(p => p.complexity === 'logarithmic');
        const linearLoops = this.loopPatterns.filter(p => p.complexity === 'linear');
        
        if (logarithmicLoops.length > 0) {
            this.divideAndConquerSignals.push('logarithmic_loops');
        }
        
        // Check for sorting pattern (nested loops with swapping)
        if (this.maxLoopDepth >= 2 && this.variableUpdates.size > 0) {
            const hasSwapPattern = Array.from(this.variableUpdates.values()).some(updates =>
                updates.filter(u => u.inLoop).length >= 2
            );
            
            if (hasSwapPattern) {
                this.divideAndConquerSignals.push('sorting_pattern');
            }
        }
    }

    getCallName(node) {
        if (node.callee.type === 'Identifier') {
            return node.callee.name;
        } else if (node.callee.type === 'MemberExpression') {
            return this.getMemberName(node.callee);
        }
        return null;
    }

    getMemberName(node) {
        let name = '';
        
        if (node.object) {
            if (node.object.type === 'Identifier') {
                name = node.object.name + '.';
            } else if (node.object.type === 'MemberExpression') {
                name = this.getMemberName(node.object) + '.';
            }
        }
        
        if (node.property && node.property.name) {
            name += node.property.name;
        }
        
        return name;
    }

    nodeToString(node) {
        // More sophisticated node to string conversion
        if (!node) return '';
        
        try {
            return JSON.stringify(node, (key, value) => {
                if (key === 'parent' || key === 'range' || key === 'loc') {
                    return undefined;
                }
                return value;
            });
        } catch (e) {
            return '';
        }
    }

    getAnalysisData() {
        // Process recursive call counts
        const recursiveCallSummary = {};
        for (const [func, count] of this.recursiveCallsPerFunction) {
            if (count > 0) {
                recursiveCallSummary[func] = count;
            }
        }
        
        return {
            maxLoopDepth: this.maxLoopDepth,
            hasRecursion: this.hasRecursion,
            recursiveFunctions: [...new Set(this.recursiveFunctions)],
            recursiveCallCounts: recursiveCallSummary,
            functionCalls: this.functionCalls,
            functionComplexity: Object.fromEntries(this.functionComplexity),
            arrayMethods: this.arrayMethods,
            asyncPatterns: this.asyncPatterns,
            hasGenerators: this.hasGenerators,
            hasAsync: this.hasAsync,
            loopPatterns: this.loopPatterns,
            binaryOperations: this.binaryOperations,
            recursionPatterns: this.recursionPatterns,
            divideAndConquerSignals: this.divideAndConquerSignals,
            conditionalComplexity: this.conditionalComplexity,
            variableUpdates: Object.fromEntries(this.variableUpdates)
        };
    }
}

module.exports = { ASTAnalyzer };