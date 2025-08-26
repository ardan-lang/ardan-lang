//
//  Interpreter.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 24/08/2025.
//

#ifndef Interpreter_hpp
#define Interpreter_hpp

#include <stdio.h>
#include <iostream>
#include <string>
//#include "../ExpressionVisitor/ExpressionVisitor.hpp"
//#include "../Statements/StatementVisitor.hpp"

using namespace std;
using R = std::variant<std::monostate, double, std::string, bool>;

class Interpreter : public ExpressionVisitor<R>, public StatementVisitor<void> {
    
public:
    
    void visitExpression(ExpressionStatement* stmt) override;
    void visitBlock(BlockStatement* stmt) override;
    void visitVariable(VariableStatement* stmt) override;
    void visitFunction(FunctionDeclaration* stmt) override;
    void visitIf(IfStatement* stmt) override;
    void visitWhile(WhileStatement* stmt) override;
    void visitFor(ForStatement* stmt) override;
    void visitReturn(ReturnStatement* stmt) override;
    void visitBreak(BreakStatement* stmt) override;
    void visitContinue(ContinueStatement* stmt) override;
    void visitThrow(ThrowStatement* stmt) override;
    void visitEmpty(EmptyStatement* stmt) override;
    void visitClass(ClassDeclaration* stmt) override;
    void visitMethodDefinition(MethodDefinition* stmt) override;
    void visitDoWhile(DoWhileStatement* stmt) override;
    void visitSwitchCase(SwitchCase* stmt) override;
    void visitSwitch(SwitchStatement* stmt) override;
    void visitCatch(CatchClause* stmt) override;
    void visitTry(TryStatement* stmt) override;
    void visitForIn(ForInStatement* stmt) override;
    void visitForOf(ForOfStatement* stmt) override;
    
    // -------- Expressions --------
    R visitLiteral(LiteralExpression* expr) override;
    R visitIdentifier(IdentifierExpression* expr) override;
    R visitBinary(BinaryExpression* expr) override;
    R visitUnary(UnaryExpression* expr) override;
    R visitUpdate(UpdateExpression* expr) override;
    R visitAssignment(AssignmentExpression* expr) override;
    R visitLogical(LogicalExpression* expr) override;
    R visitConditional(ConditionalExpression* expr) override;
    R visitCall(CallExpression* expr) override;
    R visitMember(MemberExpression* expr) override;
    R visitThis(ThisExpression* expr) override;
    R visitNew(NewExpression* expr) override;
    R visitArray(ArrayLiteralExpression* expr) override;
    R visitObject(ObjectLiteralExpression* expr) override;
    R visitSuper(SuperExpression* expr) override;
    R visitProperty(PropertyExpression* expr) override;
    R visitSequence(SequenceExpression* expr) override;
    R visitFalseKeyword(FalseKeyword* expr) override;
    R visitTrueKeyword(TrueKeyword* expr) override;
    R visitNumericLiteral(NumericLiteral* expr) override;
    R visitStringLiteral(StringLiteral* expr) override;
    R visitPublicKeyword(PublicKeyword* expr) override;
    R visitPrivateKeyword(PrivateKeyword* expr) override;
    R visitProtectedKeyword(ProtectedKeyword* expr) override;
    R visitStaticKeyword(StaticKeyword* expr) override;
    
};

#endif /* Interpreter_hpp */
