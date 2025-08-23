//
//  AstPrinter.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 20/08/2025.
//

#ifndef AstPrinter_h
#define AstPrinter_h

#include <iostream>
#include <string>
#include "../../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../../Expression/Expression.hpp"
#include "../../Statements/Statements.hpp"

using namespace std;

class AstPrinter : public ExpressionVisitor, public StatementVisitor {
    int indent = 0;
    
    void printIndent() {
        for (int i = 0; i < indent; i++) std::cout << "  ";
    }
    
public:
    // -------- Statements --------
    void visitExpression(ExpressionStatement* stmt) override {
        printIndent(); std::cout << "ExpressionStatement:\n";
        indent++;
        stmt->expression->accept(*this);
        indent--;
    }
    
    void visitBlock(BlockStatement* stmt) override {
        printIndent(); std::cout << "Block:\n";
        indent++;
        for (auto& s : stmt->body) {
            s->accept(*this);
        }
        indent--;
    }
    
    void visitVariable(VariableStatement* stmt) override {
        printIndent(); std::cout << "VariableDeclaration " << stmt->kind;
        std::cout << " =\n";
        indent++;
        for (auto& declarator : stmt->declarations) {
            std::cout << declarator.id << "\n";
            declarator.init->accept(*this);
        }
        indent--;
        //        if (stmt->init) {
        //            std::cout << " =\n";
        //            indent++;
        //            stmt->init->accept(*this);
        //            indent--;
        //        } else {
        //            std::cout << "\n";
        //        }
    }
    
    void visitFunction(FunctionDeclaration* stmt) override {
        printIndent(); std::cout << "Function " << stmt->id << "(";
        for (size_t i = 0; i < stmt->params.size(); i++) {
            std::cout << stmt->params[i];
            if (i < stmt->params.size() - 1) std::cout << ", ";
        }
        std::cout << ")\n";
        indent++;
        stmt->body->accept(*this);
        indent--;
    }
    
    void visitIf(IfStatement* stmt) override {
        printIndent(); std::cout << "If\n";
        indent++;
        stmt->test->accept(*this);
        printIndent(); std::cout << "Then:\n";
        stmt->consequent->accept(*this);
        if (stmt->alternate) {
            printIndent(); std::cout << "Else:\n";
            stmt->alternate->accept(*this);
        }
        indent--;
    }
    
    void visitWhile(WhileStatement* stmt) override {
        printIndent(); std::cout << "While\n";
        indent++;
        stmt->test->accept(*this);
        stmt->body->accept(*this);
        indent--;
    }
    
    void visitFor(ForStatement* stmt) override {
        printIndent(); std::cout << "For\n";
        indent++;
        if (stmt->init) stmt->init->accept(*this);
        if (stmt->test) stmt->test->accept(*this);
        if (stmt->update) stmt->update->accept(*this);
        stmt->body->accept(*this);
        indent--;
    }
    
    void visitReturn(ReturnStatement* stmt) override {
        printIndent(); std::cout << "Return\n";
        if (stmt->argument) {
            indent++;
            stmt->argument->accept(*this);
            indent--;
        }
    }
    
    void visitBreak(BreakStatement* stmt) override {
        printIndent(); std::cout << "Break\n";
    }
    
    void visitContinue(ContinueStatement* stmt) override {
        printIndent(); std::cout << "Continue\n";
    }
    
    void visitThrow(ThrowStatement* stmt) override {
        printIndent(); std::cout << "Throw\n";
        indent++;
        stmt->argument->accept(*this);
        indent--;
    }
    
    void visitTryCatch(TryCatchStatement* stmt) override {
        //        printIndent(); std::cout << "Try\n";
        //        indent++;
        //        stmt->block->accept(*this);
        //        indent--;
        //        printIndent(); std::cout << "Catch(" << stmt->param << ")\n";
        //        indent++;
        //        stmt->handler->accept(*this);
        //        indent--;
        //        if (stmt->finalizer) {
        //            printIndent(); std::cout << "Finally\n";
        //            indent++;
        //            stmt->finalizer->accept(*this);
        //            indent--;
        //        }
    }
    
    // -------- Expressions --------
    void visitLiteral(LiteralExpression* expr) override {
        printIndent(); std::cout << "Literal(" << expr->token.lexeme << ")\n";
    }
    
    void visitIdentifier(IdentifierExpression* expr) override {
        printIndent(); std::cout << "Identifier(" << expr->name << expr->previous.lexeme << ")\n";
    }
    
    void visitBinary(BinaryExpression* expr) override {
        printIndent(); std::cout << "Binary(" << expr->op.lexeme << ")\n";
        indent++;
        expr->left->accept(*this);
        expr->right->accept(*this);
        indent--;
    }
    
    void visitUnary(UnaryExpression* expr) override {
        printIndent(); std::cout << "Unary(" << expr->op.lexeme << ")\n";
        indent++;
        // expr->argument->accept(*this);
        expr->right->accept(*this);
        indent--;
    }
    
    void visitUpdate(UpdateExpression* expr) override {
        printIndent();
        std::cout << "Update(" << expr->op.lexeme
        << (expr->prefix ? " prefix" : " postfix") << ")\n";
        indent++;
        expr->argument->accept(*this);
        indent--;
    }
    
    void visitAssignment(AssignmentExpression* expr) override {
        printIndent(); std::cout << "Assignment(" << expr->op.lexeme << ")\n";
        indent++;
        expr->left->accept(*this);
        expr->right->accept(*this);
        indent--;
    }
    
    void visitLogical(LogicalExpression* expr) override {
        printIndent(); std::cout << "Logical(" << expr->op.lexeme << ")\n";
        indent++;
        expr->left->accept(*this);
        expr->right->accept(*this);
        indent--;
    }
    
    void visitConditional(ConditionalExpression* expr) override {
        printIndent(); std::cout << "Conditional\n";
        indent++;
        expr->test->accept(*this);
        expr->consequent->accept(*this);
        expr->alternate->accept(*this);
        indent--;
    }
    
    void visitCall(CallExpression* expr) override {
        printIndent(); std::cout << "Call\n";
        indent++;
        expr -> callee -> accept(*this);
        // expr->expression->accept(*this);
        for (auto& arg : expr->arguments) {
            arg->accept(*this);
        }
        indent--;
    }
    
    void visitMember(MemberExpression* expr) override {
        printIndent(); std::cout << "Member(" << (expr->computed ? "computed" : "dot") << ")\n";
        indent++;
        expr->object->accept(*this);
        
        if (expr->computed) {
            expr->property->accept(*this);
        } else {
            printIndent(); cout << expr->name.lexeme << "\n";
        }
        indent--;
    }
    
    void visitThis(ThisExpression* expr) override {
        printIndent(); std::cout << "This\n";
    }
    
    void visitNew(NewExpression* expr) override {
        printIndent(); std::cout << "New\n";
        indent++;
        expr->callee->accept(*this);
        for (auto& arg : expr->arguments) arg->accept(*this);
        indent--;
    }
    
    void visitArray(ArrayLiteralExpression* expr) override {
        printIndent(); std::cout << "Array\n";
        indent++;
        for (auto& e : expr->elements) e->accept(*this);
        indent--;
    }
    
    void visitObject(ObjectLiteralExpression* expr) override {
        printIndent(); std::cout << "Object\n";
        indent++;
        for (auto& prop : expr->properties) {
            //            printIndent(); std::cout << "Property " << prop.key << ":\n";
            //            indent++;
            //            prop.value->accept(*this);
            indent--;
        }
        indent--;
    }
    
    void visitSuper(SuperExpression* expr) override {
        printIndent(); std::cout << "Super:\n";
        
    }
    
    void visitProperty(PropertyExpression* expr) override {
        printIndent(); std::cout << "Property:\n";
        
    }
    
    void visitSequence(SequenceExpression* expr) override {
        printIndent(); std::cout << "Sequence:\n";
        indent++;
        for(auto& expr : expr -> expressions) {
            expr->accept(*this);
        }
        indent--;
    }
    
    void visitEmpty(EmptyStatement* stmt) override {
        printIndent(); std::cout << "Empty\n";
    }
    
    void visitFalseKeyword(FalseKeyword* expr) override {
        printIndent(); std::cout << "False\n";
    }
    
    void visitTrueKeyword(TrueKeyword* expr) override {
        printIndent(); std::cout << "True\n";
    }
    
    void visitNumericLiteral(NumericLiteral* expr) override {
        printIndent(); std::cout << "Numeric ";
        indent++;
        cout << expr -> text << "\n";
        indent--;
    }
    
    void visitStringLiteral(StringLiteral* expr) override {
        printIndent(); std::cout << "String ";
        indent++;
        cout << expr -> text << "\n";
        indent--;
    }
    
    void visitClass(ClassDeclaration* stmt) override {
        printIndent(); std::cout << "Class " << stmt->id << " {\n";
        indent++;
        for (auto& field : stmt->fields) {
            field->accept(*this);
        }
        for (auto& method : stmt->body) {
            method->accept(*this);
        }
        indent--;
        printIndent(); std::cout << "}\n";
    }
    
    void visitMethodDefinition(MethodDefinition* stmt) override {
        printIndent(); std::cout << "Method " << stmt->name << "(";
        for (size_t i = 0; i < stmt->params.size(); i++) {
            std::cout << stmt->params[i];
            if (i < stmt->params.size() - 1) std::cout << ", ";
        }
        std::cout << ") ";
        stmt->methodBody->accept(*this);
    }
    
    void visitDoWhile(DoWhileStatement* stmt) override {
        cout << "DoWhileStatement {\n";
        cout << "  body: ";
        if (stmt->body) stmt->body->accept(*this);
        cout << "\n  condition: ";
        if (stmt->condition) stmt->condition->accept(*this);
        cout << "\n}\n";
    }
    
    void visitSwitchCase(SwitchCase* stmt) override {
        cout << "SwitchCase {\n";
        cout << "  test: ";
        if (stmt->test) stmt->test->accept(*this);
        else cout << "default"; // if `test` is null → default case
        cout << "\n  consequent: [\n";
        for (auto& cons : stmt->consequent) {
            if (cons) {
                cout << "    ";
                cons->accept(*this);
                cout << "\n";
            }
        }
        cout << "  ]\n";
        cout << "}\n";
    }
    
    void visitSwitch(SwitchStatement* stmt) override {
        cout << "SwitchStatement {\n";
        cout << "  discriminant: ";
        if (stmt->discriminant) stmt->discriminant->accept(*this);
        cout << "\n  cases: [\n";
        for (auto& scase : stmt->cases) {
            if (scase) {
                cout << "    ";
                scase->accept(*this);
            }
        }
        cout << "  ]\n";
        cout << "}\n";
    }
    
    void visitCatch(CatchClause* stmt) override {
        printIndent();
        cout << "catch (" << stmt->param << ") ";
        if (stmt->body) {
            cout << "{\n";
            indent++;
            stmt->body->accept(*this);
            indent--;
            printIndent();
            cout << "}\n";
        } else {
            cout << "{}\n";
        }
    }

    void visitTry(TryStatement* stmt) override {
        printIndent();
        cout << "try ";
        if (stmt->block) {
            cout << "{\n";
            indent++;
            stmt->block->accept(*this);
            indent--;
            printIndent();
            cout << "}";
        }
        cout << "\n";

        if (stmt->handler) {
            stmt->handler->accept(*this);
        }

        if (stmt->finalizer) {
            printIndent();
            cout << "finally ";
            cout << "{\n";
            indent++;
            stmt->finalizer->accept(*this);
            indent--;
            printIndent();
            cout << "}\n";
        }
    }
    
};

#endif /* AstPrinter_h */
