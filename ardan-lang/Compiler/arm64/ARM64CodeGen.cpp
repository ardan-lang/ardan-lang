//
//  ARM64CodeGen.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 30/10/2025.
//

#include "ARM64CodeGen.hpp"

#define FP 29

//void emitSimpleAddFunc(ARM64Emitter& emitter) {
//    emitter.mov_reg_imm(0, 3); // MOV X0, #3
//    emitter.mov_reg_imm(1, 5); // MOV X1, #5
//    emitter.add(0, 0, 1);      // ADD X0, X0, X1
//    emitter.ret();             // RET
//}

// --- STATEMENTS ---

//R ARM64CodeGen::visitExpression(ExpressionStatement* stmt) {
//    R value = stmt->expr->accept(this);
//    // Result is in a register, typically discard or use for side-effects
//    regAlloc.free(value.reg);
//    return {};
//}

//R ARM64CodeGen::visitBlock(BlockStatement* stmt) {
//    for (auto& s : stmt->statements) s->accept(this);
//    return {};
//}

//R ARM64CodeGen::visitVariable(VariableStatement* stmt) {
//    int reg = regAlloc.alloc();
//    // R value = stmt->init ? stmt->init->accept(this) : R{reg, 0};
//    // Store to stack offset for this local
//    // emitter.str(reg, FP, local_offset);
//    // ... bookkeeping for scope
//    return {};
//}

R ARM64CodeGen::visitVariable(VariableStatement* stmt) {
    for (auto& decl : stmt->declarations) {
        // Determine if this is a local or global variable
        bool isGlobal = (scopeDepth == 0);

        // Allocate a register for the variable's value
        int reg = regAlloc.alloc();

        // Evaluate initializer or default value
        if (decl.init) {
            int initReg = get<int>(decl.init->accept(*this));
            emitter.mov_reg_reg(reg, initReg);
            regAlloc.free(initReg);
        } else {
            // For uninitialized, store zero (undefined)
            emitter.mov_reg_imm(reg, 0);
        }

        if (isGlobal) {
            // Global variable: store in a static/global storage area
            int globalAddr = symbolTable.addGlobal(decl.id);
            emitter.str_global(reg, globalAddr); // Store reg to global memory address
        } else {
            // Local variable: store in stack frame, tracked by offset
            int localOffset = stackFrame.addLocal(decl.id);
            emitter.str(reg, FP, localOffset); // Store reg to [FP, #offset]
        }

        regAlloc.free(reg);

        // Optionally: update symbol table for debugging or further codegen use
        // symbolTable.bind(decl.id, isGlobal ? SymbolKind::Global : SymbolKind::Local, localOffset or globalAddr);
    }

    return {};
}

//R ARM64CodeGen::visitIf(IfStatement* stmt) {
//    int condReg = stmt->test->accept(this).reg;
//    int endLabel = emitter.genLabel();
//    emitter.cmp_imm(condReg, 0);
//    emitter.b_eq(endLabel); // If false, jump to end
//    stmt->consequent->accept(this);
//    if (stmt->alternate) {
//        int elseLabel = emitter.genLabel();
//        emitter.b(elseLabel); // skip else
//        emitter.setLabel(endLabel);
//        stmt->alternate->accept(this);
//        emitter.setLabel(elseLabel);
//    } else {
//        emitter.setLabel(endLabel);
//    }
//    regAlloc.free(condReg);
//    return {};
//}

//R ARM64CodeGen::visitWhile(WhileStatement* stmt) {
//    int condLabel = emitter.genLabel();
//    int bodyLabel = emitter.genLabel();
//    emitter.setLabel(condLabel);
//    int condReg = stmt->test->accept(this).reg;
//    emitter.cmp_imm(condReg, 0);
//    emitter.b_eq(bodyLabel); // exit if false
//    stmt->body->accept(this);
//    emitter.b(condLabel);
//    emitter.setLabel(bodyLabel);
//    regAlloc.free(condReg);
//    return {};
//}

//R ARM64CodeGen::visitFor(ForStatement* stmt) {
//    // Pseudo: for (init; cond; step) body
//    if (stmt->init) stmt->init->accept(this);
//    int condLabel = emitter.genLabel();
//    int endLabel = emitter.genLabel();
//    emitter.setLabel(condLabel);
//    int condReg = stmt->test->accept(this).reg;
//    emitter.cmp_imm(condReg, 0);
//    emitter.b_eq(endLabel);
//    stmt->body->accept(*this);
//    if (stmt->update) stmt->update->accept(*this);
//    emitter.b(condLabel);
//    emitter.setLabel(endLabel);
//    regAlloc.free(condReg);
//    return {};
//}

//R ARM64CodeGen::visitReturn(ReturnStatement* stmt) {
//    R val = stmt->argument ? stmt->argument->accept(this) : R{-1, 0};
//    if (val.reg != -1)
//        emitter.mov_reg_reg(0, val.reg); // x0 = return value
//    emitter.ret();
//    return {};
//}

//R ARM64CodeGen::visitFunction(FunctionDeclaration* stmt) {
//    // New frame, prologue: push FP, LR, allocate locals, etc.
//    emitter.push_fp_lr();
//    // Compile body
//    stmt->body->accept(this);
//    // Epilogue: pop FP, LR, ret
//    emitter.pop_fp_lr();
//    emitter.ret();
//    return {};
//}

// --- EXPRESSIONS ---

//R ARM64CodeGen::visitBinary(BinaryExpression* expr) {
//    // Example: a + b
//    R left = expr->left->accept(this);
//    R right = expr->right->accept(this);
//    int result = regAlloc.alloc();
//    if (expr->op == "+")
//        emitter.add(result, left.reg, right.reg);
//    else if (expr->op == "-")
//        emitter.sub(result, left.reg, right.reg);
//    // ...other ops
//    regAlloc.free(left.reg);
//    regAlloc.free(right.reg);
//    return {result, 0};
//}

R ARM64CodeGen::visitLiteral(LiteralExpression* expr) {
    int reg = regAlloc.alloc();
    // emitter.mov_reg_imm(reg, expr->asInt());
    return reg;
}

R ARM64CodeGen::visitNumericLiteral(NumericLiteral* expr) {
    int reg = regAlloc.alloc();
    emitter.mov_reg_imm(reg, get<int>(expr->value));
    // return {reg, 0};
    return reg;
}

R ARM64CodeGen::visitStringLiteral(StringLiteral* expr) {
    // Place string in data section, emit pointer to reg
    int reg = regAlloc.alloc();
    int addr = emitter.addData(expr->text);
    emitter.mov_reg_imm(reg, addr); // Address of string
    // return {reg, 0};
    return reg;
}

R ARM64CodeGen::visitIdentifier(IdentifierExpression* expr) {
    // Look up address of local/global, load to reg
    int reg = regAlloc.alloc();
    emitter.ldr(reg, FP, stackFrame.getLocal(expr->name));
    // emitter.ldr(reg, FP, offset_of(expr->name));

    return reg;
}

//R ARM64CodeGen::visitCall(CallExpression* expr) {
//    // Evaluate arguments (left-to-right)
//    std::vector<int> argRegs;
//    for (auto* a : expr->arguments) argRegs.push_back(a->accept(this).reg);
//    // Call: move args to x0-x7, bl function address
//    for (size_t i = 0; i < argRegs.size(); ++i)
//        emitter.mov_reg_reg(i, argRegs[i]);
//    // Get address of callee
//    int fnReg = expr->callee->accept(this).reg;
//    emitter.blr(fnReg);
//    // Result in x0
//    int result = regAlloc.alloc();
//    emitter.mov_reg_reg(result, 0);
//    // Free arg regs
//    for (auto r : argRegs) regAlloc.free(r);
//    regAlloc.free(fnReg);
//    return {result, 0};
//}

//R ARM64CodeGen::visitMember(MemberExpression* expr) {
//    // Address computation, usually for objects/structs/arrays
//    int objReg = expr->object->accept(this).reg;
//    int result = regAlloc.alloc();
//    // emitter.ldr(result, objReg, offset_of(expr->property));
//    regAlloc.free(objReg);
//    return {result, 0};
//}

