//
//  ExpressionVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 20/08/2025.
//

#ifndef ExpressionVisitor_hpp
#define ExpressionVisitor_hpp

#include <stdio.h>

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

// Visitor interface
class ExpressionVisitor {
public:
    virtual ~ExpressionVisitor() = default;

    virtual void visitLiteral(LiteralExpression* expr) = 0;
    virtual void visitIdentifier(IdentifierExpression* expr) = 0;
    virtual void visitUnary(UnaryExpression* expr) = 0;
    virtual void visitBinary(BinaryExpression* expr) = 0;
    virtual void visitAssignment(AssignmentExpression* expr) = 0;
    virtual void visitConditional(ConditionalExpression* expr) = 0;
    virtual void visitLogical(LogicalExpression* expr) = 0;
    virtual void visitCall(CallExpression* expr) = 0;
    virtual void visitMember(MemberExpression* expr) = 0;
    virtual void visitThis(ThisExpression* expr) = 0;
    virtual void visitSuper(SuperExpression* expr) = 0;
    virtual void visitNew(NewExpression* expr) = 0;
    virtual void visitArray(ArrayLiteralExpression* expr) = 0;
    virtual void visitObject(ObjectLiteralExpression* expr) = 0;
    virtual void visitProperty(PropertyExpression* expr) = 0;
    virtual void visitSequence(SequenceExpression* expr) = 0;
    virtual void visitUpdate(UpdateExpression* expr) = 0;
    virtual void visitFalseKeyword(FalseKeyword* expr) = 0;
    virtual void visitTrueKeyword(TrueKeyword* expr) = 0;
    virtual void visitNumericLiteral(NumericLiteral* expr) = 0;
    virtual void visitStringLiteral(StringLiteral* expr) = 0;
    virtual void visitPublicKeyword(PublicKeyword* expr) = 0;
    virtual void visitPrivateKeyword(PrivateKeyword* expr) = 0;
    virtual void visitProtectedKeyword(ProtectedKeyword* expr) = 0;
    virtual void visitStaticKeyword(StaticKeyword* expr) = 0;

};

#endif /* ExpressionVisitor_hpp */
