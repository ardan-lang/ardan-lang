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
#include "../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../Statements/StatementVisitor.hpp"
#include "R.hpp"
#include "Env.h"

using namespace std;

class Interpreter : public ExpressionVisitor, public StatementVisitor {

private:
    Env env;
    
public:
    Interpreter();
    void execute(vector<unique_ptr<Statement>> ast);
    void executeBlock(unique_ptr<Statement> block);

    R visitExpression(ExpressionStatement* stmt) override;
    R visitBlock(BlockStatement* stmt) override;
    R visitVariable(VariableStatement* stmt) override;
    R visitFunction(FunctionDeclaration* stmt) override;
    R visitIf(IfStatement* stmt) override;
    R visitWhile(WhileStatement* stmt) override;
    R visitFor(ForStatement* stmt) override;
    R visitReturn(ReturnStatement* stmt) override;
    R visitBreak(BreakStatement* stmt) override;
    R visitContinue(ContinueStatement* stmt) override;
    R visitThrow(ThrowStatement* stmt) override;
    R visitEmpty(EmptyStatement* stmt) override;
    R visitClass(ClassDeclaration* stmt) override;
    R visitMethodDefinition(MethodDefinition* stmt) override;
    R visitDoWhile(DoWhileStatement* stmt) override;
    R visitSwitchCase(SwitchCase* stmt) override;
    R visitSwitch(SwitchStatement* stmt) override;
    R visitCatch(CatchClause* stmt) override;
    R visitTry(TryStatement* stmt) override;
    R visitForIn(ForInStatement* stmt) override;
    R visitForOf(ForOfStatement* stmt) override;
    
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
        
    inline void printValue(const R& value) {
        if (std::holds_alternative<std::monostate>(value)) {
            std::cout << "nil";
        } else if (std::holds_alternative<double>(value)) {
            std::cout << std::get<double>(value);
        } else if (std::holds_alternative<std::string>(value)) {
            std::cout << std::get<std::string>(value);
        } else if (std::holds_alternative<bool>(value)) {
            std::cout << (std::get<bool>(value) ? "true" : "false");
        }
    }
    
};

#endif /* Interpreter_hpp */
