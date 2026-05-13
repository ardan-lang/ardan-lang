//
//  IRBuilderVisitor.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/05/2026.
//

#ifndef IRBuilderVisitor_hpp
#define IRBuilderVisitor_hpp

#include <stdio.h>
#include "ExpressionVisitor/ExpressionVisitor.hpp"
#include "Statements/StatementVisitor.hpp"
#include "Statements/Statements.hpp"
#include "engines/"

class IRBuilderVisitor : public ExpressionVisitor, public StatementVisitor {

public:
    void build(const vector<unique_ptr<Statement>> &program);
    IRModule module;

        IRFunction* currentFunction = nullptr;
        BasicBlock* currentBlock = nullptr;

        int temp = 0;
        int blockId = 0;

        std::unordered_map<std::string, IRValue> symTable;

        IRValue createTemp(std::string type = "i32") {
            return IRValue("%t" + std::to_string(temp++), type);
        }

        BasicBlock* createBlock(const std::string& prefix) {
            auto bb = std::make_unique<BasicBlock>(
                prefix + std::to_string(blockId++)
            );

            BasicBlock* ptr = bb.get();
            currentFunction->blocks.push_back(std::move(bb));
            return ptr;
        }
    
private:
    
    class RegisterAllocator {
        uint32_t nextReg = 0; // reserve 0 for special uses if needed
        vector<uint32_t> freeRegs;
    public:
        uint32_t alloc() {
            if (!freeRegs.empty()) { uint32_t r = freeRegs.back(); freeRegs.pop_back(); return r; }
            return nextReg++;
        }
        void free(uint32_t r) {
            if (r==0) return; // don't free 0
            freeRegs.push_back(r);
        }
        void reset() { nextReg = 1; freeRegs.clear(); }
        int getNextReg() { return nextReg; }
    };

    enum class BindingKind {
        Var,
        Let,
        Const,
    };

    struct Variable {
        string name;
        BindingKind kind;
    };

    vector<Variable> variables;
    RegisterAllocator allocator;

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
    R visitEnumDeclaration(EnumDeclaration* stmt) override;
    R visitInterfaceDeclaration(InterfaceDeclaration* stmt) override;
    
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
    R visitYieldExpression(YieldExpression* visitor) override;
    R visitSpreadExpression(SpreadExpression* visitor) override;
    R visitComma(CommaExpression *expr) override;

};

#endif /* IRBuilderVisitor_hpp */
