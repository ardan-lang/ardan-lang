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
class ClassDeclaration;
class MethodDefinition;
class DoWhileStatement;
class SwitchStatement;
class SwitchCase;
class TryStatement;
class CatchClause;
class ForInStatement;
class ForOfStatement;

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
    virtual void visitClass(ClassDeclaration* stmt) = 0;
    virtual void visitMethodDefinition(MethodDefinition* stmt) = 0;
    virtual void visitDoWhile(DoWhileStatement* stmt) = 0;

    virtual void visitSwitchCase(SwitchCase* stmt) = 0;
    virtual void visitSwitch(SwitchStatement* stmt) = 0;
    virtual void visitTry(TryStatement* stmt) = 0;
    virtual void visitCatch(CatchClause* stmt) = 0;
    virtual void visitForIn(ForInStatement* stmt) = 0;
    virtual void visitForOf(ForOfStatement* stmt) = 0;

};

#endif /* StatementVisitor_hpp */
