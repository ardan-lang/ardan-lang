//
//  CodeGenerator.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "TurboCodeGenerator.hpp"

size_t TurboCodeGen::generate(const vector<unique_ptr<Statement>> &program) {
    cur = make_shared<TurboChunk>();
    cur->name = "BYTECODE";
    locals.clear();
    nextLocalSlot = 0;
    
    for (const auto &s : program) {
        s->accept(*this);
    }
    
    emit(TurboOpCode::Halt);
    disassembleChunk(cur.get(), cur->name);
    
    uint32_t idx = module_->addChunk(cur);
    module_->entryChunkIndex = idx;
    
    return idx;
}

R TurboCodeGen::visitExpression(ExpressionStatement* stmt) {
    stmt->expression->accept(*this);
    return 0;
}

R TurboCodeGen::visitBlock(BlockStatement* stmt) {
    beginScope();
    for (auto& s : stmt->body) {
        s->accept(*this);
        registerAllocator->reset();
    }
    endScope();
    return 0;
}

//// Allocate register for the variable
//int slot = allocRegister();
//
//// Handle initializer
//if (decl.init) {
//    int initReg = ...; // Compile initializer and get result register
//    emit(TurboOpCode::Move, slot, initReg); // Move to var slot if needed
//    // freeRegister(initReg) if not slot!
//} else {
//    emit(TurboOpCode::LoadConst, slot, emitConstant(Value::undefined()));
//}
//
//// Register as a local variable
//locals.push_back({decl.id, scopeDepth, false, slot});
//
//// (Do NOT freeRegister(slot) here)

R TurboCodeGen::create(string decl, uint32_t reg_slot, BindingKind kind) {
    
    TurboOpCode op;

    // decide local or global
    if (hasLocal(decl)) {
        uint32_t idx = getLocal(decl);
        
        if (locals[idx].kind == BindingKind::Const) {
            throw std::runtime_error("Assignment to constant variable.");
        }
        
        switch (kind) {
            case BindingKind::Var:
                op = TurboOpCode::CreateLocalVar;
                break;
            case BindingKind::Let:
                op = TurboOpCode::CreateLocalLet;
                break;
            case BindingKind::Const:
                op = TurboOpCode::CreateLocalConst;
                break;
            default:
                op = TurboOpCode::CreateLocalVar;
                break;
        }

        emit(op, idx, reg_slot);

    } else {
        
        //        if (classInfo.fields.count(decl)) {
        //
        //            // Rewrite: legs = one;  ⇒  this.legs = one;
        //            // Rewrite: legs;  ⇒  this.legs;
        //            emit(TurboOpCode::SetThisProperty);
        //            int nameIdx = emitConstant(Value::str(decl));
        //            emitUint32(nameIdx);
        //            return R();
        //        }
        
        //        int upvalue = resolveUpvalue(decl);
        //        if (upvalue != -1) {
        //            emit(TurboOpCode::SetUpvalue);
        //            emitUint32(upvalue);
        //            return R();
        //        }
        
        // top-level/global
        int nameIdx = emitConstant(Value::str(decl));
        switch (kind) {
            case BindingKind::Var:
                op = TurboOpCode::CreateGlobalVar;
                break;
            case BindingKind::Let:
                op = TurboOpCode::CreateGlobalLet;
                break;
            case BindingKind::Const:
                op = TurboOpCode::CreateGlobalConst;
                break;
            default:
                op = TurboOpCode::CreateGlobalVar;
                break;
        }

        emit(op, (uint32_t)nameIdx, reg_slot);
                
    }
    
    return true;
    
}

// moves data in reg_slot into local/global
R TurboCodeGen::store(string decl, uint32_t reg_slot) {
    
    // decide local or global
    if (hasLocal(decl)) {
        
        uint32_t idx = getLocal(decl);
        
        Local local = locals[idx];
        
        if (locals[idx].kind == BindingKind::Const) {
            throw std::runtime_error("Assignment to constant variable.");
        }
        
        if (local.kind == BindingKind::Var) {
            emit(TurboOpCode::StoreLocalVar, idx, reg_slot);
        } else if (local.kind == BindingKind::Let) {
            emit(TurboOpCode::StoreLocalLet, idx, reg_slot);
        }
        
    } else {
        
        //        if (classInfo.fields.count(decl)) {
        //
        //            // Rewrite: legs = one;  ⇒  this.legs = one;
        //            // Rewrite: legs;  ⇒  this.legs;
        //            emit(OpCode::SetThisProperty);
        //            int nameIdx = emitConstant(Value::str(decl));
        //            emitUint32(nameIdx);
        //            return R();
        //        }
        
        //        int upvalue = resolveUpvalue(decl);
        //        if (upvalue != -1) {
        //            emit(OpCode::SetUpvalue);
        //            emitUint32(upvalue);
        //            return R();
        //        }
        
        // top-level/global
        int nameIdx = emitConstant(Value::str(decl));
        Global global = globals[lookupGlobal(decl)];
        
        if (global.kind == BindingKind::Var) {
            emit(TurboOpCode::StoreGlobalVar, (uint32_t)nameIdx, reg_slot);
        } else if (global.kind == BindingKind::Let) {
            emit(TurboOpCode::StoreGlobalLet, (uint32_t)nameIdx, reg_slot);
        }
        
    }
    
    return true;
    
}

// moves data from local/global into reg_slot
R TurboCodeGen::load(string decl, uint32_t reg_slot) {

    // decide local or global
    if (hasLocal(decl)) {
        uint32_t idx = getLocal(decl);
        emit(TurboOpCode::LoadLocalVar, reg_slot, idx);
    } else {
        int nameIdx = emitConstant(Value::str(decl));
        emit(TurboOpCode::LoadGlobalVar, reg_slot, (uint32_t)nameIdx);
    }
    
    return true;
    
}

R TurboCodeGen::visitVariable(VariableStatement* stmt) {
    
    string kind = stmt->kind;
    
    for (auto& decl : stmt->declarations) {
        // Allocate a register slot for this variable
        int slot = allocRegister();
        
        // If initializer: compile it, store result in slot
        if (decl.init) {
            int initReg = get<int>(decl.init->accept(*this));
            emit(TurboOpCode::Move, slot, initReg); // move data inside initReg into register slot.
            freeRegister(initReg);
        } else {
            emit(TurboOpCode::LoadConst, slot, emitConstant(Value::undefined()));
        }
        
        declareLocal(decl.id);
        declareGlobal(decl.id, get_kind(kind));
        
        create(decl.id, slot, get_kind(kind));
        freeRegister(slot);
        // DO NOT freeRegister(slot) here!
        // It must stay alive until endScope() pops it.
    }
    
    return 0;
    
}

//R TurboCodeGen::_visitVariable(VariableStatement* stmt) {
//    for (auto& decl : stmt->declarations) {
//        Reg reg = allocRegister();
//        if (decl.init)
//            decl.init->accept(*this);
//        else
//            emit(TurboOpCode::LoadConst, reg, emitConstant(Value::undefined()));
//        declareLocal(decl.id);
//        freeRegister();
//    }
//    return 0;
//}

R TurboCodeGen::visitIf(IfStatement* stmt) {
    uint32_t cond = get<int>(stmt->test->accept(*this));
    int elseJump = emitJump(TurboOpCode::JumpIfFalse, cond);
    freeRegister(cond);
    stmt->consequent->accept(*this);
    int endJump = -1;
    if (stmt->alternate)
        endJump = emitJump(TurboOpCode::Jump);
    patchJump(elseJump, (int)cur->code.size());
    if (stmt->alternate) {
        stmt->alternate->accept(*this);
        patchJump(endJump, (int)cur->code.size());
    }
    return 0;
}

R TurboCodeGen::visitWhile(WhileStatement* stmt) {
    beginLoop();
    int loopStart = (int)cur->code.size();
    uint32_t cond = get<int>(stmt->test->accept(*this));
    int exitJump = emitJump(TurboOpCode::JumpIfFalse, cond);
    freeRegister(cond);
    stmt->body->accept(*this);
    emit(TurboOpCode::Jump, 0, loopStart - (int)cur->code.size() - 1); // backwards jump
    patchJump(exitJump, (int)cur->code.size());
    endLoop();
    return 0;
}

R TurboCodeGen::visitFor(ForStatement* stmt) {
    beginScope();
    if (stmt->init)
        stmt->init->accept(*this);
    beginLoop();
    int loopStart = (int)cur->code.size();
    if (stmt->test) {
        uint32_t testReg = allocRegister();
        stmt->test->accept(*this);
        int exitJump = emitJump(TurboOpCode::JumpIfFalse, testReg);
        //freeRegister();
        stmt->body->accept(*this);
        if (stmt->update)
            stmt->update->accept(*this);
        emit(TurboOpCode::Jump, 0, loopStart - (int)cur->code.size() - 1);
        patchJump(exitJump, (int)cur->code.size());
    } else {
        stmt->body->accept(*this);
        if (stmt->update)
            stmt->update->accept(*this);
        emit(TurboOpCode::Jump, 0, loopStart - (int)cur->code.size() - 1);
    }
    endLoop();
    endScope();
    return 0;
}

R TurboCodeGen::visitReturn(ReturnStatement* stmt) {
    uint32_t value = allocRegister();
    if (stmt->argument)
        stmt->argument->accept(*this);
    else
        emit(TurboOpCode::LoadConst, value, emitConstant(Value::undefined()));
    emit(TurboOpCode::Return, value);
    //freeRegister();
    return 0;
}

R TurboCodeGen::visitFunction(FunctionDeclaration* stmt) {
    // Omitted: See previous examples for function/lambda codegen
    return 0;
}

//R TurboCodeGen::visitBinary(BinaryExpression* expr) {
//    allocRegister();
//    uint32_t left =  expr->left->accept(*this);
//    uint32_t right = allocRegister();
//    expr->right->accept(*this);
//    uint32_t result = allocRegister();
//    //    OpCode op = getBinaryOp(expr->op); // implement op mapping
//    //    emit(op, result, left, right);
//    freeRegister();
//    freeRegister();
//    return result;
//}

TurboOpCode TurboCodeGen::getBinaryOp(const Token& op) {
    
    switch (op.type) {
            // --- Arithmetic ---
        case TokenType::ADD:                 return TurboOpCode::Add;
        case TokenType::MINUS:               return TurboOpCode::Subtract;
        case TokenType::MUL:                 return TurboOpCode::Multiply;
        case TokenType::DIV:                 return TurboOpCode::Divide;
        case TokenType::MODULI:              return TurboOpCode::Modulo;
        case TokenType::POWER:               return TurboOpCode::Power;
            
            // --- Comparisons ---
        case TokenType::VALUE_EQUAL:         return TurboOpCode::Equal;
        case TokenType::REFERENCE_EQUAL:     return TurboOpCode::StrictEqual;
        case TokenType::INEQUALITY:          return TurboOpCode::NotEqual;
        case TokenType::STRICT_INEQUALITY:   return TurboOpCode::StrictNotEqual;
        case TokenType::LESS_THAN:           return TurboOpCode::LessThan;
        case TokenType::LESS_THAN_EQUAL:     return TurboOpCode::LessThanOrEqual;
        case TokenType::GREATER_THAN:        return TurboOpCode::GreaterThan;
        case TokenType::GREATER_THAN_EQUAL:  return TurboOpCode::GreaterThanOrEqual;
            
            // --- Logical ---
        case TokenType::LOGICAL_AND:         return TurboOpCode::LogicalAnd;
        case TokenType::LOGICAL_OR:          return TurboOpCode::LogicalOr;
        case TokenType::NULLISH_COALESCING:  return TurboOpCode::NullishCoalescing;
            
            // --- Bitwise ---
        case TokenType::BITWISE_AND:         return TurboOpCode::BitAnd;
        case TokenType::BITWISE_OR:          return TurboOpCode::BitOr;
        case TokenType::BITWISE_XOR:         return TurboOpCode::BitXor;
        case TokenType::BITWISE_LEFT_SHIFT:  return TurboOpCode::ShiftLeft;
        case TokenType::BITWISE_RIGHT_SHIFT: return TurboOpCode::ShiftRight;
        case TokenType::UNSIGNED_RIGHT_SHIFT:return TurboOpCode::UnsignedShiftRight;
            
        default:
            throw std::runtime_error("Unknown binary operator in compiler: " + op.lexeme);
    }
    
}

R TurboCodeGen::visitBinary(BinaryExpression* expr) {
    switch (expr->op.type) {
        case TokenType::ASSIGN:
        case TokenType::ASSIGN_ADD:
        case TokenType::ASSIGN_MINUS:
        case TokenType::ASSIGN_MUL:
        case TokenType::ASSIGN_DIV:
        case TokenType::MODULI_ASSIGN:
        case TokenType::POWER_ASSIGN:
        case TokenType::BITWISE_LEFT_SHIFT_ASSIGN:
        case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN:
        case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN:
        case TokenType::BITWISE_AND_ASSIGN:
        case TokenType::BITWISE_OR_ASSIGN:
        case TokenType::BITWISE_XOR_ASSIGN:
        case TokenType::LOGICAL_AND_ASSIGN:
        case TokenType::LOGICAL_OR_ASSIGN:
        case TokenType::NULLISH_COALESCING_ASSIGN:
            emitAssignment(expr);
            return true;
        default:
            break;
    }
    
    // For all other operators, existing logic:
    int left = get<int>(expr->left->accept(*this));
    int right = get<int>(expr->right->accept(*this));
    int result = allocRegister();
    
    // Emit the appropriate operation instruction
    //    switch (expr->op.type) {
    //        // --- Arithmetic ---
    //        case TokenType::ADD:
    //            emit(TurboOpCode::Add); // stack: left, right -> left+right
    //            break;
    //        case TokenType::MINUS:
    //            emit(TurboOpCode::Subtract);
    //            break;
    //        case TokenType::MUL:
    //            emit(TurboOpCode::Multiply);
    //            break;
    //        case TokenType::DIV:
    //            emit(TurboOpCode::Divide);
    //            break;
    //        case TokenType::MODULI:
    //            emit(TurboOpCode::Modulo);
    //            break;
    //        case TokenType::POWER:
    //            emit(TurboOpCode::Power);
    //            break;
    //
    //        // --- Comparisons ---
    //        case TokenType::VALUE_EQUAL:
    //            emit(TurboOpCode::Equal);
    //            break;
    //        case TokenType::REFERENCE_EQUAL:
    //            emit(TurboOpCode::StrictEqual);
    //            break;
    //        case TokenType::INEQUALITY:
    //            emit(TurboOpCode::NotEqual);
    //            break;
    //        case TokenType::STRICT_INEQUALITY:
    //            emit(TurboOpCode::StrictNotEqual);
    //            break;
    //        case TokenType::LESS_THAN:
    //            emit(TurboOpCode::LessThan);
    //            break;
    //        case TokenType::LESS_THAN_EQUAL:
    //            emit(TurboOpCode::LessThanOrEqual);
    //            break;
    //        case TokenType::GREATER_THAN:
    //            emit(TurboOpCode::GreaterThan);
    //            break;
    //        case TokenType::GREATER_THAN_EQUAL:
    //            emit(TurboOpCode::GreaterThanOrEqual);
    //            break;
    //
    //        // --- Logical ---
    //        case TokenType::LOGICAL_AND:
    //            emit(TurboOpCode::LogicalAnd);
    //            break;
    //        case TokenType::LOGICAL_OR:
    //            emit(TurboOpCode::LogicalOr);
    //            break;
    //        case TokenType::NULLISH_COALESCING:
    //            emit(TurboOpCode::NullishCoalescing);
    //            break;
    //
    //        // --- Bitwise ---
    //        case TokenType::BITWISE_AND:
    //            emit(TurboOpCode::BitAnd);
    //            break;
    //        case TokenType::BITWISE_OR:
    //            emit(TurboOpCode::BitOr);
    //            break;
    //        case TokenType::BITWISE_XOR:
    //            emit(TurboOpCode::BitXor);
    //            break;
    //        case TokenType::BITWISE_LEFT_SHIFT:
    //            emit(TurboOpCode::ShiftLeft);
    //            break;
    //        case TokenType::BITWISE_RIGHT_SHIFT:
    //            emit(TurboOpCode::ShiftRight);
    //            break;
    //        case TokenType::UNSIGNED_RIGHT_SHIFT:
    //            emit(TurboOpCode::UnsignedShiftRight);
    //            break;
    //
    //        default:
    //            throw std::runtime_error("Unknown binary operator in compiler: " + expr->op.lexeme);
    //    }
    
    TurboOpCode op = getBinaryOp(expr->op);
    emit(op, result, left, right);

    freeRegister(left);
    freeRegister(right);

    return result;
    
}

//void TurboCodeGen::_emitAssignment(BinaryExpression* expr) {
//    auto left = expr->left.get();
//
//    // Plain assignment (=)
//    if (expr->op.type == TokenType::ASSIGN) {
//        if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
//            // Evaluate RHS first
//            uint32_t right = get<int>((expr->right->accept(*this));
//
//            // Assign to variable (local/global/class field/upvalue)
//            define(ident->token.lexeme);
//        }
//        else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
//            // Assignment to property (obj.prop = ...)
//
//            // Push object first
//            member->object->accept(*this);
//
//            if (member->computed) {
//                // Computed property: obj[expr] = rhs
//                member->property->accept(*this); // push property key
//
//                // Evaluate RHS
//                expr->right->accept(*this);
//
//                emit(OpCode::SetPropertyDynamic);
//            } else {
//
//                // Evaluate RHS
//                expr->right->accept(*this);
//
//                // Static property name (e.g. obj.x = rhs)
//                int nameIdx = emitConstant(Value::str(member->name.lexeme));
//                emit(OpCode::SetProperty);
//                emitUint32(nameIdx);
//            }
//        }
//        else {
//            throw std::runtime_error("Unsupported assignment target in CodeGen");
//        }
//
//        // Assignments as statements typically discard result
//        // emit(OpCode::Pop);
//
//        return;
//    }
//
//    // Compound assignment (+=, -=, etc.)
//    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
//        // Load current value
//        ident->accept(*this);
//    }
//    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
//        // Load current property value
//        member->object->accept(*this);
//        if (member->computed) {
//            member->property->accept(*this);
//            emit(OpCode::Dup2);
//            emit(OpCode::GetPropertyDynamic);
//        } else {
//            emit(OpCode::Dup); // [obj, obj]
//            int nameIdx = emitConstant(Value::str(member->name.lexeme));
//            emit(OpCode::GetProperty);
//            emitUint32(nameIdx);
//        }
//    }
//    else {
//        throw std::runtime_error("Unsupported assignment target in CodeGen");
//    }
//
//    // Evaluate RHS
//    expr->right->accept(*this);
//
//    // Apply compound operation
//    switch (expr->op.type) {
//        case TokenType::ASSIGN_ADD: emit(OpCode::Add); break;
//        case TokenType::ASSIGN_MINUS: emit(OpCode::Subtract); break;
//        case TokenType::ASSIGN_MUL: emit(OpCode::Multiply); break;
//        case TokenType::ASSIGN_DIV: emit(OpCode::Divide); break;
//        case TokenType::MODULI_ASSIGN: emit(OpCode::Modulo); break;
//        case TokenType::POWER_ASSIGN: emit(OpCode::Power); break;
//        case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: emit(OpCode::ShiftLeft); break;
//        case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: emit(OpCode::ShiftRight); break;
//        case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: emit(OpCode::UnsignedShiftRight); break;
//        case TokenType::BITWISE_AND_ASSIGN: emit(OpCode::BitAnd); break;
//        case TokenType::BITWISE_OR_ASSIGN: emit(OpCode::BitOr); break;
//        case TokenType::BITWISE_XOR_ASSIGN: emit(OpCode::BitXor); break;
//        case TokenType::LOGICAL_AND_ASSIGN: emit(OpCode::LogicalAnd); break;
//        case TokenType::LOGICAL_OR_ASSIGN: emit(OpCode::LogicalOr); break;
//        case TokenType::NULLISH_COALESCING_ASSIGN: emit(OpCode::NullishCoalescing); break;
//        default: throw std::runtime_error("Unknown compound assignment operator in emitAssignment");
//    }
//
//    // Store the result back
//    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
//        define(ident->token.lexeme);
//    }
//    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
//        if (member->computed) {
//            emit(OpCode::SetPropertyDynamic);
//        } else {
//            int nameIdx = emitConstant(Value::str(member->name.lexeme));
//            emit(OpCode::SetProperty);
//            emitUint32(nameIdx);
//        }
//    }
//
//}

void TurboCodeGen::emitAssignment(BinaryExpression* expr) {
    auto left = expr->left.get();
    
    // ----------- Plain assignment (=) -----------
    if (expr->op.type == TokenType::ASSIGN) {
        // Evaluate RHS into a fresh register
        // int rhsReg = allocRegister();
        int resultReg = get<int>(expr->right->accept(*this));
        
        // Assignment to a variable (identifier)
        if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
            // int destReg = lookupLocalSlot(ident->name);
            // Move/copy result into local/global slot
            // emit(TurboOpCode::Move, destReg, resultReg);
            store(ident->name, resultReg);
        }
        // Assignment to an object property
        else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
            // Evaluate object to reg
            int objReg = get<int>(member->object->accept(*this));
            
            if (member->computed) {
                
                int propReg = get<int>(member->property->accept(*this)); // Property key to reg
                
                // SetPropertyDynamic: objReg, propReg, valueReg
                emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, resultReg);
                freeRegister(propReg); // propReg
                
            } else {
                
                int nameIdx = emitConstant(Value::str(member->name.lexeme));
                // SetProperty: objReg, nameIdx, valueReg
                emit(TurboOpCode::SetProperty, objReg, nameIdx, resultReg);
                
            }
            
            freeRegister(objReg); // objReg
            
        }
        else {
            throw std::runtime_error("Unsupported assignment target in CodeGen");
        }
        
        // freeRegister(rhsReg); // rhsReg
        // freeRegister(resultReg);
        return;
    }
    
    // ----------- Compound assignments (+=, -=, etc.) -----------
    // Evaluate LHS (load current value)

    int lhsReg = allocRegister();
    int objReg = -1;
    int propReg = -1;
    int nameIdx = -1;
    
    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
        load(ident->name, lhsReg);
    }
    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
        
         objReg = get<int>(member->object->accept(*this));
        
         if (member->computed) {
             propReg = get<int>(member->property->accept(*this));
             // Get property value into lhsReg
             emit(TurboOpCode::GetPropertyDynamic, lhsReg, objReg, propReg);
         } else {
             nameIdx = emitConstant(Value::str(member->name.lexeme));
             emit(TurboOpCode::GetProperty, lhsReg, objReg, nameIdx);
         }
         // int valueReg = allocRegister(); // dest for the result
        
    } else {
        throw std::runtime_error("Unsupported assignment target in CodeGen");
    }
    
    // Evaluate RHS into a new register
    int rhsReg = get<int>(expr->right->accept(*this));
    
    // Compute result of compound operation
    int opResultReg = allocRegister(); // where to store result (reuse for identifiers)

    switch (expr->op.type) {
        case TokenType::ASSIGN_ADD: emit(TurboOpCode::Add, opResultReg, lhsReg, rhsReg); break;
        case TokenType::ASSIGN_MINUS: emit(TurboOpCode::Subtract, opResultReg, lhsReg, rhsReg); break;
        case TokenType::ASSIGN_MUL: emit(TurboOpCode::Multiply, opResultReg, lhsReg, rhsReg); break;
        case TokenType::ASSIGN_DIV: emit(TurboOpCode::Divide, opResultReg, lhsReg, rhsReg); break;
        case TokenType::MODULI_ASSIGN: emit(TurboOpCode::Modulo, opResultReg, lhsReg, rhsReg); break;
        case TokenType::POWER_ASSIGN: emit(TurboOpCode::Power, opResultReg, lhsReg, rhsReg); break;
        case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: emit(TurboOpCode::ShiftLeft, opResultReg, lhsReg, rhsReg); break;
        case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: emit(TurboOpCode::ShiftRight, opResultReg, lhsReg, rhsReg); break;
        case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: emit(TurboOpCode::UnsignedShiftRight, opResultReg, lhsReg, rhsReg); break;
        case TokenType::BITWISE_AND_ASSIGN: emit(TurboOpCode::BitAnd, opResultReg, lhsReg, rhsReg); break;
        case TokenType::BITWISE_OR_ASSIGN: emit(TurboOpCode::BitOr, opResultReg, lhsReg, rhsReg); break;
        case TokenType::BITWISE_XOR_ASSIGN: emit(TurboOpCode::BitXor, opResultReg, lhsReg, rhsReg); break;
        case TokenType::LOGICAL_AND_ASSIGN: emit(TurboOpCode::LogicalAnd, opResultReg, lhsReg, rhsReg); break;
        case TokenType::LOGICAL_OR_ASSIGN: emit(TurboOpCode::LogicalOr, opResultReg, lhsReg, rhsReg); break;
        case TokenType::NULLISH_COALESCING_ASSIGN: emit(TurboOpCode::NullishCoalescing, opResultReg, lhsReg, rhsReg); break;
        default: throw std::runtime_error("Unknown compound assignment operator in emitAssignment");
    }
    
    // Store back to LHS
    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
        // emit(TurboOpCode::Move, lhsReg, opResultReg);
        store(ident->name, opResultReg);
    }
    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
         if (member->computed) {
             emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, opResultReg);
             freeRegister(propReg); // propReg
         } else {
             emit(TurboOpCode::SetProperty, objReg, nameIdx, opResultReg);
         }
        freeRegister(objReg); // objReg
        //freeRegister(); // valueReg if not reused
    }
    
    freeRegister(rhsReg); // rhsReg
    freeRegister(lhsReg); // lhsReg if not reused
}

R TurboCodeGen::visitLiteral(LiteralExpression* expr) {
    int reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(expr->token.lexeme));
    return reg;
}

R TurboCodeGen::visitNumericLiteral(NumericLiteral* expr) {
    int reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(toValue(expr->value))); // load from constant into reg register
    return reg;
}

R TurboCodeGen::visitStringLiteral(StringLiteral* expr) {
    int reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(Value(expr->text)));
    return reg;
}

R TurboCodeGen::visitIdentifier(IdentifierExpression* expr) {
    // return lookupLocalSlot(expr->name);
    int reg = allocRegister();
    load(expr->name, reg);
    return reg;
}

//R TurboCodeGen::visitCall(CallExpression* expr) {
//    
//    int funcReg = get<int>(expr->callee->accept(*this));
//    
//    vector<int> argRegs;
//    for (auto& arg : expr->arguments) {
//        int r = get<int>(arg->accept(*this));
//        argRegs.push_back(r);
//    }
//    
//    int result = argRegs.size() > 0 ? argRegs.at(0) : allocRegister();
//
//    emit(TurboOpCode::Call, result, funcReg, argRegs.at(argRegs.size() - 1));
//
//    for (auto r : argRegs) {
//        freeRegister(r);
//    }
//    
//    freeRegister(funcReg); // funcReg
//
//    return result;
//    
//}

R TurboCodeGen::visitCall(CallExpression* expr) {

    int funcReg = get<int>(expr->callee->accept(*this));
    auto funcGuard = makeRegGuard(funcReg, *this);

    vector<int> argRegs;
    argRegs.reserve(expr->arguments.size());
    for (auto& arg : expr->arguments) {
        int r = get<int>(arg->accept(*this));
        argRegs.push_back(r);
    }

    int resultReg = allocRegister();
    auto resultGuard = makeRegGuard(resultReg, *this, /*autoFree=*/false);

    for (int argReg : argRegs) {
        emit(TurboOpCode::PushArg, argReg);
    }

    emit(TurboOpCode::Call, resultReg, funcReg, static_cast<int>(argRegs.size()));

    for (int argReg : argRegs) {
        freeRegister(argReg);
    }

    funcGuard.release();
    return resultReg;
    
}

//R TurboCodeGen::visitMember(MemberExpression* expr) {
//    
//    int obj = get<int>(expr->object->accept(*this));
//    int result = allocRegister();
//    
//    if (expr->computed) {
//        int propReg = get<int>(expr->property->accept(*this));
//
//        emit(TurboOpCode::GetPropertyDynamic,
//             result,
//             obj,
//             propReg);
//        
//        freeRegister(propReg);
//        
//    } else {
//        emit(TurboOpCode::GetProperty, result, obj, emitConstant(expr->name.lexeme));
//    }
//    
//    freeRegister(obj);
//    
//    return result;
//    
//}

R TurboCodeGen::visitMember(MemberExpression* expr) {
    // Evaluate the object expression
    int objectReg = get<int>(expr->object->accept(*this));
    auto objectGuard = makeRegGuard(objectReg, *this);

    // Allocate target register for result
    int targetReg = allocRegister();
    auto targetGuard = makeRegGuard(targetReg, *this, /*autoFree=*/false);

    if (expr->computed) {
        // obj[prop]
        int propertyReg = get<int>(expr->property->accept(*this));
        auto propertyGuard = makeRegGuard(propertyReg, *this);

        emit(TurboOpCode::GetPropertyDynamic, targetReg, objectReg, propertyReg);
    } else {
        // obj.name
        int nameConst = emitConstant(expr->name.lexeme);
        emit(TurboOpCode::GetProperty, targetReg, objectReg, nameConst);
    }

    // Free object register after use
    objectGuard.release();
    return targetReg;
}

R TurboCodeGen::visitNew(NewExpression* expr) {
    return 0;
}

R TurboCodeGen::visitArray(ArrayLiteralExpression* expr) {
    int arr = allocRegister();
    emit(TurboOpCode::NewArray, arr);
    for (auto& el : expr->elements) {
        int val = get<int>(el->accept(*this));
        emit(TurboOpCode::ArrayPush, arr, val);
        freeRegister(val);
    }
    return arr;
}

R TurboCodeGen::visitObject(ObjectLiteralExpression* expr) {
    
    int obj = allocRegister();
    emit(TurboOpCode::NewObject, obj);
    
    for (auto& prop : expr->props) {
        int val = get<int>(prop.second->accept(*this));
        emit(TurboOpCode::SetProperty, obj, emitConstant(prop.first.lexeme), val);
        freeRegister(val);
    }
    
    return obj;
    
}

R TurboCodeGen::visitConditional(ConditionalExpression* expr) {
    uint32_t cond = get<int>(expr->test->accept(*this));
    int elseJump = emitJump(TurboOpCode::JumpIfFalse, cond);
    freeRegister(cond);
    expr->consequent->accept(*this);
    int endJump = emitJump(TurboOpCode::Jump);
    patchJump(elseJump, (int)cur->code.size());
    expr->alternate->accept(*this);
    patchJump(endJump, (int)cur->code.size());
    return 0;
}

TurboOpCode TurboCodeGen::getUnaryOp(const Token& op) {
    switch (op.type) {
        //case TokenType::NOT:         return TurboOpCode::Not;
        case TokenType::MINUS:       return TurboOpCode::Negate;
        //case TokenType::PLUS:        return TurboOpCode::NoOp; // Or handle separately if no-op
        //case TokenType::BITWISE_NOT: return TurboOpCode::BitwiseNot;
        default:
            throw std::runtime_error("Unknown unary operator in compiler: " + op.lexeme);
    }
}

R TurboCodeGen::visitUnary(UnaryExpression* expr) {
    uint32_t operand = get<int>(expr->right->accept(*this));
    uint32_t result = allocRegister();
    TurboOpCode op = getUnaryOp(expr->op); // implement unary op mapping
    emit(op, result, operand);
    freeRegister(operand);
    return result;
}

R TurboCodeGen::visitArrowFunction(ArrowFunction* expr) {
    // Omitted for brevity; see previous examples on nested codegen
    return 0;
}

R TurboCodeGen::visitFunctionExpression(FunctionExpression* expr) {
    // Omitted for brevity; similar pattern as above
    return 0;
}

R TurboCodeGen::visitTemplateLiteral(TemplateLiteral* expr) {
    // Concatenate all pieces
    uint32_t reg = allocRegister();
        emit(TurboOpCode::LoadConst, reg, emitConstant(Value("")));
    for (size_t i = 0; i < expr->quasis.size(); ++i) {
        uint32_t strReg = allocRegister();
        emit(TurboOpCode::LoadConst, strReg, emitConstant(expr->quasis[i]->text));
        emit(TurboOpCode::Add, reg, reg, strReg);
        //freeRegister();
        if (i < expr->expressions.size()) {
            uint32_t exprReg = allocRegister();
            expr->expressions[i]->accept(*this);
            emit(TurboOpCode::Add, reg, reg, exprReg);
            //freeRegister();
        }
    }
    return reg;
}

R TurboCodeGen::visitImportDeclaration(ImportDeclaration* stmt) {
    uint32_t pathReg = allocRegister();
    //    emit(TurboOpCode::LoadConst, pathReg, emitConstant(Value(stmt->path.lexeme)));
    // Call import builtin, etc.
    //freeRegister();
    return 0;
}

R TurboCodeGen::visitAssignment(AssignmentExpression* expr) {
    uint32_t valueReg = allocRegister();
    expr->right->accept(*this);
    // handle assign to variable or property
    //freeRegister();
    return valueReg;
}

R TurboCodeGen::visitLogical(LogicalExpression* expr) {
    expr->left->accept(*this);
    int endJump = emitJump(TurboOpCode::JumpIfFalse);
    expr->right->accept(*this);
    patchJump(endJump, (int)cur->code.size());
    return 0;
}

R TurboCodeGen::visitThis(ThisExpression* expr) {
    return lookupLocalSlot("this");
}

R TurboCodeGen::visitSuper(SuperExpression* expr) {
    // Omitted; implement according to your semantics
    return 0;
}

R TurboCodeGen::visitProperty(PropertyExpression* expr) {
    // Omitted; see Member visitor
    return 0;
}

R TurboCodeGen::visitSequence(SequenceExpression* expr) {
    for (auto& ex : expr->expressions) {
        ex->accept(*this);
    }
    return 0;
}

R TurboCodeGen::visitUpdate(UpdateExpression* expr) {
    uint32_t reg = allocRegister();
    expr->argument->accept(*this);
    TurboOpCode op = expr->op.type == TokenType::INCREMENT ? TurboOpCode::Add : TurboOpCode::Subtract;
    emit(op, reg, reg, emitConstant(Value(1)));
    //freeRegister();
    return reg;
}

R TurboCodeGen::visitFalseKeyword(FalseKeyword* expr) {
    uint32_t reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(Value(false)));
    return reg;
}

R TurboCodeGen::visitTrueKeyword(TrueKeyword* expr) {
    uint32_t reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(Value(true)));
    return reg;
}

R TurboCodeGen::visitPublicKeyword(PublicKeyword*) { return 0; }
R TurboCodeGen::visitPrivateKeyword(PrivateKeyword*) { return 0; }
R TurboCodeGen::visitProtectedKeyword(ProtectedKeyword*) { return 0; }
R TurboCodeGen::visitStaticKeyword(StaticKeyword*) { return 0; }
R TurboCodeGen::visitRestParameter(RestParameter*) { return 0; }

R TurboCodeGen::visitClassExpression(ClassExpression*) { return 0; }

R TurboCodeGen::visitNullKeyword(NullKeyword*) {
    uint32_t reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(Value::nullVal()));
    return reg;
}

R TurboCodeGen::visitUndefinedKeyword(UndefinedKeyword*) {
    uint32_t reg = allocRegister();
    emit(TurboOpCode::LoadConst, reg, emitConstant(Value::undefined()));
    return reg;
}

R TurboCodeGen::visitAwaitExpression(AwaitExpression*) { return 0; }

R TurboCodeGen::visitBreak(BreakStatement*) {
    // Usually: emit a jump to end of current loop
    return 0;
}

R TurboCodeGen::visitContinue(ContinueStatement*) {
    // Usually: emit a jump to loop start
    return 0;
}

R TurboCodeGen::visitThrow(ThrowStatement* stmt) {
    // stmt->argument->accept(*this);
    emit(TurboOpCode::Throw);
    return 0;
}

R TurboCodeGen::visitEmpty(EmptyStatement*) { return 0; }

R TurboCodeGen::visitClass(ClassDeclaration*) { return 0; }

R TurboCodeGen::visitMethodDefinition(MethodDefinition*) { return 0; }

R TurboCodeGen::visitDoWhile(DoWhileStatement*) { return 0; }

R TurboCodeGen::visitSwitchCase(SwitchCase*) { return 0; }

R TurboCodeGen::visitSwitch(SwitchStatement*) { return 0; }

R TurboCodeGen::visitCatch(CatchClause*) { return 0; }

R TurboCodeGen::visitTry(TryStatement*) { return 0; }

R TurboCodeGen::visitForIn(ForInStatement*) { return 0; }

R TurboCodeGen::visitForOf(ForOfStatement*) { return 0; }

void TurboCodeGen::emit(TurboOpCode op, int a, int b = 0, int c = 0) {
    if (!cur) throw std::runtime_error("No active chunk for code generation.");
    cur->code.push_back({op, (uint8_t)a, (uint8_t)b, (uint8_t)c});
}

void TurboCodeGen::emit(TurboOpCode op, int a) {
    emit(op, a, 0, 0);
}

void TurboCodeGen::emit(TurboOpCode op, int a, int b) {
    emit(op, a, b, 0);
}

void TurboCodeGen::emit(TurboOpCode op) {
    emit(op, 0, 0, 0);
}

int TurboCodeGen::allocRegister() { return registerAllocator->alloc(); }

void TurboCodeGen::freeRegister(uint32_t slot) {
    registerAllocator->free(slot);
}

// Jump helpers
int TurboCodeGen::emitJump(TurboOpCode op, int cond_reg = 0) {
    // Reserve space, return jump location to patch
    // Implementation depends on code buffer layout
    // Emit the jump instruction with placeholder offset
    // We'll patch .b later to hold the actual jump offset
    Instruction instr(op, (uint8_t)cond_reg, 0, 0); // b is offset placeholder
    cur->code.push_back(instr);
    return (int)cur->code.size() - 1; // Return index of this jump instruction
}

void TurboCodeGen::patchJump(int jumpPos, int target) {
    int offset = target - (jumpPos + 1); // Offset is relative to the next instruction
    cur->code[jumpPos].b = (uint8_t)offset; // If you need bigger jumps, use a different format
}

// Lookup helpers
int TurboCodeGen::lookupLocalSlot(const std::string& name) {
    return paramSlot(name);
}

void TurboCodeGen::beginScope() {
    scopeDepth++;
}

void TurboCodeGen::endScope() {
    //     Pop locals declared in this scope
    //    while (!locals.empty() && locals.back().depth == scopeDepth) {
    //        if (locals.back().isCaptured) {
    //            // Local captured by closure → close it
    //            emit(TurboOpCode::OP_CLOSE_UPVALUE);
    //        } else {
    //            // Normal local → just pop
    //            emit(TurboOpCode::OP_POP);
    //        }
    //        locals.pop_back();
    //    }
    scopeDepth--;
}

int TurboCodeGen::resolveLocal(const string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].name == name) {
            return locals[i].slot_index; // return slot index
        }
    }
    return -1;
}

int TurboCodeGen::paramSlot(const string& name) {
    return resolveLocal(name); //locals[name].slot_index;
}

int TurboCodeGen::emitConstant(const Value& v) {
    cur->constants.push_back(v);
    return (int)cur->constants.size() - 1;
}

bool TurboCodeGen::hasLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return true;
    }
    return false;
}

uint32_t TurboCodeGen::getLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return locals[i].slot_index;
    }
    throw std::runtime_error("Local not found: " + name);
}

//int TurboCodeGen::declareLocal(const std::string& name) {
//    // Check for duplicate declaration in current scope (optional)
//    for (int i = (int)locals.size() - 1; i >= 0; --i) {
//        if (locals[i].depth < scopeDepth)
//            break; // only check current scope
//        if (locals[i].name == name)
//            throw std::runtime_error("Variable already declared in this scope: " + name);
//    }
//    Local local;
//    local.name = name;
//    local.depth = scopeDepth;
//    local.isCaptured = false;
//    local.slot_index = nextLocalSlot++;
//    locals.push_back(local);
//    return local.slot_index;
//}

void TurboCodeGen::declareLocal(const string& name) {
    if (scopeDepth == 0) return; // globals aren’t locals

    // prevent shadowing in same scope
    for (int i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].depth != -1 && locals[i].depth < scopeDepth) break;
        if (locals[i].name == name) {
            throw runtime_error("Variable already declared in this scope");
        }
    }

    Local local { name, scopeDepth, false, (uint32_t)locals.size() };
    locals.push_back(local);
    
}

void TurboCodeGen::declareGlobal(const string& name, BindingKind kind) {
    
    if (scopeDepth > 0) return; // locals aren’t globals

    for (int i = (int)globals.size() - 1; i >= 0; i--) {
        if (globals[i].name == name) {
            throw runtime_error("Variable already declared in this scope");
        }
    }

    Global global { name, kind };
    globals.push_back(global);
    
}

int TurboCodeGen::lookupGlobal(const string& name) {
    for (int i = (int)globals.size() - 1; i >= 0; i--) {
        if (globals[i].name == name) {
            return i;
        }
    }
    return -1;
}

void TurboCodeGen::endLoop() {}

int TurboCodeGen::emitJump(TurboOpCode op) {
    return 0;
}

void TurboCodeGen::beginLoop() {
    
}

TurboCodeGen::BindingKind TurboCodeGen::get_kind(string kind) {
    if (kind == "CONST") {
        return BindingKind::Const;
    }
    
    if (kind == "LET") {
        return BindingKind::Let;
    }
    
    return BindingKind::Var;
}

size_t TurboCodeGen::disassembleInstruction(const TurboChunk* chunk, size_t offset) {
    if (offset >= chunk->code.size()) return offset + 1;

    const Instruction& instr = chunk->code[offset];
    std::cout << std::setw(4) << offset << " ";

    // Print opcode name
    std::string opName;
    switch (instr.op) {
        case TurboOpCode::Add: opName = "Add"; break;
        case TurboOpCode::Subtract: opName = "Subtract"; break;
        case TurboOpCode::Multiply: opName = "Multiply"; break;
        case TurboOpCode::Divide: opName = "Divide"; break;
        case TurboOpCode::Modulo: opName = "Modulo"; break;
        case TurboOpCode::Power: opName = "Power"; break;
        case TurboOpCode::Equal: opName = "Equal"; break;
        case TurboOpCode::StrictEqual: opName = "StrictEqual"; break;
        case TurboOpCode::NotEqual: opName = "NotEqual"; break;
        case TurboOpCode::StrictNotEqual: opName = "StrictNotEqual"; break;
        case TurboOpCode::LessThan: opName = "LessThan"; break;
        case TurboOpCode::LessThanOrEqual: opName = "LessThanOrEqual"; break;
        case TurboOpCode::GreaterThan: opName = "GreaterThan"; break;
        case TurboOpCode::GreaterThanOrEqual: opName = "GreaterThanOrEqual"; break;
        case TurboOpCode::LogicalAnd: opName = "LogicalAnd"; break;
        case TurboOpCode::LogicalOr: opName = "LogicalOr"; break;
        case TurboOpCode::NullishCoalescing: opName = "NullishCoalescing"; break;
        case TurboOpCode::BitAnd: opName = "BitAnd"; break;
        case TurboOpCode::BitOr: opName = "BitOr"; break;
        case TurboOpCode::BitXor: opName = "BitXor"; break;
        case TurboOpCode::ShiftLeft: opName = "ShiftLeft"; break;
        case TurboOpCode::ShiftRight: opName = "ShiftRight"; break;
        case TurboOpCode::UnsignedShiftRight: opName = "UnsignedShiftRight"; break;
        case TurboOpCode::LoadConst: opName = "LoadConst"; break;
        case TurboOpCode::Move: opName = "Move"; break;
        case TurboOpCode::StoreLocalLet: opName = "StoreLocalLet"; break;
        case TurboOpCode::StoreLocalVar: opName = "StoreLocalVar"; break;
        case TurboOpCode::StoreGlobalVar: opName = "StoreGlobalVar"; break;
        case TurboOpCode::StoreGlobalLet: opName = "StoreGlobalLet"; break;
        case TurboOpCode::LoadLocalVar: opName = "LoadLocalVar"; break;
        case TurboOpCode::LoadGlobalVar: opName = "LoadGlobalVar"; break;
        case TurboOpCode::CreateLocalVar: opName = "CreateLocalVar"; break;
        case TurboOpCode::CreateLocalLet: opName = "CreateLocalLet"; break;
        case TurboOpCode::CreateLocalConst: opName = "CreateLocalConst"; break;
        case TurboOpCode::CreateGlobalVar: opName = "CreateGlobalVar"; break;
        case TurboOpCode::CreateGlobalLet: opName = "CreateGlobalLet"; break;
        case TurboOpCode::CreateGlobalConst: opName = "CreateGlobalConst"; break;
        case TurboOpCode::JumpIfFalse: opName = "JumpIfFalse"; break;
        case TurboOpCode::Jump: opName = "Jump"; break;
        case TurboOpCode::Return: opName = "Return"; break;
        case TurboOpCode::Call: opName = "Call"; break;
        case TurboOpCode::Halt: opName = "Halt"; break;
        case TurboOpCode::Throw: opName = "Throw"; break;
        // Add more opcodes as needed
        default: opName = "Unknown"; break;
    }

    std::cout << std::left << std::setw(20) << opName;

    // Print operands
    std::cout << " a: " << (int)instr.a
              << " b: " << (int)instr.b
              << " c: " << (int)instr.c;

    // For LoadConst, print constant value
    if (instr.op == TurboOpCode::LoadConst && instr.b < chunk->constants.size()) {
        std::cout << " [const: " << chunk->constants[instr.b].toString() << "]";
    }

    std::cout << std::endl;
    return offset + 1;
}

void TurboCodeGen::disassembleChunk(const TurboChunk* chunk, const std::string& name) {
    std::cout << "== " << name << " ==\n";
    for (size_t offset = 0; offset < chunk->code.size();) {
        offset = disassembleInstruction(chunk, offset);
    }
}

