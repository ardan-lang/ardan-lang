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

using namespace std;

class TypeDetector {
    
public:
    bool isNumber(const Expression* expr) const {
        auto number_expr = dynamic_cast<const NumericLiteral*>(expr);
        if (number_expr) {
            return true;
        }
        return false;
    }
    
    bool isString(const Expression* expr) const {
        auto string_expr = dynamic_cast<const StringLiteral*>(expr);
        if (string_expr) {
            return true;
        }
        return false;
    }
    
    bool isNull(const Expression* expr) const {
        auto null_expr = dynamic_cast<const NullKeyword*>(expr);
        if (null_expr) {
            return true;
        }
        return false;
    }

    bool isUndefined(const Expression* expr) const {
        auto undefined_expr = dynamic_cast<const UndefinedKeyword*>(expr);
        if (undefined_expr) {
            return true;
        }
        return false;
    }

    bool isBoolean(const Expression* expr) const {
        auto true_expr = dynamic_cast<const TrueKeyword*>(expr);
        auto false_expr = dynamic_cast<const FalseKeyword*>(expr);

        if (true_expr || false_expr) {
            return true;
        }
        return false;
    }

};

class ARM64CodeGen : public ExpressionVisitor, public StatementVisitor {
    
    class ARM64RegisterAllocator {
    private:
        vector<int> freeRegs;
    public:
        ARM64RegisterAllocator() {
            // ARM64 has x0-x30, but reserve x0-x7 for ABI/args/returns. Use x9-x15 for temporaries.
            for (int r = 9; r <= 15; ++r)
                freeRegs.push_back(r);
        }
        int alloc() {
            if (freeRegs.empty())
                throw std::runtime_error("Out of free registers!");
            int reg = freeRegs.back();
            freeRegs.pop_back();
            return reg;
        }
        void free(int reg) {
            freeRegs.push_back(reg);
        }
    };
    
    class SymbolTable {
    private:
        unordered_map<string, int> globals;
        int nextGlobalAddr = 0;
    public:
        int addGlobal(const string& name) {
            if (globals.count(name) == 0)
                globals[name] = nextGlobalAddr++;
            return globals[name];
        }

        int addGlobal(const string& name, const int addr) {
            if (globals.count(name) == 0)
                globals[name] = addr;
            return globals[name];
        }

        // For debugging or lookups
        int getGlobal(const std::string& name) const {
            auto it = globals.find(name);
            if (it == globals.end())
                throw std::runtime_error("No such global: " + name);
            return it->second;
        }
        
        bool hasGlobal(string name) {
            auto it = globals.find(name);
            if (it == globals.end())
                return false;
            return true;
        }

    };
    
    class StackFrame {
    private:
        unordered_map<std::string, int> locals;
        int nextOffset = -8; // Assume negative offset, 8 bytes per local (64 bits)
    public:
        int addLocal(const string& name) {
            if (locals.count(name) == 0) {
                locals[name] = nextOffset;
                nextOffset -= 8;
            }
            return locals[name];
        }
        int getLocal(const std::string& name) const {
            auto it = locals.find(name);
            if (it == locals.end())
                throw std::runtime_error("No such local: " + name);
            return it->second;
        }
        
        bool hasLocal(string name) {
            auto it = locals.find(name);
            if (it == locals.end())
                return false;
            return true;
        }
        
    };
    
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

public:
    ARM64Emitter emitter;
    ARM64RegisterAllocator regAlloc;
    vector<Local> locals;
    vector<Global> globals;
    int scopeDepth = 0;
    SymbolTable symbolTable;
    StackFrame stackFrame;
    TypeDetector detector;
    
    size_t generate(const vector<unique_ptr<Statement>> &program);
    void disassemble();
    void dump(int result, int* data);
    void run();

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

};

#endif /* ARM64CodeGen_hpp */
