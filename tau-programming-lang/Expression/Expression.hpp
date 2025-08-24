//
//  Expression.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 19/08/2025.
//

#ifndef Expression_hpp
#define Expression_hpp

#include <memory>
#include <string>
#include <vector>
#include "../Scanner/Token/Token.hpp"
#include "../Scanner/Token/TokenType.h"
#include "../ExpressionVisitor/ExpressionVisitor.hpp"

using std::string;
using std::unique_ptr;
using std::vector;

// Base Expression
class Expression {
public:
    virtual ~Expression() = default;
    virtual void accept(ExpressionVisitor& visitor) = 0;
};

// Literal
class LiteralExpression : public Expression {
public:
    Token token;
    explicit LiteralExpression(Token token) : token(token) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitLiteral(this); }
};

// Identifier
class IdentifierExpression : public Expression {
public:
    string name;
    Token previous;
    
    explicit IdentifierExpression(const string& name) : name(name) {}
    explicit IdentifierExpression(Token previous) : previous(previous) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitIdentifier(this); }
};

// Unary: !x, -x, +x, typeof x
class UnaryExpression : public Expression {
public:
    Token op;
    unique_ptr<Expression> right;
    UnaryExpression(Token op, unique_ptr<Expression> right)
        : op(op), right(std::move(right)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitUnary(this); }
};

// Binary: x + y, x * y,
class BinaryExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    
    BinaryExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}
    
    BinaryExpression(Token op, unique_ptr<Expression> expr, unique_ptr<Expression> right) : left(std::move(expr)), op(op), right(std::move(right)) {}
    
    void accept(ExpressionVisitor& visitor) override { visitor.visitBinary(this); }
};

// Assignment: x = y, x += y
class AssignmentExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    AssignmentExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitAssignment(this); }
};

// Conditional (ternary): cond ? then : else
class ConditionalExpression : public Expression {
public:
    unique_ptr<Expression> test;
    unique_ptr<Expression> consequent;
    unique_ptr<Expression> alternate;
    ConditionalExpression(unique_ptr<Expression> test,
                          unique_ptr<Expression> consequent,
                          unique_ptr<Expression> alternate)
        : test(std::move(test)), consequent(std::move(consequent)), alternate(std::move(alternate)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitConditional(this); }
};

// Logical: &&, ||, ??
class LogicalExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    LogicalExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitLogical(this); }
};

// Call: f(x,y)
class CallExpression : public Expression {
public:
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    CallExpression(unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitCall(this); }
};

// Member: obj.prop or obj["prop"]
class MemberExpression : public Expression {
public:
    unique_ptr<Expression> object;
    unique_ptr<Expression> property;
    bool computed; // true for [], false for .
    Token name;
    
    MemberExpression(unique_ptr<Expression> object, unique_ptr<Expression> property, bool computed)
        : object(std::move(object)), property(std::move(property)), computed(computed) {}

    MemberExpression(unique_ptr<Expression> object, Token name, bool computed)
        : object(std::move(object)), name(name), computed(computed) {}

    void accept(ExpressionVisitor& visitor) override { visitor.visitMember(this); }
    
};

// this
class ThisExpression : public Expression {
public:
    void accept(ExpressionVisitor& visitor) override { visitor.visitThis(this); }
};

// super
class SuperExpression : public Expression {
public:
    void accept(ExpressionVisitor& visitor) override { visitor.visitSuper(this); }
};

// new expr()
class NewExpression : public Expression {
public:
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    NewExpression(unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitNew(this); }
};

// [a,b,...]
class ArrayLiteralExpression : public Expression {
public:
    vector<unique_ptr<Expression>> elements;
    explicit ArrayLiteralExpression(vector<unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitArray(this); }
};

// { key: value, ... }
class ObjectLiteralExpression : public Expression {
public:
    vector<pair<Token, unique_ptr<Expression>>> props;
    explicit ObjectLiteralExpression(vector<pair<Token, unique_ptr<Expression>>> props)
        : props(std::move(props)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitObject(this); }
};

// key:value inside object
class PropertyExpression : public Expression {
public:
    unique_ptr<Expression> key;
    unique_ptr<Expression> value;
    PropertyExpression(unique_ptr<Expression> key, unique_ptr<Expression> value)
        : key(std::move(key)), value(std::move(value)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitProperty(this); }
};

// Sequence (comma operator): (a, b, c)
class SequenceExpression : public Expression {
public:
    vector<unique_ptr<Expression>> expressions;
    explicit SequenceExpression(vector<unique_ptr<Expression>> expressions)
        : expressions(std::move(expressions)) {}
    void accept(ExpressionVisitor& visitor) override { visitor.visitSequence(this); }
};

class UpdateExpression : public Expression {
public:
    Token op;                       // ++ or --
    unique_ptr<Expression> argument; // the variable/expression being updated
    bool prefix;                    // true if prefix (++x), false if postfix (x++)

    explicit UpdateExpression(Token op, unique_ptr<Expression> argument, bool prefix)
        : op(op), argument(std::move(argument)), prefix(prefix) {}

    void accept(ExpressionVisitor& visitor) override {
        visitor.visitUpdate(this);
    }
};

class FalseKeyword : public Expression {
public:
    FalseKeyword() {}
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitFalseKeyword(this);
    }

};

class TrueKeyword : public Expression {
public:
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitTrueKeyword(this);
    }

};

class NumericLiteral : public Expression {
public:
    const string text;
    NumericLiteral(const string text) : text(text) {}
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitNumericLiteral(this);
    }

};

class StringLiteral : public Expression {
public:
    StringLiteral(const string text) : text(text) {}
    const string text;
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitStringLiteral(this);
    }

};

class PublicKeyword : public Expression {
public:
    PublicKeyword() {}
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitPublicKeyword(this);
    }

};

class PrivateKeyword : public Expression {
public:
    PrivateKeyword() {}
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitPrivateKeyword(this);
    }

};

class ProtectedKeyword : public Expression {
public:
    ProtectedKeyword() {}
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitProtectedKeyword(this);
    }

};

class StaticKeyword : public Expression {
public:
    StaticKeyword() {}
    void accept(ExpressionVisitor& visitor) override {
        visitor.visitStaticKeyword(this);
    }

};

#endif /* Expression_hpp */
