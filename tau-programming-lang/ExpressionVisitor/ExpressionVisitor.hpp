//
//  ExpressionVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 20/08/2025.
//

#ifndef ExpressionVisitor_hpp
#define ExpressionVisitor_hpp

#include <stdio.h>

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

#endif /* ExpressionVisitor_hpp */
