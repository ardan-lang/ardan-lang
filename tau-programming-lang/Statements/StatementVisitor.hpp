//
//  StatementVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 18/08/2025.
//

#ifndef StatementVisitor_hpp
#define StatementVisitor_hpp

#include <stdio.h>

class EmptyStatement;
class BlockStatement;
class ExpressionStatement;
class IfStatement;
class WhileStatement;
class ForStatement;
class VariableStatement;
class FunctionDeclaration;
class ReturnStatement;
class BreakStatement;
class ContinueStatement;
class ThrowStatement;
class TryCatchStatement;

class StatementVisitor {
public:
    virtual ~StatementVisitor() = default;
    
    virtual void visitEmpty(EmptyStatement* stmt) = 0;
    virtual void visitBlock(BlockStatement* stmt) = 0;
    virtual void visitExpression(ExpressionStatement* stmt) = 0;
    virtual void visitIf(IfStatement* stmt) = 0;
    virtual void visitWhile(WhileStatement* stmt) = 0;
    virtual void visitFor(ForStatement* stmt) = 0;
    virtual void visitVariable(VariableStatement* stmt) = 0;
    virtual void visitFunction(FunctionDeclaration* stmt) = 0;
    virtual void visitReturn(ReturnStatement* stmt) = 0;
    virtual void visitBreak(BreakStatement* stmt) = 0;
    virtual void visitContinue(ContinueStatement* stmt) = 0;
    virtual void visitThrow(ThrowStatement* stmt) = 0;
    virtual void visitTryCatch(TryCatchStatement* stmt) = 0;
};

#endif /* StatementVisitor_hpp */
