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
#include "../Interpreter/R.hpp"

using std::string;
using std::unique_ptr;
using std::vector;

// Base Expression
class Expression {
public:
    virtual ~Expression() = default;
    virtual R accept(ExpressionVisitor& visitor) = 0;
};

// Literal
class LiteralExpression : public Expression {
public:
    Token token;
    explicit LiteralExpression(Token token) : token(token) {}
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitLiteral(this); }
};

// Identifier
class IdentifierExpression : public Expression {
public:
    string name;
    Token token;
    
    explicit IdentifierExpression(const string& name) : name(name) {}
    explicit IdentifierExpression(Token token) : token(token), name(token.lexeme) {}

    R accept(ExpressionVisitor& visitor) { return visitor.visitIdentifier(this); }
};

// Unary: !x, -x, +x, typeof x
class UnaryExpression : public Expression {
public:
    Token op;
    unique_ptr<Expression> right;
    UnaryExpression(Token op, unique_ptr<Expression> right)
        : op(op), right(std::move(right)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitUnary(this); }
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
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitBinary(this); }
};

// Assignment: x = y, x += y
class AssignmentExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    AssignmentExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitAssignment(this); }
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
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitConditional(this); }
};

// Logical: &&, ||, ??
class LogicalExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    LogicalExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitLogical(this); }
};

// Call: f(x,y)
class CallExpression : public Expression {
public:
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    CallExpression(unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitCall(this); }
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

    
    R accept(ExpressionVisitor& visitor) { return visitor.visitMember(this); }
    
};

// this
class ThisExpression : public Expression {
public:
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitThis(this); }
};

// super
class SuperExpression : public Expression {
public:
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitSuper(this); }
};

// new expr()
class NewExpression : public Expression {
public:
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    NewExpression(unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitNew(this); }
};

// [a,b,...]
class ArrayLiteralExpression : public Expression {
public:
    vector<unique_ptr<Expression>> elements;
    explicit ArrayLiteralExpression(vector<unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitArray(this); }
};

// { key: value, ... }
class ObjectLiteralExpression : public Expression {
public:
    vector<pair<Token, unique_ptr<Expression>>> props;
    explicit ObjectLiteralExpression(vector<pair<Token, unique_ptr<Expression>>> props)
        : props(std::move(props)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitObject(this); }
};

// key:value inside object
class PropertyExpression : public Expression {
public:
    unique_ptr<Expression> key;
    unique_ptr<Expression> value;
    PropertyExpression(unique_ptr<Expression> key, unique_ptr<Expression> value)
        : key(std::move(key)), value(std::move(value)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitProperty(this); }
};

// Sequence (comma operator): (a, b, c)
class SequenceExpression : public Expression {
public:
    vector<unique_ptr<Expression>> expressions;
    explicit SequenceExpression(vector<unique_ptr<Expression>> expressions)
        : expressions(std::move(expressions)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitSequence(this); }
};

class UpdateExpression : public Expression {
public:
    Token op;                       // ++ or --
    unique_ptr<Expression> argument; // the variable/expression being updated
    bool prefix;                    // true if prefix (++x), false if postfix (x++)

    explicit UpdateExpression(Token op, unique_ptr<Expression> argument, bool prefix)
        : op(op), argument(std::move(argument)), prefix(prefix) {}

    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitUpdate(this);
    }
};

class FalseKeyword : public Expression {
public:
    FalseKeyword() {}
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitFalseKeyword(this);
    }

};

class TrueKeyword : public Expression {
public:
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitTrueKeyword(this);
    }

};

class NumericLiteral : public Expression {
public:
    const string text;
    NumericLiteral(const string text) : text(text) {}
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitNumericLiteral(this);
    }

};

class StringLiteral : public Expression {
public:
    StringLiteral(const string text) : text(text) {}
    const string text;
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitStringLiteral(this);
    }

};

class PublicKeyword : public Expression {
public:
    PublicKeyword() {}
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitPublicKeyword(this);
    }

};

class PrivateKeyword : public Expression {
public:
    PrivateKeyword() {}
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitPrivateKeyword(this);
    }

};

class ProtectedKeyword : public Expression {
public:
    ProtectedKeyword() {}
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitProtectedKeyword(this);
    }

};

class StaticKeyword : public Expression {
public:
    StaticKeyword() {}
    
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitStaticKeyword(this);
    }

};

#endif /* Expression_hpp */
