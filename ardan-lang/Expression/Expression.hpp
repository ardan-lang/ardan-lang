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
#include <variant>
#include <limits>
#include <typeinfo>

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
    Token token;
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    NewExpression(Token token, unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)), token(token) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitNew(this); }
};

// [a,b,...]
class ArrayLiteralExpression : public Expression {
public:
    Token token;
    vector<unique_ptr<Expression>> elements;
    explicit ArrayLiteralExpression(Token token, vector<unique_ptr<Expression>> elements)
        : elements(std::move(elements)), token(token) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitArray(this); }
};

// { key: value, ... }
class ObjectLiteralExpression : public Expression {
public:
    Token token;
    vector<pair<Token, unique_ptr<Expression>>> props;
    explicit ObjectLiteralExpression(Token token, vector<pair<Token, unique_ptr<Expression>>> props)
        : props(std::move(props)), token(token) {}
    
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
    const R value;

        NumericLiteral(const std::string& text)
            : value((parseNumber(text))) {}

    static R parseNumber(const std::string& text) {
        // Check for floating-point
        if (text.find('.') != std::string::npos || text.find('e') != std::string::npos || text.find('E') != std::string::npos) {
            long double v = std::stold(text);
            
            // Try narrowest type
            if (v >= std::numeric_limits<float>::lowest() && v <= std::numeric_limits<float>::max()) {
                return static_cast<float>(v);
            }
            if (v >= std::numeric_limits<double>::lowest() && v <= std::numeric_limits<double>::max()) {
                return static_cast<double>(v);
            }
            return v; // long double
        }
        
        // Otherwise parse as integer
        bool isNegative = !text.empty() && text[0] == '-';
        
        if (isNegative) {
            long long v = std::stoll(text);
            
            if (v >= std::numeric_limits<short>::min() && v <= std::numeric_limits<short>::max())
                return static_cast<short>(v);
            if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max())
                return static_cast<int>(v);
            if (v >= std::numeric_limits<long>::min() && v <= std::numeric_limits<long>::max())
                return static_cast<long>(v);
            return v; // long long
        } else {
            unsigned long long v = std::stoull(text);
            
            if (v <= std::numeric_limits<unsigned short>::max())
                return static_cast<unsigned short>(v);
            if (v <= std::numeric_limits<unsigned int>::max())
                return static_cast<unsigned int>(v);
            if (v <= std::numeric_limits<unsigned long>::max())
                return static_cast<unsigned long>(v);
            if (v <= std::numeric_limits<long>::max())
                return static_cast<long>(v); // fits signed long
            if (v <= std::numeric_limits<long long>::max())
                return static_cast<long long>(v); // fits signed long long
            return v; // unsigned long long
        }
    }
    
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

class RestParameter : public Expression {
public:
    Token token;
    RestParameter(Token token) : token(token) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitRestParameter(this);
    }

};

#endif /* Expression_hpp */
