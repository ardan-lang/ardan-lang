//
//  ExpressionVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 20/08/2025.
//

#ifndef ExpressionVisitor_hpp
#define ExpressionVisitor_hpp

#include <stdio.h>
#include "../Interpreter/R.hpp"

// Forward declare all expression classes
class LiteralExpression;
class IdentifierExpression;
class UnaryExpression;
class BinaryExpression;
class AssignmentExpression;
class ConditionalExpression;
class LogicalExpression;
class CallExpression;
class MemberExpression;
class ThisExpression;
class SuperExpression;
class NewExpression;
class ArrayLiteralExpression;
class ObjectLiteralExpression;
class PropertyExpression;
class SequenceExpression;
class UpdateExpression;
class FalseKeyword;
class TrueKeyword;
class NumericLiteral;
class StringLiteral;
class PublicKeyword;
class PrivateKeyword;
class ProtectedKeyword;
class StaticKeyword;
class ArrowFunction;
class TemplateLiteral;

// Visitor interface
class ExpressionVisitor {
public:
    virtual ~ExpressionVisitor() = default;

    virtual R visitLiteral(LiteralExpression* expr) = 0;
    virtual R visitIdentifier(IdentifierExpression* expr) = 0;
    virtual R visitUnary(UnaryExpression* expr) = 0;
    virtual R visitBinary(BinaryExpression* expr) = 0;
    virtual R visitAssignment(AssignmentExpression* expr) = 0;
    virtual R visitConditional(ConditionalExpression* expr) = 0;
    virtual R visitLogical(LogicalExpression* expr) = 0;
    virtual R visitCall(CallExpression* expr) = 0;
    virtual R visitMember(MemberExpression* expr) = 0;
    virtual R visitThis(ThisExpression* expr) = 0;
    virtual R visitSuper(SuperExpression* expr) = 0;
    virtual R visitNew(NewExpression* expr) = 0;
    virtual R visitArray(ArrayLiteralExpression* expr) = 0;
    virtual R visitObject(ObjectLiteralExpression* expr) = 0;
    virtual R visitProperty(PropertyExpression* expr) = 0;
    virtual R visitSequence(SequenceExpression* expr) = 0;
    virtual R visitUpdate(UpdateExpression* expr) = 0;
    virtual R visitFalseKeyword(FalseKeyword* expr) = 0;
    virtual R visitTrueKeyword(TrueKeyword* expr) = 0;
    virtual R visitNumericLiteral(NumericLiteral* expr) = 0;
    virtual R visitStringLiteral(StringLiteral* expr) = 0;
    virtual R visitPublicKeyword(PublicKeyword* expr) = 0;
    virtual R visitPrivateKeyword(PrivateKeyword* expr) = 0;
    virtual R visitProtectedKeyword(ProtectedKeyword* expr) = 0;
    virtual R visitStaticKeyword(StaticKeyword* expr) = 0;
    virtual R visitArrowFunction(ArrowFunction* expr) = 0;
    virtual R visitTemplateLiteral(TemplateLiteral* expr) = 0;
    
};

#endif /* ExpressionVisitor_hpp */
