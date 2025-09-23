//
//  CodeGenerator.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#pragma once
#include <memory>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include <iostream>


#include "../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../Statements/StatementVisitor.hpp"
//#include "../Interpreter/R.hpp"
#include "../Statements/Statements.hpp"
#include "../Expression/Expression.hpp"
#include "../Interpreter/Utils/Utils.h"

#include "VM/Bytecode.hpp"
#include "VM/Chunk.hpp"
#include "VM/VM.hpp"

using namespace std;

using std::shared_ptr;
using std::unordered_map;
using std::string;
using std::vector;

class CodeGen : public ExpressionVisitor, public StatementVisitor {
public:
    CodeGen();

    shared_ptr<Chunk> generate(const vector<unique_ptr<Statement>> &program);
    void emitAssignment(BinaryExpression* expr);
    
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
        
private:
    shared_ptr<Chunk> cur; // current chunk being emitted
    // locals map for current function: name -> slot index
    unordered_map<string, uint32_t> locals;
    uint32_t nextLocalSlot = 0;

    // helpers
    void emit(OpCode op);
    void emitUint32(uint32_t v);
    void emitUint8(uint8_t v);
    int emitConstant(const Value &v);
    uint32_t makeLocal(const string &name); // allocate a local slot
    bool hasLocal(const string &name);
    uint32_t getLocal(const string &name);
    void resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames);

    // jump helpers
    int emitJump(OpCode op);
    void patchJump(int jumpPos);
    void emitLoop(uint32_t offset);
    
};

inline uint32_t readUint32(const Chunk* chunk, size_t offset);

size_t disassembleInstruction(const Chunk* chunk, size_t offset);

void disassembleChunk(const Chunk* chunk, const std::string& name);
Token createToken(TokenType type);
