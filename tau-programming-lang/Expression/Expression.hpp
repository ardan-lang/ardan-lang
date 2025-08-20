//
//  Expression.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 19/08/2025.
//

#ifndef Expression_hpp
#define Expression_hpp

#include <stdio.h>
#include <cstring>
#include <iostream>

using namespace std;

// Forward declarations of AST nodes
class LiteralExpression;
class BinaryExpression;
class VariableExpression;
class GroupingExpression;
class UnaryExpression;

class ExpressionVisitor {
public:
    virtual ~ExpressionVisitor() = default;

    virtual void visitLiteral(LiteralExpression* expr) = 0;
    virtual void visitBinary(BinaryExpression* expr) = 0;
    virtual void visitVariable(VariableExpression* expr) = 0;
    virtual void visitGrouping(GroupingExpression* expr) = 0;
    virtual void visitUnary(UnaryExpression* expr) = 0;
};

struct Expression {
    virtual ~Expression() = default;
    virtual void accept(ExpressionVisitor& visitor) = 0;
};

class LiteralExpression : public Expression {
public:
    LiteralExpression(const string token) : token(token) {};
    const string token;
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitLiteral(this);
    }
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(const string op, unique_ptr<Expression> left, unique_ptr<Expression> right) : op(op), left(std::move(left)), right(std::move(right)) {};
    const string op;
    unique_ptr<Expression> left;
    unique_ptr<Expression> right;
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitBinary(this);
    }
    
};

class VariableExpression : public Expression {
public:
    const string name;
    VariableExpression(const string name) : name(name) {};
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitVariable(this);
    }

};

class GroupingExpression : public Expression {
public:
    GroupingExpression(unique_ptr<Expression> expression) : expression(std::move(expression)) {};
    unique_ptr<Expression> expression;
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitGrouping(this);
    }

};

class UnaryExpression : public Expression {
public:
    UnaryExpression(const string op, unique_ptr<Expression> expression) : op(op), expression(std::move(expression)) {};
    const string op;
    unique_ptr<Expression> expression;
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitUnary(this);
    }
};

#endif /* Expression_hpp */
