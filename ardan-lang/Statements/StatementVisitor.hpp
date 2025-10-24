//
//  StatementVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 18/08/2025.
//

#ifndef StatementVisitor_hpp
#define StatementVisitor_hpp

#include <stdio.h>
#include "../Interpreter/R.hpp"

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
class ImportDeclaration;
class EnumDeclaration;
class InterfaceDeclaration;

class StatementVisitor {
public:
    virtual ~StatementVisitor() = default;

    virtual R visitExpression(ExpressionStatement* stmt) = 0;
    virtual R visitEmpty(EmptyStatement* stmt) = 0;
    virtual R visitBlock(BlockStatement* stmt) = 0;
    virtual R visitIf(IfStatement* stmt) = 0;
    virtual R visitWhile(WhileStatement* stmt) = 0;
    virtual R visitFor(ForStatement* stmt) = 0;
    virtual R visitVariable(VariableStatement* stmt) = 0;
    virtual R visitFunction(FunctionDeclaration* stmt) = 0;
    virtual R visitReturn(ReturnStatement* stmt) = 0;
    virtual R visitBreak(BreakStatement* stmt) = 0;
    virtual R visitContinue(ContinueStatement* stmt) = 0;
    virtual R visitThrow(ThrowStatement* stmt) = 0;
    virtual R visitClass(ClassDeclaration* stmt) = 0;
    virtual R visitMethodDefinition(MethodDefinition* stmt) = 0;
    virtual R visitDoWhile(DoWhileStatement* stmt) = 0;

    virtual R visitSwitchCase(SwitchCase* stmt) = 0;
    virtual R visitSwitch(SwitchStatement* stmt) = 0;
    virtual R visitTry(TryStatement* stmt) = 0;
    virtual R visitCatch(CatchClause* stmt) = 0;
    virtual R visitForIn(ForInStatement* stmt) = 0;
    virtual R visitForOf(ForOfStatement* stmt) = 0;
    virtual R visitImportDeclaration(ImportDeclaration* stmt) = 0;
    virtual R visitEnumDeclaration(EnumDeclaration* stmt) = 0;
    virtual R visitInterfaceDeclaration(InterfaceDeclaration* stmt) = 0;

};

#endif /* StatementVisitor_hpp */
