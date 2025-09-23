#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <vector>
#include <string>
#include <set>
#include <map>

struct AnalysisData {
    int maxLoopDepth = 0;
    int currentLoopDepth = 0;
    bool hasRecursion = false;
    std::vector<std::string> recursiveFunctions;
    std::vector<std::string> functionCalls;
    std::set<std::string> currentFunctions;
    std::map<std::string, int> functionComplexity;
    std::vector<std::string> stlContainers;
    std::vector<std::string> stlAlgorithms;
    bool hasTemplates = false;
    bool hasPointers = false;
    bool hasDynamicMemory = false;
};

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    ASTVisitor();
    
    bool VisitFunctionDecl(clang::FunctionDecl* func);
    bool VisitForStmt(clang::ForStmt* forStmt);
    bool VisitWhileStmt(clang::WhileStmt* whileStmt);
    bool VisitDoStmt(clang::DoStmt* doStmt);
    bool VisitCXXForRangeStmt(clang::CXXForRangeStmt* rangeFor);
    bool VisitCallExpr(clang::CallExpr* call);
    bool VisitCXXNewExpr(clang::CXXNewExpr* newExpr);
    bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* deleteExpr);
    bool VisitClassTemplateDecl(clang::ClassTemplateDecl* templateDecl);
    bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl* templateDecl);
    
    void TraverseAST(clang::ASTContext& context);
    const AnalysisData& getData() const { return data; }
    
private:
    AnalysisData data;
    void checkForLogarithmicPattern(clang::Stmt* stmt);
    void checkSTLUsage(const std::string& name);
};

#endif // AST_VISITOR_HPP