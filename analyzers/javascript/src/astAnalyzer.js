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
        this.currentFunctions = new Set();
        this.functionComplexity = new Map();
        this.arrayMethods = [];
        this.asyncPatterns = [];
        this.hasRecursion = false;
        this.hasGenerators = false;
        this.hasAsync = false;
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

        // Traverse AST
        estraverse.traverse(ast, {
            enter: (node, parent) => this.enterNode(node, parent),
            leave: (node, parent) => this.leaveNode(node, parent)
        });

        return this.getAnalysisData();
    }

    enterNode(node, parent) {
        switch (node.type) {
            case 'FunctionDeclaration':
            case 'FunctionExpression':
            case 'ArrowFunctionExpression':
                this.handleFunction(node);
                break;
                
            case 'ForStatement':
            case 'ForInStatement':
            case 'ForOfStatement':
                this.handleForLoop(node);
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
        const funcName = node.id ? node.id.name : '<anonymous>';
        this.currentFunctions.add(funcName);
        
        if (node.async) {
            this.hasAsync = true;
            this.asyncPatterns.push('async_function');
        }
        
        if (node.generator) {
            this.hasGenerators = true;
        }
    }

    exitFunction(node) {
        const funcName = node.id ? node.id.name : '<anonymous>';
        this.functionComplexity.set(funcName, this.maxLoopDepth);
        this.currentFunctions.delete(funcName);
    }

    handleForLoop(node) {
        this.currentLoopDepth++;
        this.maxLoopDepth = Math.max(this.maxLoopDepth, this.currentLoopDepth);
        
        // Check for logarithmic patterns
        if (node.type === 'ForStatement' && node.update) {
            const updateStr = this.nodeToString(node.update);
            if (updateStr.includes('*=') || updateStr.includes('/=')) {
                this.functionComplexity.set('logarithmic_pattern', true);
            }
        }
    }

    handleWhileLoop(node) {
        this.currentLoopDepth++;
        this.maxLoopDepth = Math.max(this.maxLoopDepth, this.currentLoopDepth);
    }

    exitLoop() {
        this.currentLoopDepth--;
    }

    handleCallExpression(node) {
        const funcName = this.getCallName(node);
        
        if (funcName) {
            this.functionCalls.push(funcName);
            
            // Check for recursive calls
            if (this.currentFunctions.has(funcName)) {
                this.hasRecursion = true;
                this.recursiveFunctions.push(funcName);
            }
            
            // Check for Promise patterns
            if (funcName === 'Promise' || funcName.endsWith('.then') || 
                funcName.endsWith('.catch') || funcName.endsWith('.finally')) {
                this.asyncPatterns.push('promises');
            }
        }
    }

    handleMemberExpression(node) {
        if (node.property && node.property.name) {
            const methodName = node.property.name;
            
            // Array methods
            const arrayMethods = [
                'map', 'filter', 'reduce', 'forEach', 'find', 'findIndex',
                'some', 'every', 'sort', 'includes', 'indexOf', 'lastIndexOf',
                'push', 'pop', 'shift', 'unshift', 'splice', 'slice', 'concat'
            ];
            
            if (arrayMethods.includes(methodName)) {
                this.arrayMethods.push(methodName);
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
        if (node.property && node.property.name) {
            return node.property.name;
        }
        return null;
    }

    nodeToString(node) {
        // Simple conversion for analysis
        return JSON.stringify(node);
    }

    getAnalysisData() {
        return {
            maxLoopDepth: this.maxLoopDepth,
            hasRecursion: this.hasRecursion,
            recursiveFunctions: [...new Set(this.recursiveFunctions)],
            functionCalls: this.functionCalls,
            functionComplexity: Object.fromEntries(this.functionComplexity),
            arrayMethods: this.arrayMethods,
            asyncPatterns: this.asyncPatterns,
            hasGenerators: this.hasGenerators,
            hasAsync: this.hasAsync
        };
    }
}

module.exports = { ASTAnalyzer };