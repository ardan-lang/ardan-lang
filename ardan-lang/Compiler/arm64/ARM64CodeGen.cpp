//
//  ARM64CodeGen.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 30/10/2025.
//

#include <sys/mman.h>
#include <cstring>

#include "ARM64CodeGen.hpp"

#define FP 29

//void emitSimpleAddFunc(ARM64Emitter& emitter) {
//    emitter.mov_reg_imm(0, 3); // MOV X0, #3
//    emitter.mov_reg_imm(1, 5); // MOV X1, #5
//    emitter.add(0, 0, 1);      // ADD X0, X0, X1
//    emitter.ret();             // RET
//}

ARM64CodeGen::ARM64CodeGen() {}

void ARM64CodeGen::run() {
    auto dataSection = emitter.getDataSection();

    // Allocate and copy data
    void* data = mmap(nullptr, dataSection.size(),
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

    // Allocate executable memory
    auto code = emitter.getCode();
    size_t pageSize = sysconf(_SC_PAGESIZE);
    size_t codeSize = ((code.size() + pageSize - 1) / pageSize) * pageSize;

    void* exec_mem = mmap(nullptr, codeSize,
                          PROT_READ | PROT_WRITE ,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT, -1, 0);
    
    if (exec_mem == MAP_FAILED) {
        perror("mmap");
        return;
    }
    
    // cout << code.data() << endl;

    memcpy(exec_mem, code.data(), code.size());

    // Flush instruction cache
    //__builtin___clear_cache((char*)exec_mem, (char*)exec_mem + code.size());

    if (mprotect(exec_mem, codeSize, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
    }

    // Call code
    auto func = reinterpret_cast<int(*)()>(exec_mem);

    std::memcpy(data, dataSection.data(), dataSection.size());
    register void* dataAddr asm("x0") = data;
    asm volatile("" :: "r"(dataAddr));

    int result = func();

    std::cout << result << std::endl;
    uint64_t* globalBase = (uint64_t*)data;
    cout << globalBase[0] << endl;

    munmap(exec_mem, codeSize);
    munmap(data, dataSection.size());
}

size_t ARM64CodeGen::generate(const vector<unique_ptr<Statement>> &program) {
    
    for (const auto &s : program) {
        s->accept(*this);
    }
    
    emitter.ret();
        
    disassemble();
    
    run();
    
    return 0;
}

// --- STATEMENTS ---

R ARM64CodeGen::visitExpression(ExpressionStatement* stmt) {
//    R value = stmt->expr->accept(this);
//    // Result is in a register, typically discard or use for side-effects
//    regAlloc.free(value.reg);
//    return {};
return true;
}

R ARM64CodeGen::visitBlock(BlockStatement* stmt) {
//    for (auto& s : stmt->statements) s->accept(this);
//    return {};
return true;
}

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
            emitter.addData(decl.id);
            
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

R ARM64CodeGen::visitIf(IfStatement* stmt) {
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
return true;
}

R ARM64CodeGen::visitWhile(WhileStatement* stmt) {
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
return true;
}

R ARM64CodeGen::visitFor(ForStatement* stmt) {
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
return true;
}

R ARM64CodeGen::visitReturn(ReturnStatement* stmt) {
//    R val = stmt->argument ? stmt->argument->accept(this) : R{-1, 0};
//    if (val.reg != -1)
//        emitter.mov_reg_reg(0, val.reg); // x0 = return value
//    emitter.ret();
//    return {};
    return true;
}

R ARM64CodeGen::visitFunction(FunctionDeclaration* stmt) {
//    // New frame, prologue: push FP, LR, allocate locals, etc.
//    emitter.push_fp_lr();
//    // Compile body
//    stmt->body->accept(this);
//    // Epilogue: pop FP, LR, ret
//    emitter.pop_fp_lr();
//    emitter.ret();
//    return {};
return true;
}

// --- EXPRESSIONS ---

R ARM64CodeGen::visitBinary(BinaryExpression* expr) {
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
    return true;
}

R ARM64CodeGen::visitLiteral(LiteralExpression* expr) {
    int reg = regAlloc.alloc();
    // emitter.mov_reg_imm(reg, expr->asInt());
    return reg;
}

R ARM64CodeGen::visitNumericLiteral(NumericLiteral* expr) {
    int reg = regAlloc.alloc();
    emitter.mov_reg_imm(reg, (toValue(expr->value).numberValue));
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
    // emitter.ldr(reg, FP, stackFrame.getLocal(expr->name));
    // emitter.ldr(reg, FP, offset_of(expr->name));
    
    if (stackFrame.hasLocal(expr->name)) {
        emitter.ldr(reg, FP, stackFrame.getLocal(expr->name));
    } else {
        // it is in global
        int globalAddr = symbolTable.getGlobal(expr->name);
        // emitter.str_global(reg, globalAddr); // Store reg to global memory address
        emitter.ldr_global(reg, globalAddr);
    }

    return reg;
}

R ARM64CodeGen::visitCall(CallExpression* expr) {
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
    return true;
}

R ARM64CodeGen::visitMember(MemberExpression* expr) {
//    // Address computation, usually for objects/structs/arrays
//    int objReg = expr->object->accept(this).reg;
//    int result = regAlloc.alloc();
//    // emitter.ldr(result, objReg, offset_of(expr->property));
//    regAlloc.free(objReg);
//    return {result, 0};
    return true;
}

R ARM64CodeGen::visitUnary(UnaryExpression* expr) { return true; }
R ARM64CodeGen::visitArrowFunction(ArrowFunction* expr) { return true; }
R ARM64CodeGen::visitFunctionExpression(FunctionExpression* expr) { return true; }
R ARM64CodeGen::visitTemplateLiteral(TemplateLiteral* expr) { return true; }
R ARM64CodeGen::visitImportDeclaration(ImportDeclaration* stmt) { return true; }

R ARM64CodeGen::visitAssignment(AssignmentExpression* expr) { return true; }
R ARM64CodeGen::visitLogical(LogicalExpression* expr) { return true; }
R ARM64CodeGen::visitThis(ThisExpression* expr) { return true; }
R ARM64CodeGen::visitSuper(SuperExpression* expr) { return true; }
R ARM64CodeGen::visitProperty(PropertyExpression* expr) { return true; }
R ARM64CodeGen::visitSequence(SequenceExpression* expr) { return true; }
R ARM64CodeGen::visitUpdate(UpdateExpression* expr) { return true; }
R ARM64CodeGen::visitFalseKeyword(FalseKeyword* expr) { return true; }
R ARM64CodeGen::visitTrueKeyword(TrueKeyword* expr) { return true; }
R ARM64CodeGen::visitPublicKeyword(PublicKeyword* expr) { return true; }
R ARM64CodeGen::visitPrivateKeyword(PrivateKeyword* expr) { return true; }
R ARM64CodeGen::visitProtectedKeyword(ProtectedKeyword* expr) { return true; }
R ARM64CodeGen::visitStaticKeyword(StaticKeyword* expr) { return true; }
R ARM64CodeGen::visitRestParameter(RestParameter* expr) { return true; }
R ARM64CodeGen::visitClassExpression(ClassExpression* expr) { return true; }
R ARM64CodeGen::visitNullKeyword(NullKeyword* expr) { return true; }
R ARM64CodeGen::visitUndefinedKeyword(UndefinedKeyword* expr) { return true; }
R ARM64CodeGen::visitAwaitExpression(AwaitExpression* expr) { return true; }
R ARM64CodeGen::visitUIExpression(UIViewExpression* visitor) { return true; }

R ARM64CodeGen::visitBreak(BreakStatement* stmt) { return true; }
R ARM64CodeGen::visitContinue(ContinueStatement* stmt) { return true; }
R ARM64CodeGen::visitThrow(ThrowStatement* stmt) { return true; }
R ARM64CodeGen::visitEmpty(EmptyStatement* stmt) { return true; }
R ARM64CodeGen::visitClass(ClassDeclaration* stmt) { return true; }
R ARM64CodeGen::visitMethodDefinition(MethodDefinition* stmt) { return true; }
R ARM64CodeGen::visitDoWhile(DoWhileStatement* stmt) { return true; }
R ARM64CodeGen::visitSwitchCase(SwitchCase* stmt) { return true; }
R ARM64CodeGen::visitSwitch(SwitchStatement* stmt) { return true; }
R ARM64CodeGen::visitCatch(CatchClause* stmt) { return true; }
R ARM64CodeGen::visitTry(TryStatement* stmt) { return true; }
R ARM64CodeGen::visitForIn(ForInStatement* stmt) { return true; }
R ARM64CodeGen::visitForOf(ForOfStatement* stmt) { return true; }
R ARM64CodeGen::visitEnumDeclaration(EnumDeclaration* stmt) { return true; }
R ARM64CodeGen::visitInterfaceDeclaration(InterfaceDeclaration* stmt) { return true; }

R ARM64CodeGen::visitYieldExpression(YieldExpression* visitor) {
    return true;
}

R ARM64CodeGen::visitSpreadExpression(SpreadExpression* visitor) {
    return true;
}

R ARM64CodeGen::visitNew(NewExpression* expr) {
    return true;
}

R ARM64CodeGen::visitArray(ArrayLiteralExpression* expr) {
    return true;
}

R ARM64CodeGen::visitObject(ObjectLiteralExpression* expr) {
    return true;
}

R ARM64CodeGen::visitConditional(ConditionalExpression* expr) {
    return true;
}

void ARM64CodeGen::disassemble() {
    // --- Disassemble ARM64 code section ---
    printf("== ARM64 Code ==\n");
    const auto& code = emitter.getCode();
//    size_t offset = 0;
//    while (offset + 4 <= code.size()) {
//        // Read 4 bytes (little-endian)
//        uint32_t instr = (uint32_t)code[offset]
//                       | ((uint32_t)code[offset+1] << 8)
//                       | ((uint32_t)code[offset+2] << 16)
//                       | ((uint32_t)code[offset+3] << 24);
//        printf("%04zx: %08x    ", offset, instr);
//
//        // Decode known patterns (add more as you support more ops)
//        if ((instr & 0xFFC00000) == 0xD2800000) {
//            // MOVZ Xd, #imm16
//            uint8_t reg = instr & 0x1F;
//            uint16_t imm = (instr >> 5) & 0xFFFF;
//            printf("movz x%d, #%u", reg, imm);
//        } else if ((instr & 0xFF2003E0) == 0x8B000000) {
//            // ADD Xd, Xn, Xm
//            uint8_t dst = instr & 0x1F;
//            uint8_t src1 = (instr >> 5) & 0x1F;
//            uint8_t src2 = (instr >> 16) & 0x1F;
//            printf("add x%d, x%d, x%d", dst, src1, src2);
//        } else if ((instr & 0xFF2003E0) == 0xCB000000) {
//            // SUB Xd, Xn, Xm
//            uint8_t dst = instr & 0x1F;
//            uint8_t src1 = (instr >> 5) & 0x1F;
//            uint8_t src2 = (instr >> 16) & 0x1F;
//            printf("sub x%d, x%d, x%d", dst, src1, src2);
//        } else if (instr == 0xD65F03C0) {
//            // RET
//            printf("ret");
//        } else if ((instr & 0xFFC00000) == 0xF9000000) {
//            // STR Xn, [Xm, #imm12]
//            uint8_t reg = instr & 0x1F;
//            uint8_t base = (instr >> 5) & 0x1F;
//            uint32_t offset12 = (instr >> 10) & 0x7FF;
//            printf("str x%d, [x%d, #%u]", reg, base, offset12 * 8);
//        } else if ((instr & 0xFFC00000) == 0xF9400000) {
//            // LDR Xn, [Xm, #imm12]
//            uint8_t reg = instr & 0x1F;
//            uint8_t base = (instr >> 5) & 0x1F;
//            uint32_t offset12 = (instr >> 10) & 0x7FF;
//            printf("ldr x%d, [x%d, #%u]", reg, base, offset12 * 8);
//        } else {
//            // Unknown, print raw
//            printf("unknown");
//        }
//        printf("\n");
//        offset += 4;
//    }
    // --- Disassemble data section ---
    const auto& data = emitter.getDataSection();
    printf("-- Data Section --\n");
    size_t dpos = 0;
    while (dpos < data.size()) {
        // Try to print as a null-terminated ASCII string if possible
        if (isprint(data[dpos]) && data[dpos] != 0) {
            size_t start = dpos;
            while (dpos < data.size() && isprint(data[dpos]) && data[dpos] != 0) dpos++;
            printf("[%04zx] \"%.*s\"\n", start, (int)(dpos - start), (const char*)&data[start]);
            // Skip over null terminator if present
            if (dpos < data.size() && data[dpos] == 0) dpos++;
        } else {
            // Print as hex
            printf("[%04zx] %02x\n", dpos, data[dpos]);
            dpos++;
        }
    }
}

//void ARM64CodeGen::disassemble()  {
//
//    const auto& code = emitter.getCode();
//
//    std::vector<std::string> out;
//    size_t n = code.size();
//    auto read32 = [&](size_t pos)->uint32_t {
//        if (pos + 4 > n) throw std::runtime_error("read32 out of range");
//        // little-endian recompose
//        uint32_t b0 = code[pos];
//        uint32_t b1 = code[pos+1];
//        uint32_t b2 = code[pos+2];
//        uint32_t b3 = code[pos+3];
//        return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
//    };
//    
//    auto regName = [](int r)->std::string {
//        std::ostringstream s;
//        s << "X" << r;
//        return s.str();
//    };
//    
//    auto signExtend = [](uint32_t val, int bits)->int32_t {
//        // sign-extend 'bits' size value in uint32_t to int32_t
//        uint32_t m = 1u << (bits - 1);
//        return (val ^ m) - m;
//    };
//    
//    for (size_t pos = 0; pos + 4 <= n; pos += 4) {
//        uint32_t inst = read32(pos);
//        std::ostringstream line;
//        line << std::setw(4) << std::setfill('0') << std::hex << pos << ": ";
//        line << std::setw(8) << std::setfill('0') << std::hex << inst << std::dec << "  ";
//        
//        // Match known patterns produced by emitter
//        // MOVZ (MOV immediate)
//        if ((inst & 0xFFC00000) == 0xD2800000) {
//            uint32_t imm = (inst >> 5) & 0xFFFF;
//            uint32_t rd  = inst & 0x1F;
//            line << "MOVZ " << regName(rd) << ", #" << imm;
//        }
//        // ADD (register)
//        else if ((inst & 0xFF000000) == 0x8B000000) {
//            uint32_t rd = inst & 0x1F;
//            uint32_t rn = (inst >> 5) & 0x1F;
//            uint32_t rm = (inst >> 16) & 0x1F;
//            line << "ADD " << regName(rd) << ", " << regName(rn) << ", " << regName(rm);
//        }
//        // SUB (register)
//        else if ((inst & 0xFF000000) == 0xCB000000) {
//            uint32_t rd = inst & 0x1F;
//            uint32_t rn = (inst >> 5) & 0x1F;
//            uint32_t rm = (inst >> 16) & 0x1F;
//            line << "SUB " << regName(rd) << ", " << regName(rn) << ", " << regName(rm);
//        }
//        // ORR / Move (the pattern used by mov_reg_reg)
//        else if ((inst & 0xFF000000) == 0xAA000000) {
//            uint32_t rd = inst & 0x1F;
//            uint32_t rn = (inst >> 5) & 0x1F;
//            uint32_t rm = (inst >> 16) & 0x1F;
//            // If operand uses XZR in one operand we can nicely print MOV
//            if (rn == 31) {
//                line << "MOV " << regName(rd) << ", " << regName(rm);
//            } else {
//                line << "ORR " << regName(rd) << ", " << regName(rn) << ", " << regName(rm);
//            }
//        }
//        // STR scaled (your encoding uses scaled by 8)
//        else if ((inst & 0xFF800000) == 0xF9000000) {
//            uint32_t rt = inst & 0x1F;
//            uint32_t rn = (inst >> 5) & 0x1F;
//            uint32_t imm11 = (inst >> 10) & 0x7FF;
//            uint32_t imm = imm11 * 8;
//            line << "STR " << regName(rt) << ", [" << regName(rn) << ", #" << imm << "]";
//        }
//        // LDR scaled
//        else if ((inst & 0xFF800000) == 0xF9400000) {
//            uint32_t rt = inst & 0x1F;
//            uint32_t rn = (inst >> 5) & 0x1F;
//            uint32_t imm11 = (inst >> 10) & 0x7FF;
//            uint32_t imm = imm11 * 8;
//            line << "LDR " << regName(rt) << ", [" << regName(rn) << ", #" << imm << "]";
//        }
//        // CMP immediate pattern used by emitter (SUBS XZR, Xn, #imm)
//        else if ((inst & 0xFFE0001F) == 0xF100001F) {
//            uint32_t imm12 = (inst >> 10) & 0xFFF;
//            uint32_t rn = (inst >> 5) & 0x1F;
//            line << "CMP " << regName(rn) << ", #" << imm12;
//        }
//        // BEQ (conditional branch)
//        else if ((inst & 0xFF000000) == 0x54000000) {
//            uint32_t imm19 = (inst >> 5) & 0x7FFFF; // 19 bits
//            int32_t s = signExtend(imm19, 19);
//            int32_t byteOffset = s * 4;
//            line << "BEQ " << (byteOffset >= 0 ? "+" : "") << byteOffset << " (PC relative)";
//        }
//        // B (unconditional)
//        else if ((inst & 0x7C000000) == 0x14000000) {
//            uint32_t imm26 = inst & 0x03FFFFFF;
//            int32_t s = signExtend(imm26, 26);
//            int32_t byteOffset = s * 4;
//            line << "B " << (byteOffset >= 0 ? "+" : "") << byteOffset << " (PC relative)";
//        }
//        // BLR (branch to register)
//        else if ((inst & 0xFFFFFE00) == 0xD63F0000) {
//            uint32_t rn = (inst >> 5) & 0x1F;
//            line << "BLR " << regName(rn);
//        }
//        // RET
//        else if (inst == 0xD65F03C0) {
//            line << "RET LR";
//        }
//        // STP FP, LR, [SP, #-16]!  (function prologue pattern emitted by push_fp_lr)
//        else if (inst == 0xA9BF7BF0) {
//            line << "STP FP, LR, [SP, #-16]!";
//        }
//        // MOV FP, SP (typical sequence after STP in push_fp_lr)
//        else if (inst == 0x910003FD) {
//            line << "MOV FP, SP";
//        }
//        // LDP FP, LR, [SP], #16  (pop_fp_lr)
//        else if (inst == 0xA8C17BF0) {
//            line << "LDP FP, LR, [SP], #16";
//        }
//        // Fallback: unknown
//        else {
//            line << "UNKN";
//        }
//        
//        out.push_back(line.str());
//    }
//    
//    auto listing = out;
//    for (auto &ln : listing) {
//        std::cout << ln << '\n';
//    }
//    
//}
