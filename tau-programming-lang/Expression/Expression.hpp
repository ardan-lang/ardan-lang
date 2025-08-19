//
//  Expression.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 19/08/2025.
//

#ifndef Expression_hpp
#define Expression_hpp

#include <stdio.h>

// Forward declarations of AST nodes
class LiteralExpression;
class BinaryExpression;
class VariableExpression;
class GroupingExpression;

class ExpressionVisitor {
public:
    virtual ~ExpressionVisitor() = default;

    virtual void visitLiteral(LiteralExpression* expr) = 0;
    virtual void visitBinary(BinaryExpression* expr) = 0;
    virtual void visitVariable(VariableExpression* expr) = 0;
    virtual void visitGrouping(GroupingExpression* expr) = 0;
};

struct Expression {
    virtual ~Expression() = default;
    virtual void accept(ExpressionVisitor& visitor) = 0;
};

#endif /* Expression_hpp */
