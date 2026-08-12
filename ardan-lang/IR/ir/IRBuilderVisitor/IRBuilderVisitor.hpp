//
//  IRBuilderVisitor.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/05/2026.
//

#ifndef IRBuilderVisitor_hpp
#define IRBuilderVisitor_hpp

#include <stdio.h>
#include <variant>
#include <string>
#include <unordered_set>
#include <vector>

#include "ExpressionVisitor/ExpressionVisitor.hpp"
#include "Statements/StatementVisitor.hpp"
#include "Statements/Statements.hpp"
#include "IR/ir/IRFunction/IRFunction.hpp"
#include "IR/ir/IRModule/IRModule.hpp"
#include "IR/ir/IRValue/IRValue.hpp"
#include "Compiler/RegisterAllocator/RegisterAllocator.hpp"
#include "Interpreter/Utils/Utils.h"

struct Scope {
    enum class Type { Global, Function, Block };
    Type type;
    Scope* parent;
    std::unordered_map<std::string, int> locals;
    
    // @TODO: check if we should add upvalues.
    
    unordered_map<string, shared_ptr<IRValue>> symbols;
};

struct Context {
    unordered_map<string, int> slots;
    shared_ptr<IRValue> contextValue;
};

class IRBuilderVisitor : public ExpressionVisitor, public StatementVisitor {
    
public:
    void build(const vector<unique_ptr<Statement>> &program);
    IRModule irModule;
    
    IRFunction* currentFunction = nullptr;
    BasicBlock* currentBlock = nullptr;
    
    int temp = 0;
    int blockId = 0;
    
    vector<Scope> scopes;
    vector<Context> contexts;
    bool currentFunctionOwnsTopContextFrame = false;
        
    shared_ptr<IRValue> createTemp(IRType type) {
        auto tempIRValue = make_shared<IRValue>("%" + std::to_string(temp++), type);
        return tempIRValue;
    }
    
    BasicBlock* createBlock(const std::string& prefix) {
        unique_ptr<BasicBlock> basicBlock = std::make_unique<BasicBlock>(prefix + std::to_string(blockId++));
        
        BasicBlock* basicBlockPtr = basicBlock.get();
        currentFunction->blocks.push_back(std::move(basicBlock));
        return basicBlockPtr;
    }
    
    IRInstruction createInstruction(IROp op, IRType type, std::vector<shared_ptr<IRValue>> inputs) {

        auto dst = createTemp(type);
        
        auto ins = IRInstruction(op, dst, inputs);
        
        return ins;
        
    }
    
    void emit(const IRInstruction inst);
    IROp getBinaryOp(const Token& op);
    
private:
    
    struct Variable {
        string name;
        BindingKind kind;
    };
    
    vector<Variable> variables;
    RegisterAllocator allocator;
    
    shared_ptr<IRValue> lookup(std::string name);
    void declare(string name, shared_ptr<IRValue> value, BindingKind kind);
    
    void create();
    void store(string id, shared_ptr<IRValue> reg);
    void load();
    
    BindingKind getBindingKind(string kind);
    std::unordered_set<string> collectDeclaredNames(Statement* body);
    unordered_set<string> collectNestedVariables(Statement* body);
    
    
    void walkForFreeVars(Expression* expr, vector<unordered_set<string>>& boundStack, unordered_set<string>& freeVars);
    void walkForFreeVars(Statement* stmt, vector<unordered_set<string>>& boundStack, unordered_set<string>& freeVars);
    unordered_set<string> freeVariablesOf(const vector<unique_ptr<Statement>>& body, vector<string>& names);
    void collectFreeVars(Expression* stmt, unordered_set<string>& names);
    void collectFreeVars(Statement* stmt, unordered_set<string>& names);
    
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
