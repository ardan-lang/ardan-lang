//
//  TurboCodeGenerator.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <stdexcept>
#include <iostream>


#include "../../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../../Statements/StatementVisitor.hpp"
#include "../../Statements/Statements.hpp"
#include "../../Expression/Expression.hpp"
#include "../../Interpreter/Utils/Utils.h"

#include "../../Scanner/Scanner.hpp"
#include "../../Parser/Parser.hpp"

#include "./TurboBytecode.hpp"
#include "./TurboChunk.hpp"
#include "./TurboVM.hpp"
#include "./TurboModule.hpp"

using namespace std;

using std::shared_ptr;
using std::unordered_map;
using std::string;
using std::vector;

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
};

class TurboCodeGen : public ExpressionVisitor, public StatementVisitor {
    
    enum class BindingKind {
        Var,
        Let,
        Const,
    };

    enum class Visibility { Public, Protected, Private };
    
    struct PropertyMeta {
        Visibility visibility;
        BindingKind kind;
        bool isStatic;
    };

    struct ClassInfo {
        unordered_map<string, PropertyMeta> fields;
    };

    class RegGuard {
        TurboCodeGen& codegen;
        int reg;
        bool active;
    public:
        RegGuard(int r, TurboCodeGen& cg, bool autoFree = true)
            : codegen(cg), reg(r), active(autoFree) {}

        void release() { active = false; }

        ~RegGuard() {
            if (active) codegen.freeRegister(reg);
        }
    };

    inline RegGuard makeRegGuard(int r, TurboCodeGen& cg, bool autoFree = true) {
        return RegGuard(r, cg, autoFree);
    }

    struct LocalScope {
        unordered_map<string, Value> locals;
        LocalScope* parent = nullptr;
    };

    struct LoopContext {
        int loopStart;         // address of loop condition start
        vector<int> breaks;    // jump addresses that need patching
    };

    struct ExceptionHandler {
        int handlerIP;
        int stackDepth;
    };

    struct ParameterInfo {
        string name;
        bool hasDefault;
        Expression* defaultExpr;
        bool isRest;
    };

    struct UpvalueMeta {
        bool isLocal;   // True if from parent's locals, false if from parent's upvalues
        uint32_t index; // Slot or upvalue index
        string name;
        BindingKind kind;
    };

    struct Local {
        string name;
        int depth;        // scope depth
        bool isCaptured;  // true if used by an inner function
        uint32_t slot_index;
        BindingKind kind;
    };

    struct Global {
        string name;
        BindingKind kind;
    };

private:
    shared_ptr<TurboChunk> cur; // current chunk being emitted
    // locals map for current function: name -> slot index
    // unordered_map<string, uint32_t> locals;
    int scopeDepth;
    TurboCodeGen* enclosing;
    R create(string decl, uint32_t reg_slot, BindingKind kind);
    R store(string decl, uint32_t reg_slot);
    R load(string decl, uint32_t reg_slot);

    BindingKind get_kind(string kind);
    TurboOpCode getBinaryOp(const Token& op);
    RegisterAllocator* registerAllocator = new RegisterAllocator();
    
    // helpers
    TurboOpCode getUnaryOp(const Token& op);
    string resolveImportPath(ImportDeclaration* stmt);
    bool isModuleLoaded(string importPath);
    void registerModule(string importPath);
    vector<string> registered_modules;

    ClassInfo classInfo;
    
    int compileMethod(MethodDefinition& method);

    void emit(TurboOpCode op);
    int emitConstant(const Value &v);
    void emitLoop(uint32_t loopStart);
    uint32_t makeLocal(const string &name); // allocate a local slot
    bool hasLocal(const string &name);
    uint32_t getLocal(const string &name);
    void resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames);
    
    int emitTryPlaceholder();
    void patchTry(int tryPos, int reg);
    void patchTryFinally(int tryPos, int target);
    void patchTryCatch(int tryPos, int target);
    
    void declareLocal(const string& name);
    void emitSetLocal(int slot);
    int paramSlot(const string& name);
    int resolveLocal(const string& name);
    int lookupLocalSlot(const std::string& name);
    
    void declareGlobal(const string& name, BindingKind kind);
    int lookupGlobal(const string& name);
    
    // jump helpers
    int emitJump(TurboOpCode op, int cond_reg);
    int emitJump(TurboOpCode op);
    void patchSingleJump(int jumpPos);
    void patchJump(int jumpPos);
    void patchJump(int jumpPos, int target);
    void emit(TurboOpCode op, int a, int b, int c);
    void emit(TurboOpCode op, int a);
    void emit(TurboOpCode op, int a, int b);
    
    void beginLoop();
    void endLoop();
    vector<LoopContext> loopStack;
    
    vector<ExceptionHandler> handlerStack;
    
    void beginScope();
    void endScope();
    int addUpvalue(bool isLocal, int index, string name, BindingKind kind);
    int resolveUpvalue(const string& name);
        
    inline uint32_t readUint32(const TurboChunk* chunk, size_t offset);
    
    size_t disassembleInstruction(const TurboChunk* chunk, size_t offset);
    
    void disassembleChunk(const TurboChunk* chunk, const std::string& name);
    Token createToken(TokenType type);

public:
    TurboCodeGen();
    TurboCodeGen(shared_ptr<TurboModule> m) : module_(m), cur(nullptr), nextLocalSlot(0) {}
    
    shared_ptr<TurboModule> module_;
    int nextRegister = 0;
    
    vector<Local> locals;
    vector<Global> globals;
    uint32_t nextLocalSlot = 0;
    vector<UpvalueMeta> upvalues;
    
    int allocRegister();
    void freeRegister(uint32_t slot);
    
    size_t generate(const vector<unique_ptr<Statement>> &program);
    
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
        
};
