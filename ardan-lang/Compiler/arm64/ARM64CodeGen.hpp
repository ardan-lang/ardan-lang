//
//  ARM64CodeGen.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 30/10/2025.
//

#ifndef ARM64CodeGen_hpp
#define ARM64CodeGen_hpp

#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "../../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../../Statements/StatementVisitor.hpp"
#include "../../Statements/Statements.hpp"
#include "../../Expression/Expression.hpp"
#include "../../Interpreter/Utils/Utils.h"
#include "../../Scanner/Scanner.hpp"
#include "../../Parser/Parser.hpp"

#include "../VM/Bytecode.hpp"
#include "../VM/Chunk.hpp"
#include "../VM/VM.hpp"
#include "../VM/Module.hpp"

#include "ARM64Emitter.hpp"

class ARM64CodeGen : public ExpressionVisitor, public StatementVisitor {
public:
    ARM64Emitter emitter;
    RegisterAllocator regAlloc;

    R visitExpression(ExpressionStatement* stmt) override;
    R visitBlock(BlockStatement* stmt) override;
    R visitVariable(VariableStatement* stmt) override;
    R visitIf(IfStatement* stmt) override;
    R visitWhile(WhileStatement* stmt) override;
    R visitFor(ForStatement* stmt) override;
    R visitReturn(ReturnStatement* stmt) override;
    R visitFunction(FunctionDeclaration* stmt) override;
    R visitBinary(BinaryExpression* expr) override;
    R visitLiteral(LiteralExpression* expr) override;
    R visitNumericLiteral(NumericLiteral* expr) override;
    R visitStringLiteral(StringLiteral* expr) override;
    R visitIdentifier(IdentifierExpression* expr) override;
    R visitCall(CallExpression* expr) override;
    R visitMember(MemberExpression* expr) override;
    R visitNew(NewExpression* expr) override;
    R visitArray(ArrayLiteralExpression* expr) override;
    R visitObject(ObjectLiteralExpression* expr) override;
    R visitConditional(ConditionalExpression* expr) override;
    R visitUnary(UnaryExpression* expr) override;
    R visitArrowFunction(ArrowFunction* expr) override;
    R visitFunctionExpression(FunctionExpression* expr) override;
    R visitTemplateLiteral(TemplateLiteral* expr) override;
    R visitImportDeclaration(ImportDeclaration* stmt) override;

    R visitAssignment(AssignmentExpression* expr) override;
    R visitLogical(LogicalExpression* expr) override;
    R visitThis(ThisExpression* expr) override;
    R visitSuper(SuperExpression* expr) override;
    R visitProperty(PropertyExpression* expr) override;
    R visitSequence(SequenceExpression* expr) override;
    R visitUpdate(UpdateExpression* expr) override;
    R visitFalseKeyword(FalseKeyword* expr) override;
    R visitTrueKeyword(TrueKeyword* expr) override;
    R visitPublicKeyword(PublicKeyword* expr) override;
    R visitPrivateKeyword(PrivateKeyword* expr) override;
    R visitProtectedKeyword(ProtectedKeyword* expr) override;
    R visitStaticKeyword(StaticKeyword* expr) override;
    R visitRestParameter(RestParameter* expr) override;
    R visitClassExpression(ClassExpression* expr) override;
    R visitNullKeyword(NullKeyword* expr) override;
    R visitUndefinedKeyword(UndefinedKeyword* expr) override;
    R visitAwaitExpression(AwaitExpression* expr) override;
    R visitUIExpression(UIViewExpression* visitor) override;
    
    R visitBreak(BreakStatement* stmt) override;
    R visitContinue(ContinueStatement* stmt) override;
    R visitThrow(ThrowStatement* stmt) override;
    R visitEmpty(EmptyStatement* stmt) override;
    R visitClass(ClassDeclaration* stmt) override;
    R visitMethodDefinition(MethodDefinition* stmt) override;
    R visitDoWhile(DoWhileStatement* stmt) override;
    R visitSwitchCase(SwitchCase* stmt) override;
    R visitSwitch(SwitchStatement* stmt) override;
    R visitCatch(CatchClause* stmt) override;
    R visitTry(TryStatement* stmt) override;
    R visitForIn(ForInStatement* stmt) override;
    R visitForOf(ForOfStatement* stmt) override;
    R visitEnumDeclaration(EnumDeclaration* stmt) override;
    R visitInterfaceDeclaration(InterfaceDeclaration* stmt) override;

};

#endif /* ARM64CodeGen_hpp */
