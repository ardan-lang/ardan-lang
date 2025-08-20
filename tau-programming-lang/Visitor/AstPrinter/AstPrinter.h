//
//  AstPrinter.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 20/08/2025.
//

#ifndef AstPrinter_h
#define AstPrinter_h

#include "../../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../../Expression/Expression.hpp"

using namespace std;

class AstPrinter : public ExpressionVisitor {
public:
    void print(Expression* expr) {
        expr->accept(*this);
    }

    void visitBinary(BinaryExpression* expr) override {
        cout << "(" + expr->op + " " ;
        expr->left->accept(*this) ;
        cout << " " ;
        expr->right->accept(*this) ;
        cout << ")" << endl;
    }

    void visitUnary(UnaryExpression* expr) override {
        cout << "(" ;
        cout << expr->op;
        cout << " " ;
        expr->right->accept(*this);
        cout << ")" << endl;
    }

    void visitLiteral(LiteralExpression* expr) override {
        cout << expr->value << endl;
    }

    void visitGrouping(GroupingExpression* expr) override {
        cout << "(group ";
        expr->expression->accept(*this);
        cout << ")" << endl;
    }
    
    void visitVariable(VariableExpression* expr) override {
        cout << expr->name << endl;
    }
};

#endif /* AstPrinter_h */
