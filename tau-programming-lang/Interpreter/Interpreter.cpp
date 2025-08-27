//
//  Interpreter.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 24/08/2025.
//

#include "Interpreter.h"
#include "../Statements/Statements.hpp"
#include "../Expression/Expression.hpp"
#include "../Visitor/AstPrinter/AstPrinter.h"
#include <memory>

Interpreter::Interpreter() {
    env = new Env();
}

Interpreter::~Interpreter() {
    delete env;
}

void Interpreter::execute(vector<unique_ptr<Statement>> ast) {
    for (unique_ptr<Statement>& stmt : ast) {
        stmt->accept(*this);
    }
}

void Interpreter::executeBlock(unique_ptr<Statement> block) {
    block->accept(*this);
}

R Interpreter::visitExpression(ExpressionStatement* stmt) {
    return stmt->expression->accept(*this);
}

R Interpreter::visitBlock(BlockStatement* stmt) {
    
    Env* previous = env;           // save current scope
    env = new Env(previous);       // allocate child env on heap
    
    for (auto& item : previous->getStack()) {
        env->setValue(item.first, item.second);
    }

    for (auto& s : stmt->body) {
        s->accept(*this);
    }
    
    delete env;                    // cleanup after block
    env = previous;                // restore old scope
    
    return true;
}

R Interpreter::visitVariable(VariableStatement* stmt) {
    
    const string kind = stmt->kind;
    
    if ((stmt->declarations).size() > 0) {
        for (auto& declarator : stmt->declarations) {
            
            if (kind == "const" && declarator.init == nullptr) {
                throw runtime_error("Missing initializer in const declaration: " + declarator.id);
            }
            
            if (declarator.init) {
                
                R value = declarator.init->accept(*this);
                
                env->setValue(declarator.id, value);
            }
        }
    }
    
    return true;
    
}


R Interpreter::visitCall(CallExpression* expr) {
    
    vector<R> vectorArg;
    
    auto callee = expr->callee->accept(*this);
    
    for (auto& arg : expr->arguments) {
        auto _arg = arg->accept(*this);
        vectorArg.push_back(_arg);
    }

    // If the callee is "print"
    if (holds_alternative<std::string>(callee) &&
        get<std::string>(callee) == "print")
    {
        for (auto& v : vectorArg) {
            printValue(v);
            std::cout << " ";
        }
        cout << std::endl;
        return std::monostate{};
    }
    
    // search call name in function declarations.

    vector<Expression*> args = env->getFunctionParams(get<std::string>(callee));
    Statement* body = env->getFunctionBody(get<std::string>(callee));
    
    for (Expression* arg : args) {
        
        string key;
        
        if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(arg)) {
            key = ident->name;
        }
        
        env->setStackValue(key, arg->accept(*this));
        
    }

    // AstPrinter printer;

    if (body != nullptr) {

        body->accept(*this);
        
        return monostate();
    }
    
    throw runtime_error("Unknown function call");
    
}

R Interpreter::visitFalseKeyword(FalseKeyword* expr) {
    return false;
}

R Interpreter::visitTrueKeyword(TrueKeyword* expr) {
    return true;
}

R Interpreter::visitNumericLiteral(NumericLiteral* expr) {
    return expr->text;
}

R Interpreter::visitStringLiteral(StringLiteral* expr) {
    return expr->text;
}

R Interpreter::visitIdentifier(IdentifierExpression* expr) {
    return env->getValue(expr->name);
}

R Interpreter::visitFunction(FunctionDeclaration* stmt) {
    
    auto id = stmt->id;
        
    env->setValue(id, id);
    env->setFunctionDeclarations(id, std::move(stmt->params), std::move(stmt->body));

    return true;
}

R Interpreter::visitIf(IfStatement* stmt) {
    return true;
}

R Interpreter::visitWhile(WhileStatement* stmt) {
    return true;
}

R Interpreter::visitFor(ForStatement* stmt) {
    return true;
}

R Interpreter::visitReturn(ReturnStatement* stmt) {
    return true;
}

R Interpreter::visitBreak(BreakStatement* stmt) {
    return true;
}

R Interpreter::visitContinue(ContinueStatement* stmt) {
    return true;
}
R Interpreter::visitThrow(ThrowStatement* stmt) {    return true;
}
R Interpreter::visitEmpty(EmptyStatement* stmt) {    return true;
}
R Interpreter::visitClass(ClassDeclaration* stmt) {    return true;
}
R Interpreter::visitMethodDefinition(MethodDefinition* stmt) {    return true;
}
R Interpreter::visitDoWhile(DoWhileStatement* stmt) {    return true;
}
R Interpreter::visitSwitchCase(SwitchCase* stmt) {    return true;
}
R Interpreter::visitSwitch(SwitchStatement* stmt) {    return true;
}
R Interpreter::visitCatch(CatchClause* stmt) {    return true;
}
R Interpreter::visitTry(TryStatement* stmt) {    return true;
}
R Interpreter::visitForIn(ForInStatement* stmt) {    return true;
}
R Interpreter::visitForOf(ForOfStatement* stmt) {    return true;
}

// -------- Expressions --------
R Interpreter::visitLiteral(LiteralExpression* expr) { return true; }
R Interpreter::visitBinary(BinaryExpression* expr) { return true; }
R Interpreter::visitUnary(UnaryExpression* expr) { return true; }
R Interpreter::visitUpdate(UpdateExpression* expr) { return true; }
R Interpreter::visitAssignment(AssignmentExpression* expr) { return true; }
R Interpreter::visitLogical(LogicalExpression* expr) { return true; }
R Interpreter::visitConditional(ConditionalExpression* expr) { return true; }
R Interpreter::visitMember(MemberExpression* expr) { return true; }
R Interpreter::visitThis(ThisExpression* expr) { return true; }
R Interpreter::visitNew(NewExpression* expr) { return true; }
R Interpreter::visitArray(ArrayLiteralExpression* expr) { return true; }
R Interpreter::visitObject(ObjectLiteralExpression* expr) { return true; }
R Interpreter::visitSuper(SuperExpression* expr) { return true; }
R Interpreter::visitProperty(PropertyExpression* expr) { return true; }
R Interpreter::visitSequence(SequenceExpression* expr) { return true; }
R Interpreter::visitPublicKeyword(PublicKeyword* expr) { return true; }
R Interpreter::visitPrivateKeyword(PrivateKeyword* expr) { return true; }
R Interpreter::visitProtectedKeyword(ProtectedKeyword* expr) { return true; }
R Interpreter::visitStaticKeyword(StaticKeyword* expr) { return true; }

