#include "ast_visitor.hpp"
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <algorithm>

ASTVisitor::ASTVisitor() {}

void ASTVisitor::TraverseAST(clang::ASTContext& context) {
    TraverseDecl(context.getTranslationUnitDecl());
}

bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl* func) {
    if (!func->hasBody()) return true;
    
    std::string funcName = func->getNameAsString();
    data.currentFunctions.insert(funcName);
    
    // Process function body
    TraverseStmt(func->getBody());
    
    // Store function complexity
    data.functionComplexity[funcName] = data.maxLoopDepth;
    data.currentFunctions.erase(funcName);
    
    return true;
}

bool ASTVisitor::VisitForStmt(clang::ForStmt* forStmt) {
    data.currentLoopDepth++;
    data.maxLoopDepth = std::max(data.maxLoopDepth, data.currentLoopDepth);
    
    // Check for logarithmic patterns
    checkForLogarithmicPattern(forStmt);
    
    TraverseStmt(forStmt->getBody());
    data.currentLoopDepth--;
    
    return true;
}

bool ASTVisitor::VisitWhileStmt(clang::WhileStmt* whileStmt) {
    data.currentLoopDepth++;
    data.maxLoopDepth = std::max(data.maxLoopDepth, data.currentLoopDepth);
    
    TraverseStmt(whileStmt->getBody());
    data.currentLoopDepth--;
    
    return true;
}

bool ASTVisitor::VisitDoStmt(clang::DoStmt* doStmt) {
    data.currentLoopDepth++;
    data.maxLoopDepth = std::max(data.maxLoopDepth, data.currentLoopDepth);
    
    TraverseStmt(doStmt->getBody());
    data.currentLoopDepth--;
    
    return true;
}

bool ASTVisitor::VisitCXXForRangeStmt(clang::CXXForRangeStmt* rangeFor) {
    data.currentLoopDepth++;
    data.maxLoopDepth = std::max(data.maxLoopDepth, data.currentLoopDepth);
    
    TraverseStmt(rangeFor->getBody());
    data.currentLoopDepth--;
    
    return true;
}

bool ASTVisitor::VisitCallExpr(clang::CallExpr* call) {
    if (auto* funcDecl = call->getDirectCallee()) {
        std::string funcName = funcDecl->getNameAsString();
        data.functionCalls.push_back(funcName);
        
        // Check for recursive calls
        if (data.currentFunctions.count(funcName) > 0) {
            data.hasRecursion = true;
            data.recursiveFunctions.push_back(funcName);
        }
        
        // Check STL usage
        checkSTLUsage(funcName);
    }
    
    return true;
}

bool ASTVisitor::VisitCXXNewExpr(clang::CXXNewExpr* newExpr) {
    data.hasDynamicMemory = true;
    return true;
}

bool ASTVisitor::VisitCXXDeleteExpr(clang::CXXDeleteExpr* deleteExpr) {
    data.hasDynamicMemory = true;
    return true;
}

bool ASTVisitor::VisitClassTemplateDecl(clang::ClassTemplateDecl* templateDecl) {
    data.hasTemplates = true;
    return true;
}

bool ASTVisitor::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl* templateDecl) {
    data.hasTemplates = true;
    return true;
}

void ASTVisitor::checkForLogarithmicPattern(clang::Stmt* stmt) {
    // Check for patterns like i *= 2, i /= 2, etc.
    // This is simplified; real implementation would analyze the AST more deeply
}

void ASTVisitor::checkSTLUsage(const std::string& name) {
    // STL algorithms
    std::vector<std::string> stlAlgos = {
        "sort", "stable_sort", "partial_sort", "nth_element",
        "binary_search", "lower_bound", "upper_bound",
        "find", "find_if", "accumulate", "transform"
    };
    
    if (std::find(stlAlgos.begin(), stlAlgos.end(), name) != stlAlgos.end()) {
        data.stlAlgorithms.push_back(name);
    }
    
    // STL containers (simplified check)
    std::vector<std::string> containers = {
        "vector", "list", "deque", "set", "map", 
        "unordered_set", "unordered_map", "priority_queue"
    };
    
    for (const auto& container : containers) {
        if (name.find(container) != std::string::npos) {
            data.stlContainers.push_back(container);
            break;
        }
    }
}