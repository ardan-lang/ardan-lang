//
//  CodeGenerator.hpp
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


#include "../ExpressionVisitor/ExpressionVisitor.hpp"
#include "../Statements/StatementVisitor.hpp"
#include "../Statements/Statements.hpp"
#include "../Expression/Expression.hpp"
#include "../Interpreter/Utils/Utils.h"
#include "../Scanner/Scanner.hpp"
#include "../Parser/Parser.hpp"

#include "VM/Bytecode.hpp"
#include "VM/Chunk.hpp"
#include "VM/VM.hpp"
#include "VM/Module.hpp"

using namespace std;

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

struct PropertyLookup {
    int level;
    PropertyMeta meta;
    BindingKind kind;
};

struct ClassInfo {
    string name;
    string super_class_name;
    unordered_map<string, PropertyMeta> fields;

};

struct LoopContext {
    int loopStart;              // address of loop condition start
    vector<int> breaks;    // jump addresses that need patching
    vector<int> continues;
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

struct ClosureInfo {
    uint8_t ci;
    vector<UpvalueMeta> upvalues;
};

struct FieldInfo {
    string name;
    enum class Access { Public, Private } access;
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

using std::shared_ptr;
using std::unordered_map;
using std::string;
using std::vector;

class CodeGen : public ExpressionVisitor, public StatementVisitor {
public:
    CodeGen();
    CodeGen(std::shared_ptr<Module> m) : module_(m), cur(nullptr), nextLocalSlot(0) { }
    shared_ptr<Module> module_;
    
    string resolveImportPath(ImportDeclaration* stmt);
    bool isModuleLoaded(string importPath);
    void registerModule(string importPath);
    vector<string> registered_modules;
    
    size_t generate(const vector<unique_ptr<Statement>> &program);
    
    unordered_map<string, ClosureInfo> closure_infos = {};

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
    R visitYieldExpression(YieldExpression* visitor) override;
    R visitSpreadExpression(SpreadExpression* visitor) override;

private:
    shared_ptr<Chunk> cur; // current chunk being emitted
    // locals map for current function: name -> slot index
    vector<Local> locals;
    vector<Global> globals;
    uint32_t nextLocalSlot = 0;
    vector<UpvalueMeta> upvalues;
    int scopeDepth;
    CodeGen* enclosing;
    
    ClassInfo classInfo;
    unordered_map<string, ClassInfo> classes;

    void compileMethod(MethodDefinition& method);
    R create(string decl, BindingKind kind);
    R store(string decl);
    R load(string decl);
    
    int resolveLocal(const std::string& name);
    
    // helpers
    void emit(OpCode op);
    void emitUint32(uint32_t v);
    void emitUint8(uint8_t v);
    int emitConstant(const Value &v);
    uint32_t makeLocal(const string &name, BindingKind kind); // allocate a local slot
    bool hasLocal(const string &name);
    uint32_t getLocal(const string &name);
    void resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames);

    int emitTryPlaceholder();
    void patchTry(int pos);
    void patchTryFinally(int tryPos, int target);
    void patchTryCatch(int tryPos, int target);
    
    void declareLocal(const string& name, BindingKind kind);
    void emitSetLocal(int slot);
    int paramSlot(const string& name);
    
    // jump helpers
    int emitJump(OpCode op);
    void patchJump(int jumpPos);
    void patchJump(int jumpPos, int target);
    void emitLoop(uint32_t offset);
    
    void beginLoop();
    void endLoop();
    vector<LoopContext> loopStack;
    
    vector<ExceptionHandler> handlerStack;
    
    void beginScope();
    void endScope();
    int addUpvalue(bool isLocal, int index, string name, BindingKind kind);
    int resolveUpvalue(const string& name);
    size_t disassembleInstruction(const Chunk* chunk, size_t offset);
    void disassembleChunk(const Chunk* chunk, const std::string& name);
    BindingKind get_kind(string kind);
    inline uint32_t readUint32(const Chunk* chunk, size_t offset);
    Token createToken(TokenType type);
    PropertyLookup lookupClassProperty(string prop_name);
    int lookupGlobal(const string& name);
    void declareVariableScoping(const string& name, BindingKind kind);
    void declareGlobal(const string& name, BindingKind kind);
    int recordInstanceField(const string& classId, const string& fieldId, Expression* initExpr, const PropertyMeta& propMeta);
    string evaluate_property(Expression* expr);
    void patchContinueStatement();
    
    CodeGen compileFunction(string name, Expression* parameters);
    void emitParameterInitializationLogic(CodeGen nested, vector<string> paramNames, vector<ParameterInfo> parameterInfos);
    void collectParameterInfo(Expression* parameters, vector<string>& paramNames,
                              vector<ParameterInfo>& parameterInfos
                              );
    R compileArgument(vector<unique_ptr<Expression>>& arguments, size_t count);
    
    static uint32_t readUint32At(const Chunk* chunk, int pos) {
        return (chunk->code[pos]) |
               (chunk->code[pos + 1] << 8) |
               (chunk->code[pos + 2] << 16) |
               (chunk->code[pos + 3] << 24);
    }

};

