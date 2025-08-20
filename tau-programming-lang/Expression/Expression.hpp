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
#include "../ExpressionVisitor/ExpressionVisitor.hpp"

using namespace std;

struct Expression {
    virtual ~Expression() = default;
    virtual void accept(ExpressionVisitor& visitor) = 0;
};

class LiteralExpression : public Expression {
public:
    LiteralExpression(const string value) : value(value) {};
    const string value;
    
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
        return visitor.visitVariable(this);
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
    UnaryExpression(const string op, unique_ptr<Expression> right) : op(op), right(std::move(right)) {};
    const string op;
    unique_ptr<Expression> right;
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitUnary(this);
    }
};

#endif /* Expression_hpp */
