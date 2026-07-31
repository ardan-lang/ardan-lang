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

#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cerrno>

#include "Scanner/Token/Token.hpp"
#include "Scanner/Token/TokenType.h"
#include "ExpressionVisitor/ExpressionVisitor.hpp"
#include "Interpreter/R.hpp"

using std::string;
using std::unique_ptr;
using std::vector;

class Expression {
public:
    virtual ~Expression() = default;
    virtual R accept(ExpressionVisitor& visitor) = 0;
};

class LiteralExpression : public Expression {
public:
    Token token;
    explicit LiteralExpression(Token token) : token(token) {}
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitLiteral(this); }
};

class IdentifierExpression : public Expression {
public:
    string name;
    Token token;
    
    explicit IdentifierExpression(const string& name) : name(name) {}
    explicit IdentifierExpression(Token token) : token(token), name(token.lexeme) {}

    R accept(ExpressionVisitor& visitor) { return visitor.visitIdentifier(this); }
};

class UnaryExpression : public Expression {
public:
    Token op;
    unique_ptr<Expression> right;
    UnaryExpression(Token op, unique_ptr<Expression> right)
        : op(op), right(std::move(right)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitUnary(this); }
};

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

class AssignmentExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    AssignmentExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitAssignment(this); }
};

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

class LogicalExpression : public Expression {
public:
    unique_ptr<Expression> left;
    Token op;
    unique_ptr<Expression> right;
    LogicalExpression(unique_ptr<Expression> left, Token op, unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitLogical(this); }
};

class CallExpression : public Expression {
public:
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    CallExpression(unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitCall(this); }
};

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


class ThisExpression : public Expression {
public:
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitThis(this); }
};


class SuperExpression : public Expression {
public:
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitSuper(this); }
};

class NewExpression : public Expression {
public:
    Token token;
    unique_ptr<Expression> callee;
    vector<unique_ptr<Expression>> arguments;
    NewExpression(Token token, unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)), token(token) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitNew(this); }
};

class ArrayLiteralExpression : public Expression {
public:
    Token token;
    vector<unique_ptr<Expression>> elements;
    explicit ArrayLiteralExpression(Token token, vector<unique_ptr<Expression>> elements)
        : elements(std::move(elements)), token(token) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitArray(this); }
};

class ObjectLiteralExpression : public Expression {
public:
    Token token;
    vector<pair<Token, unique_ptr<Expression>>> props;
    explicit ObjectLiteralExpression(Token token, vector<pair<Token, unique_ptr<Expression>>> props)
        : props(std::move(props)), token(token) {}
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitObject(this); }
};

class PropertyExpression : public Expression {
public:
    unique_ptr<Expression> key;
    unique_ptr<Expression> value;
    PropertyExpression(unique_ptr<Expression> key, unique_ptr<Expression> value)
        : key(std::move(key)), value(std::move(value)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitProperty(this); }
};

class CommaExpression : public Expression {
public:
    unique_ptr<Expression> left;
    unique_ptr<Expression> right;
    
    explicit CommaExpression(unique_ptr<Expression> left, unique_ptr<Expression> right) : left(std::move(left)), right(std::move(right)) {}
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitComma(this); }


};

class SequenceExpression : public Expression {
public:
    vector<unique_ptr<Expression>> expressions;
    explicit SequenceExpression(vector<unique_ptr<Expression>> expressions)
        : expressions(std::move(expressions)) {}
    
    
    R accept(ExpressionVisitor& visitor) { return visitor.visitSequence(this); }
};

class UpdateExpression : public Expression {
public:
    Token op;
    unique_ptr<Expression> argument;
    bool prefix;

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

//enum NumberType {
//    NONE,
//    INT,
//    DOUBLE,
//    FLOAT,
//    SHORT,
//    LONG,
//    
//};
//
//class NumericLiteral : public Expression {
//public:
//    const R value;
//    const NumberType type = NumberType::NONE;
//
//        NumericLiteral(const std::string& text)
//            : value((parseNumber(text))) {}
//
//    static R parseNumber(const std::string& text) {
//
//        if (text.find('.') != std::string::npos || text.find('e') != std::string::npos || text.find('E') != std::string::npos) {
//            long double v = std::stold(text);
//            
//            if (v >= std::numeric_limits<float>::lowest() && v <= std::numeric_limits<float>::max()) {
//                return static_cast<float>(v);
//            }
//            if (v >= std::numeric_limits<double>::lowest() && v <= std::numeric_limits<double>::max()) {
//                return static_cast<double>(v);
//            }
//            return v; // long double
//        }
//        
//        bool isNegative = !text.empty() && text[0] == '-';
//        
//        if (isNegative) {
//            long long v = std::stoll(text);
//            
//            if (v >= std::numeric_limits<short>::min() && v <= std::numeric_limits<short>::max())
//                return static_cast<short>(v);
//            if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max())
//                return static_cast<int>(v);
//            if (v >= std::numeric_limits<long>::min() && v <= std::numeric_limits<long>::max())
//                return static_cast<long>(v);
//            return v;
//        } else {
//            unsigned long long v = std::stoull(text);
//            
//            if (v <= std::numeric_limits<unsigned short>::max())
//                return static_cast<unsigned short>(v);
//            if (v <= std::numeric_limits<unsigned int>::max())
//                return static_cast<unsigned int>(v);
//            if (v <= std::numeric_limits<unsigned long>::max())
//                return static_cast<unsigned long>(v);
//            if (v <= std::numeric_limits<long>::max())
//                return static_cast<long>(v);
//            if (v <= std::numeric_limits<long long>::max())
//                return static_cast<long long>(v);
//            return v;
//        }
//    }
//    
//    R accept(ExpressionVisitor& visitor) {
//        return visitor.visitNumericLiteral(this);
//    }
//
//};

enum class NumberType {
    NONE,
    SHORT, USHORT,
    INT, UINT,
    LONG, ULONG,
    LONG_LONG, ULONG_LONG,
    FLOAT, DOUBLE, LONG_DOUBLE
};

struct LiteralParseError : std::runtime_error {
    explicit LiteralParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class NumericLiteral : public Expression {
public:
    const R value;
    const NumberType type;

    explicit NumericLiteral(const std::string& text)
        : NumericLiteral(parse(text)) {}

    R accept(ExpressionVisitor& visitor) {
        return visitor.visitNumericLiteral(this);
    }

private:
    struct Parsed {
        R value;
        NumberType type;
    };

    explicit NumericLiteral(Parsed p) : value(std::move(p.value)), type(p.type) {}

    static std::string stripSeparators(const std::string& text) {
        std::string out;
        out.reserve(text.size());
        for (char c : text) if (c != '\'') out.push_back(c);
        return out;
    }

    static void splitSuffix(const std::string& text, std::string& digits, std::string& suffix) {
        size_t end = text.size();
        while (end > 0 && std::isalpha(static_cast<unsigned char>(text[end - 1])))
            --end;
        digits = text.substr(0, end);
        suffix = text.substr(end);
        for (char& c : suffix) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    static bool isFloatingLiteral(const std::string& digits, const std::string& suffix) {
        if ((suffix == "f" || suffix == "l") && digits.find('.') != std::string::npos) return true;
        if (digits.find('.') != std::string::npos) return true;
        
        bool isHex = digits.size() > 1 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X');
        if (!isHex && (digits.find('e') != std::string::npos || digits.find('E') != std::string::npos))
            return true;
        return suffix == "f";
    }

    static int detectBase(const std::string& digits) {
        if (digits.size() > 1 && digits[0] == '0') {
            char c = digits[1];
            if (c == 'x' || c == 'X') return 16;
            if (c == 'b' || c == 'B') return 2;
            return 8;
        }
        return 10;
    }

    static Parsed parseFloating(const std::string& digits, const std::string& suffix) {
        errno = 0;
        char* endp = nullptr;
        long double v = std::strtold(digits.c_str(), &endp);
        if (endp == digits.c_str() || errno == ERANGE)
            throw LiteralParseError("invalid or out-of-range float literal: " + digits);

        if (suffix == "f")
            return { static_cast<float>(v), NumberType::FLOAT };
        if (suffix == "l")
            return { v, NumberType::LONG_DOUBLE };

        if (v >= std::numeric_limits<float>::lowest() && v <= std::numeric_limits<float>::max())
            return { static_cast<float>(v), NumberType::FLOAT };
        if (v >= std::numeric_limits<double>::lowest() && v <= std::numeric_limits<double>::max())
            return { static_cast<double>(v), NumberType::DOUBLE };
        return { v, NumberType::LONG_DOUBLE };
    }

    static Parsed parseSigned(const std::string& digits, const std::string& suffix, int base) {
        errno = 0;
        char* endp = nullptr;
        long long v = std::strtoll(digits.c_str(), &endp, base);
        if (endp == digits.c_str() || errno == ERANGE)
            throw LiteralParseError("invalid or out-of-range integer literal: " + digits);

        if (suffix == "ll") return { v, NumberType::LONG_LONG };
        if (suffix == "l")  return { static_cast<long>(v), NumberType::LONG };

        if (v >= std::numeric_limits<short>::min() && v <= std::numeric_limits<short>::max())
            return { static_cast<short>(v), NumberType::SHORT };
        if (v >= std::numeric_limits<int>::min() && v <= std::numeric_limits<int>::max())
            return { static_cast<int>(v), NumberType::INT };
        if (v >= std::numeric_limits<long>::min() && v <= std::numeric_limits<long>::max())
            return { static_cast<long>(v), NumberType::LONG };
        return { v, NumberType::LONG_LONG };
    }

    static Parsed parseUnsigned(const std::string& digits, const std::string& suffix, int base) {
        errno = 0;
        char* endp = nullptr;
        unsigned long long v = std::strtoull(digits.c_str(), &endp, base);
        if (endp == digits.c_str() || errno == ERANGE)
            throw LiteralParseError("invalid or out-of-range integer literal: " + digits);

        bool forceUnsigned = suffix.find('u') != std::string::npos;
        bool forceLL = suffix.find("ll") != std::string::npos;
        bool forceL  = !forceLL && suffix.find('l') != std::string::npos;

        if (forceLL) return forceUnsigned
            ? Parsed{ v, NumberType::ULONG_LONG }
            : Parsed{ static_cast<long long>(v), NumberType::LONG_LONG };
        if (forceL) return forceUnsigned
            ? Parsed{ static_cast<unsigned long>(v), NumberType::ULONG }
            : Parsed{ static_cast<long>(v), NumberType::LONG };

        if (forceUnsigned) {
            if (v <= std::numeric_limits<unsigned short>::max())
                return { static_cast<unsigned short>(v), NumberType::USHORT };
            if (v <= std::numeric_limits<unsigned int>::max())
                return { static_cast<unsigned int>(v), NumberType::UINT };
            if (v <= std::numeric_limits<unsigned long>::max())
                return { static_cast<unsigned long>(v), NumberType::ULONG };
            return { v, NumberType::ULONG_LONG };
        }

        if (v <= static_cast<unsigned long long>(std::numeric_limits<short>::max()))
            return { static_cast<short>(v), NumberType::SHORT };
        if (v <= static_cast<unsigned long long>(std::numeric_limits<int>::max()))
            return { static_cast<int>(v), NumberType::INT };
        if (v <= static_cast<unsigned long long>(std::numeric_limits<long>::max()))
            return { static_cast<long>(v), NumberType::LONG };
        if (v <= static_cast<unsigned long long>(std::numeric_limits<long long>::max()))
            return { static_cast<long long>(v), NumberType::LONG_LONG };
        return { v, NumberType::ULONG_LONG };
    }

    static Parsed parse(const std::string& rawText) {
        if (rawText.empty())
            throw LiteralParseError("empty numeric literal");

        std::string text = stripSeparators(rawText);

        std::string digits, suffix;
        splitSuffix(text, digits, suffix);

        if (digits.empty())
            throw LiteralParseError("numeric literal has no digits: " + rawText);

        if (isFloatingLiteral(digits, suffix))
            return parseFloating(digits, suffix);

        bool isNegative = digits[0] == '-';
        int base = detectBase(isNegative ? digits.substr(1) : digits);

        return isNegative
            ? parseSigned(digits, suffix, base)
            : parseUnsigned(digits, suffix, base);
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

class NullKeyword : public Expression {
public:
    Token token;
    NullKeyword(Token token) : token(token) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitNullKeyword(this);
    }

};

class UndefinedKeyword : public Expression {
public:
    Token token;
    UndefinedKeyword(Token token) : token(token) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitUndefinedKeyword(this);
    }

};

class AwaitExpression : public Expression {
public:
    unique_ptr<Expression> inner;
    AwaitExpression(unique_ptr<Expression> inner) : inner(std::move(inner)) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitAwaitExpression(this);
    }

};

class YieldExpression : public Expression {
public:
    Token token;
    unique_ptr<Expression> expression;
    bool delegate;
    
    YieldExpression(Token token,
                    unique_ptr<Expression> expression,
                    bool delegate) : token(token), expression(std::move(expression)), delegate(delegate) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitYieldExpression(this);
    }
    
};

class SpreadExpression : public Expression {

public:
    Token token;
    unique_ptr<Expression> expression;

    SpreadExpression(Token token,
                     unique_ptr<Expression> expression) : token(token), expression(std::move(expression)) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitSpreadExpression(this);
    }

};

#endif /* Expression_hpp */
