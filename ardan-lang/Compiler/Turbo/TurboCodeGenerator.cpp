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

R TurboCodeGen::visitIf(IfStatement* stmt) {

    beginScope();

    uint32_t cond = get<int>(stmt->test->accept(*this));
    int elseJump = emitJump(TurboOpCode::JumpIfFalse, cond);
    freeRegister(cond);
        
    stmt->consequent->accept(*this);
    
    int finalEndJump = emitJump(TurboOpCode::Jump);
    
    int endJump = -1;
    if (stmt->alternate)
        endJump = emitJump(TurboOpCode::Jump);
    
    patchJump(elseJump);
    
    if (stmt->alternate) {
        scopeDepth--;
        stmt->alternate->accept(*this);
        patchJump(endJump, (int)cur->code.size());
    }
    
    patchSingleJump(finalEndJump);
    
    endScope();
    
    return 0;
}

// TODO: fix to work
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
    beginLoop();
    loopStack.back().loopStart = (int)cur->size();

    if (stmt->init)
        (stmt->init->accept(*this));
    else throw runtime_error("For loop must have an initializer.");
    
    int loopStart = (int)cur->code.size();
    
    if (stmt->test) {
        
        uint32_t testReg = get<int>(stmt->test->accept(*this));
        int exitJump = emitJump(TurboOpCode::JumpIfFalse, testReg);
        freeRegister(testReg);

        stmt->body->accept(*this);

        if (stmt->update) {
            stmt->update->accept(*this);
        }
        
        // move up to test
        emitLoop(loopStart);

        patchJump(exitJump, (int)cur->code.size());
        
    } else {
        stmt->body->accept(*this);
        if (stmt->update)
            stmt->update->accept(*this);
        emit(TurboOpCode::Loop, (int)cur->code.size() - loopStart);
    }
    
    endLoop();
    endScope();
    return 0;
}

R TurboCodeGen::visitReturn(ReturnStatement* stmt) {
    
    uint32_t value = allocRegister();
    
    if (stmt->argument)
        value = get<int>(stmt->argument->accept(*this));
    else
        emit(TurboOpCode::LoadConst, value, emitConstant(Value::undefined()));
    
    emit(TurboOpCode::Return, value);
    
    freeRegister(value);
    
    return 0;
    
}

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
        
    TurboOpCode op = getBinaryOp(expr->op);
    emit(op, result, left, right);

    freeRegister(left);
    freeRegister(right);

    return result;
    
}

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
        
        freeRegister(propertyReg);
        
    } else {
        // obj.name
        int nameConst = emitConstant(Value::str(expr->name.lexeme));
        emit(TurboOpCode::GetProperty, targetReg, objectReg, nameConst);
        
    }

    // Free object register after use
    objectGuard.release();
    
    return targetReg;
    
}

R TurboCodeGen::visitNew(NewExpression* expr) {
    
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->callee.get())) {
                
        int reg = get<int>(ident->accept(*this));
        
        emit(TurboOpCode::CreateInstance, reg);

        vector<int> argRegs;
        argRegs.resize(expr->arguments.size());
        // emit args count
        for (auto& arg : expr->arguments) {
            argRegs.push_back(get<int>(arg->accept(*this)));
        }
        
        // TODO: set the arg start to -1 to denote no args
        emit(TurboOpCode::InvokeConstructor,
             reg,
             argRegs.size() == 0 ? 0 : argRegs[0],
             (int)argRegs.size());

        // free args regs
        for (auto argReg : argRegs) {
            freeRegister(argReg);
        }
        
        // freeRegister(reg);
        return reg;

    }
    
    throw runtime_error("New must be called on an exting class.");
    
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

// --x, ++x, !x, -x
R TurboCodeGen::visitUnary(UnaryExpression* expr) {
    
    // For prefix unary ops that target identifiers or members, we need special handling.
    if (expr->op.type == TokenType::INCREMENT || expr->op.type == TokenType::DECREMENT) {
        // ++x or --x
        if (auto ident = dynamic_cast<IdentifierExpression*>(expr->right.get())) {
            
            // load ident
            int lhsReg = allocRegister();
            load(ident->name, lhsReg);
            
            int rhsReg = allocRegister();
            emit(TurboOpCode::LoadConst, rhsReg, emitConstant(Value(1)));
            
            int opResultReg = allocRegister();
            // TurboOpCode::Add, opResultReg, lhsReg, rhsReg
            emit(expr->op.type == TokenType::INCREMENT ? TurboOpCode::Add : TurboOpCode::Subtract,
                 opResultReg,
                 lhsReg,
                 rhsReg);
            
            freeRegister(lhsReg);
            freeRegister(rhsReg);

            store(ident->name, opResultReg);
                        
            return opResultReg;
            
        }

        if (auto member_expr = dynamic_cast<MemberExpression*>(expr->right.get())) {
            
            int lhsReg = get<int>(member_expr->accept(*this));
            int rhsReg = allocRegister();
            emit(TurboOpCode::LoadConst, rhsReg, emitConstant(Value(1)));
            int opResultReg = allocRegister();
            // TurboOpCode::Add, opResultReg, lhsReg, rhsReg
            emit(expr->op.type == TokenType::INCREMENT ? TurboOpCode::Add : TurboOpCode::Subtract,
                 opResultReg,
                 lhsReg,
                 rhsReg);

            freeRegister(lhsReg);
            freeRegister(rhsReg);

            // Evaluate object to reg
            int objReg = get<int>(member_expr->object->accept(*this));

            if (member_expr->computed) {
                
                int propReg = get<int>(member_expr->property->accept(*this)); // Property key to reg
                
                // SetPropertyDynamic: objReg, propReg, valueReg
                emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, opResultReg);
                freeRegister(propReg); // propReg
                
            } else {
                
                int nameIdx = emitConstant(Value::str(member_expr->name.lexeme));
                // SetProperty: objReg, nameIdx, valueReg
                emit(TurboOpCode::SetProperty, objReg, nameIdx, opResultReg);
                
            }
            
            freeRegister(objReg); // objReg

            return opResultReg;

        }

        throw std::runtime_error("Unsupported unary increment/decrement target");
    }

    // non-targeted unary ops (prefix) evaluate their operand first
    int reg = get<int>(expr->right->accept(*this));
    
    switch (expr->op.type) {
        case TokenType::LOGICAL_NOT: emit(TurboOpCode::LogicalNot, reg); break;
        case TokenType::MINUS: emit(TurboOpCode::Negate, reg); break;
        case TokenType::BITWISE_NOT: emit(TurboOpCode::LogicalNot, reg); break;
        case TokenType::ADD: emit(TurboOpCode::Positive, reg); break;
        default: throw std::runtime_error("Unsupported unary op in CodeGen");
    }
    
    return reg;

}

R TurboCodeGen::visitUpdate(UpdateExpression* expr) {
    
    int lhsReg = get<int>(expr->argument->accept(*this));
    //    TurboOpCode op = expr->op.type == TokenType::INCREMENT ? TurboOpCode::Add : TurboOpCode::Subtract;
    //    emit(op, reg, reg, emitConstant(Value(1)));
    //    //freeRegister();
    //    return reg;
    
    int returnReg = -1;
    
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
        
        int rhsReg = allocRegister();
        emit(TurboOpCode::LoadConst, rhsReg, emitConstant(Value(1)));
        
        int opResultReg = allocRegister();
        // TurboOpCode::Add, opResultReg, lhsReg, rhsReg
        emit(expr->op.type == TokenType::INCREMENT ? TurboOpCode::Add : TurboOpCode::Subtract,
             opResultReg,
             lhsReg,
             rhsReg);
        
        freeRegister(lhsReg);
        freeRegister(rhsReg);
        
        store(ident->name, opResultReg);
        
        returnReg = opResultReg;
        
    }
    else if (auto member = dynamic_cast<MemberExpression*>(expr->argument.get())) {
        
        int rhsReg = allocRegister();
        emit(TurboOpCode::LoadConst, rhsReg, emitConstant(Value(1)));
        int opResultReg = allocRegister();
        // TurboOpCode::Add, opResultReg, lhsReg, rhsReg
        emit(expr->op.type == TokenType::INCREMENT ? TurboOpCode::Add : TurboOpCode::Subtract,
             opResultReg,
             lhsReg,
             rhsReg);
        
        freeRegister(lhsReg);
        freeRegister(rhsReg);
        
        // Evaluate object to reg
        int objReg = get<int>(member->object->accept(*this));
        
        if (member->computed) {
            
            int propReg = get<int>(member->property->accept(*this)); // Property key to reg
            
            // SetPropertyDynamic: objReg, propReg, valueReg
            emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, opResultReg);
            freeRegister(propReg); // propReg
            
        } else {
            
            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            // SetProperty: objReg, nameIdx, valueReg
            emit(TurboOpCode::SetProperty, objReg, nameIdx, opResultReg);
            
        }
        
        freeRegister(objReg); // objReg
        
        returnReg = opResultReg;
    }
    
    return returnReg;
    
}

R TurboCodeGen::visitArrowFunction(ArrowFunction* expr) {
    
    // Create a nested CodeGen for the function body
    TurboCodeGen nested(module_);
    nested.cur = make_shared<TurboChunk>();
    nested.enclosing = this;
    nested.beginScope();

    vector<string> paramNames;
    vector<ParameterInfo> parameterInfos;

    // Collect parameter info (name, hasDefault, defaultExpr, isRest)
    if (expr->parameters) {
        if (SequenceExpression* seq = dynamic_cast<SequenceExpression*>(expr->parameters.get())) {
            for (auto& p : seq->expressions) {
                if (auto* rest = dynamic_cast<RestParameter*>(p.get())) {
                    // ...rest
                    //if (auto* ident = dynamic_cast<IdentifierExpression*>(rest->argument.get())) {
                    paramNames.push_back(rest->token.lexeme);
                    parameterInfos
                        .push_back(ParameterInfo{rest->token.lexeme, false, nullptr, true});
                    //}
                } else if (auto* assign = dynamic_cast<BinaryExpression*>(p.get())) {
                    // b = 90 or c = b
                    if (auto* ident = dynamic_cast<IdentifierExpression*>(assign->left.get())) {
                        paramNames.push_back(ident->name);
                        parameterInfos.push_back(ParameterInfo{ident->name, true, assign->right.get(), false});
                    }
                } else if (auto* ident = dynamic_cast<IdentifierExpression*>(p.get())) {
                    // Simple arg
                    paramNames.push_back(ident->name);
                    parameterInfos.push_back(ParameterInfo{ident->name, false, nullptr, false});
                }
            }
        } else if (auto* ident = dynamic_cast<IdentifierExpression*>(expr->parameters.get())) {
            paramNames.push_back(ident->name);
            parameterInfos.push_back(ParameterInfo{ident->name, false, nullptr, false});
        }
    }

    // Allocate local slots for parameters
    nested.resetLocalsForFunction((uint32_t)paramNames.size(), paramNames);

    // Emit parameter initialization logic
    for (size_t i = 0; i < parameterInfos.size(); ++i) {
        const auto& info = parameterInfos[i];
        // For rest parameter
        if (info.isRest) {
            
            // collect rest arguments as array: arguments.slice(i)
            
            int arg_array_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArguments, arg_array_reg); // Push arguments array
            
            int i_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, i_reg, nested.emitConstant(Value::number(i))); // Push i
            nested.emit(TurboOpCode::Slice, arg_array_reg, i_reg); // arguments.slice(i)
            
            nested.freeRegister(i_reg);
            
            nested.store(info.name, arg_array_reg);
            
            nested.freeRegister(arg_array_reg);
            
            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            
            int store_reg = nested.allocRegister();
     
            int args_len_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArgumentsLength, args_len_reg);
            
            int index_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, index_reg, nested.emitConstant(Value::number(i)));
            
            nested.emit(TurboOpCode::GreaterThan, args_len_reg, index_reg);
            int useArg = nested.emitJump(TurboOpCode::JumpIfFalse, args_len_reg); // false means to use default value
            
            // Use argument
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));
            nested.emit(TurboOpCode::LoadArgument, reg);
            nested.emit(TurboOpCode::Move, store_reg, reg);
            
            int setLocalJump = nested.emitJump(TurboOpCode::Jump);
            
            // Use default
            nested.patchJump(useArg);
            
            // Evaluate default expression (can reference previous params!)
            int default_expr_reg = get<int>(info.defaultExpr->accept(nested));
            nested.emit(TurboOpCode::Move, store_reg, default_expr_reg);
            
            // Set local either way
            nested.patchSingleJump(setLocalJump);

            nested.store(info.name, store_reg);
            
            nested.freeRegister(store_reg);
            nested.freeRegister(default_expr_reg);
            nested.freeRegister(reg);
            nested.freeRegister(args_len_reg);
            nested.freeRegister(index_reg);
            
        } else {
            
            // Direct: assign argument i to local slot
            
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));

            nested.emit(TurboOpCode::LoadArgument, reg);
            
            nested.store(info.name, reg);
            
            nested.freeRegister(reg);
            
        }
    }

    // Compile function body
    if (expr->exprBody) {
        expr->exprBody->accept(nested);
        nested.emit(TurboOpCode::Return);
        // we must return the value of the expr
    } else if (expr->stmtBody) {
        
        expr->stmtBody->accept(nested);
        
        // TODO: we need to check if return is the last statement.
        bool is_return_avaialble = false;
        if (BlockStatement* block = dynamic_cast<BlockStatement*>(expr->stmtBody.get())) {
            for (auto& body : block->body) {
                if (auto return_stmt = dynamic_cast<ReturnStatement*>(body.get())) {
                    is_return_avaialble = true;
                }
            }
        }
        if (!is_return_avaialble) {
            int reg = nested.allocRegister();
            int ud = nested.emitConstant(Value::undefined());
            nested.emit(TurboOpCode::LoadConst, reg, ud);

            nested.emit(TurboOpCode::Return, reg);
        }
        
    }

    // Register chunk & emit as constant
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();

    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = "<arrow>";
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

    int closureChunkIndexReg = allocRegister();
    emit(TurboOpCode::LoadConst, closureChunkIndexReg, emitConstant(Value(ci)));
    
    emit(TurboOpCode::CreateClosure, closureChunkIndexReg);
    
//    ClosureInfo closure_info = {};
//    closure_info.ci = ci;
//    closure_info.upvalues = nested.upvalues;

    for (auto& uv : nested.upvalues) {
        
        // emitUint8(uv.isLocal ? 1 : 0);
        // emitUint8(uv.index);
        
        int isLocalReg = allocRegister();
        emit(TurboOpCode::LoadConst, isLocalReg, emitConstant(Value(uv.isLocal ? 1 : 0)));
        emit(TurboOpCode::SetClosureIsLocal, isLocalReg, closureChunkIndexReg);
        
        int indexReg = allocRegister();
        emit(TurboOpCode::LoadConst, indexReg, emitConstant(Value(uv.index)));
        emit(TurboOpCode::SetClosureIndex, indexReg, closureChunkIndexReg);
        
    }

    // gather createclosure info for dissaemble
    //closure_infos[to_string(ci)] = closure_info;
    
    if (scopeDepth == 0) {
        
    } else {
        // declareLocal(stmt->id);
        // int slot = paramSlot(stmt->id);
        // emit(OpCode::StoreLocal);
        // int nameIdx = emitConstant(Value::str(stmt->id));
        // emitUint32(slot);
    }
    
    disassembleChunk(nested.cur.get(), nested.cur->name);

    return closureChunkIndexReg;
    
}

R TurboCodeGen::visitFunctionExpression(FunctionExpression* expr) {
    
//    Token token;
//    vector<unique_ptr<Expression>> params;
//    unique_ptr<Statement> body;
//    bool is_async;

    // Create a nested CodeGen for the function body
    TurboCodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = make_shared<TurboChunk>();
    nested.beginScope();

    vector<string> paramNames;
    vector<ParameterInfo> parameterInfos;

    // Collect parameter info (name, hasDefault, defaultExpr, isRest)
    for (auto& param : expr->params) {
        
        if (SequenceExpression* seq = dynamic_cast<SequenceExpression*>(param.get())) {
            for (auto& p : seq->expressions) {
                if (auto* rest = dynamic_cast<RestParameter*>(p.get())) {
                    // ...rest
                    //if (auto* ident = dynamic_cast<IdentifierExpression*>(rest->argument.get())) {
                    paramNames.push_back(rest->token.lexeme);
                    parameterInfos
                        .push_back(ParameterInfo{rest->token.lexeme, false, nullptr, true});
                    //}
                } else if (auto* assign = dynamic_cast<BinaryExpression*>(p.get())) {
                    // b = 90 or c = b
                    if (auto* ident = dynamic_cast<IdentifierExpression*>(assign->left.get())) {
                        paramNames.push_back(ident->name);
                        parameterInfos.push_back(ParameterInfo{ident->name, true, assign->right.get(), false});
                    }
                } else if (auto* ident = dynamic_cast<IdentifierExpression*>(p.get())) {
                    // Simple arg
                    paramNames.push_back(ident->name);
                    parameterInfos.push_back(ParameterInfo{ident->name, false, nullptr, false});
                }
            }
        }
        else if (auto* rest = dynamic_cast<RestParameter*>(param.get())) {
            paramNames.push_back(rest->token.lexeme);
            parameterInfos.emplace_back(rest->token.lexeme, false, nullptr, true);
        }
        else if (auto* binary_expr = dynamic_cast<BinaryExpression*>(param.get())) {
            if (auto* ident = dynamic_cast<IdentifierExpression*>(binary_expr->left.get())) {
                paramNames.push_back(ident->name);
                parameterInfos.emplace_back(ident->name, true, binary_expr->right.get(), false);
            }
        }
        else if (auto* ident = dynamic_cast<IdentifierExpression*>(param.get())) {
            paramNames.push_back(ident->name);
            parameterInfos.push_back(ParameterInfo{ident->name, false, nullptr, false});
        }
        
    }

    // Allocate local slots for parameters
    nested.resetLocalsForFunction((uint32_t)paramNames.size(), paramNames);

    // Emit parameter initialization logic
    for (size_t i = 0; i < parameterInfos.size(); ++i) {
        const auto& info = parameterInfos[i];
        // For rest parameter
        if (info.isRest) {
            
            // collect rest arguments as array: arguments.slice(i)
            
            int arg_array_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArguments, arg_array_reg); // Push arguments array
            
            int i_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, i_reg, nested.emitConstant(Value::number(i))); // Push i
            nested.emit(TurboOpCode::Slice, arg_array_reg, i_reg); // arguments.slice(i)
            
            nested.freeRegister(i_reg);
            
            nested.store(info.name, arg_array_reg);
            
            nested.freeRegister(arg_array_reg);
            
            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            
            int store_reg = nested.allocRegister();
     
            int args_len_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArgumentsLength, args_len_reg);
            
            int index_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, index_reg, nested.emitConstant(Value::number(i)));
            
            nested.emit(TurboOpCode::GreaterThan, args_len_reg, index_reg);
            int useArg = nested.emitJump(TurboOpCode::JumpIfFalse, args_len_reg); // false means to use default value
            
            // Use argument
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));
            nested.emit(TurboOpCode::LoadArgument, reg);
            nested.emit(TurboOpCode::Move, store_reg, reg);
            
            int setLocalJump = nested.emitJump(TurboOpCode::Jump);
            
            // Use default
            nested.patchJump(useArg);
            
            // Evaluate default expression (can reference previous params!)
            int default_expr_reg = get<int>(info.defaultExpr->accept(nested));
            nested.emit(TurboOpCode::Move, store_reg, default_expr_reg);
            
            // Set local either way
            nested.patchSingleJump(setLocalJump);

            nested.store(info.name, store_reg);
            
            nested.freeRegister(store_reg);
            nested.freeRegister(default_expr_reg);
            nested.freeRegister(reg);
            nested.freeRegister(args_len_reg);
            nested.freeRegister(index_reg);
            
        } else {
            
            // Direct: assign argument i to local slot
            
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));

            nested.emit(TurboOpCode::LoadArgument, reg);
            
            nested.store(info.name, reg);
            nested.freeRegister(reg);
            
        }
    }

    // Compile function body
    if (expr->body) {
        expr->body->accept(nested);
        // TODO: walk the body ast to ensure OP_RETURN is emitted at the end if not emitted
        // TODO: we need to check if return is the last statement.
        bool is_return_avaialble = false;
        if (BlockStatement* block = dynamic_cast<BlockStatement*>(expr->body.get())) {
            for (auto& body : block->body) {
                if (auto return_stmt = dynamic_cast<ReturnStatement*>(body.get())) {
                    is_return_avaialble = true;
                }
            }
        }
        
        if (!is_return_avaialble) {
//            nested.emit(OpCode::LoadConstant);
//            int ud = nested.emitConstant(Value::undefined());
//            nested.emitUint32(ud);
//            nested.emit(OpCode::Return);
            
            int reg = nested.allocRegister();
            int ud = nested.emitConstant(Value::undefined());
            nested.emit(TurboOpCode::LoadConst, reg, ud);

            nested.emit(TurboOpCode::Return, reg);

        }

    }

    // Register chunk & emit as constant
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();

    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = std::make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = expr->token.lexeme; //"<anon>";
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

    int closureChunkIndexReg = allocRegister();
    emit(TurboOpCode::LoadConst, closureChunkIndexReg, emitConstant(Value(ci)));
    
    emit(TurboOpCode::CreateClosure, closureChunkIndexReg);

//    emit(OpCode::CreateClosure);
//    emitUint8((uint8_t)ci);

//    ClosureInfo closure_info = {};
//    closure_info.ci = ci;
//    closure_info.upvalues = nested.upvalues;

    // Emit upvalue descriptors
    for (auto& uv : nested.upvalues) {
//        emitUint8(uv.isLocal ? 1 : 0);
//        emitUint8(uv.index);

        int isLocalReg = allocRegister();
        emit(TurboOpCode::LoadConst, isLocalReg, emitConstant(Value(uv.isLocal ? 1 : 0)));
        emit(TurboOpCode::SetClosureIsLocal, isLocalReg, closureChunkIndexReg);
        
        int indexReg = allocRegister();
        emit(TurboOpCode::LoadConst, indexReg, emitConstant(Value(uv.index)));
        emit(TurboOpCode::SetClosureIndex, indexReg, closureChunkIndexReg);
        
    }
    
    // Bind function to its name in the global environment
    if (scopeDepth == 0) {
        // emit(OpCode::CreateGlobal);
         // int nameIdx = emitConstant(Value::str(expr->id));
         // emitUint32(nameIdx);
    } else {
        // declareLocal(stmt->id);
        // int slot = paramSlot(stmt->id);
        // emit(OpCode::StoreLocal);
        // int nameIdx = emitConstant(Value::str(stmt->id));
        // emitUint32(slot);
    }

    // closure_infos[to_string(ci)] = closure_info;
    disassembleChunk(nested.cur.get(), nested.cur->name);

    return closureChunkIndexReg;
}

R TurboCodeGen::visitFunction(FunctionDeclaration* stmt) {
    // Create a nested code generator for the function body
    TurboCodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = make_shared<TurboChunk>();
    nested.beginScope();
    
    std::vector<std::string> paramNames;
    std::vector<ParameterInfo> parameterInfos;

    // Collect parameter info (name, hasDefault, defaultExpr, isRest)
    for (auto& param : stmt->params) {
        if (SequenceExpression* seq = dynamic_cast<SequenceExpression*>(param.get())) {
            for (auto& p : seq->expressions) {
                if (auto* rest = dynamic_cast<RestParameter*>(p.get())) {
                    paramNames.push_back(rest->token.lexeme);
                    parameterInfos.emplace_back(rest->token.lexeme, false, nullptr, true);
                } else if (auto* assign = dynamic_cast<BinaryExpression*>(p.get())) {
                    if (auto* ident = dynamic_cast<IdentifierExpression*>(assign->left.get())) {
                        paramNames.push_back(ident->name);
                        parameterInfos.emplace_back(ident->name, true, assign->right.get(), false);
                    }
                } else if (auto* ident = dynamic_cast<IdentifierExpression*>(p.get())) {
                    paramNames.push_back(ident->name);
                    parameterInfos.emplace_back(ident->name, false, nullptr, false);
                }
            }
        }
        else if (auto* rest = dynamic_cast<RestParameter*>(param.get())) {
            paramNames.push_back(rest->token.lexeme);
            parameterInfos.emplace_back(rest->token.lexeme, false, nullptr, true);
        }
        else if (auto* binary_expr = dynamic_cast<BinaryExpression*>(param.get())) {
            if (auto* ident = dynamic_cast<IdentifierExpression*>(binary_expr->left.get())) {
                paramNames.push_back(ident->name);
                parameterInfos.emplace_back(ident->name, true, binary_expr->right.get(), false);
            }
        }
        else if (auto* ident = dynamic_cast<IdentifierExpression*>(param.get())) {
            paramNames.push_back(ident->name);
            parameterInfos.emplace_back(ident->name, false, nullptr, false);
        }
    }

    // Allocate local slots for parameters
    nested.resetLocalsForFunction((uint32_t)paramNames.size(), paramNames);

    // Emit parameter initialization logic
    for (size_t i = 0; i < parameterInfos.size(); ++i) {
        const auto& info = parameterInfos[i];
        // For rest parameter
        if (info.isRest) {
            
            // collect rest arguments as array: arguments.slice(i)
            
            int arg_array_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArguments, arg_array_reg); // Push arguments array
            
            int i_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, i_reg, nested.emitConstant(Value::number(i))); // Push i
            nested.emit(TurboOpCode::Slice, arg_array_reg, i_reg); // arguments.slice(i)
            
            nested.freeRegister(i_reg);
            
            nested.store(info.name, arg_array_reg);
            
            nested.freeRegister(arg_array_reg);
            
            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            
            int store_reg = nested.allocRegister();
     
            int args_len_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArgumentsLength, args_len_reg);
            
            int index_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, index_reg, nested.emitConstant(Value::number(i)));
            
            nested.emit(TurboOpCode::GreaterThan, args_len_reg, index_reg);
            int useArg = nested.emitJump(TurboOpCode::JumpIfFalse, args_len_reg); // false means to use default value
            
            // Use argument
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));
            nested.emit(TurboOpCode::LoadArgument, reg);
            nested.emit(TurboOpCode::Move, store_reg, reg);
            
            int setLocalJump = nested.emitJump(TurboOpCode::Jump);
            
            // Use default
            nested.patchJump(useArg);
            
            // Evaluate default expression (can reference previous params!)
            int default_expr_reg = get<int>(info.defaultExpr->accept(nested));
            nested.emit(TurboOpCode::Move, store_reg, default_expr_reg);
            
            // Set local either way
            nested.patchSingleJump(setLocalJump);

            nested.store(info.name, store_reg);
            
            nested.freeRegister(store_reg);
            nested.freeRegister(default_expr_reg);
            nested.freeRegister(reg);
            nested.freeRegister(args_len_reg);
            nested.freeRegister(index_reg);
            
        } else {
            
            // Direct: assign argument i to local slot
            
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));

            nested.emit(TurboOpCode::LoadArgument, reg);
            
            nested.store(info.name, reg);
            nested.freeRegister(reg);
            
        }
    }

    // Compile function body
    if (stmt->body) {
        stmt->body->accept(nested);
        // TODO: walk the body ast to ensure OP_RETURN is emitted at the end if not emitted
        // TODO: we need to check if return is the last statement.
        bool is_return_avaialble = false;
        if (BlockStatement* block = dynamic_cast<BlockStatement*>(stmt->body.get())) {
            for (auto& body : block->body) {
                if (auto return_stmt = dynamic_cast<ReturnStatement*>(body.get())) {
                    is_return_avaialble = true;
                }
            }
        }
        if (!is_return_avaialble) {
//            nested.emit(OpCode::LoadConstant);
//            int ud = nested.emitConstant(Value::undefined());
//            nested.emitUint32(ud);
//            nested.emit(OpCode::Return);
            
            int reg = nested.allocRegister();
            int ud = nested.emitConstant(Value::undefined());
            nested.emit(TurboOpCode::LoadConst, reg, ud);

            nested.emit(TurboOpCode::Return, reg);

        }

    }

    // Register chunk & emit as constant
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();

    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = stmt->id;
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

//    emit(OpCode::CreateClosure);
//    emitUint8((uint8_t)ci);
//
//    ClosureInfo closure_info = {};
//    closure_info.ci = ci;
//    closure_info.upvalues = nested.upvalues;
    int closureChunkIndexReg = allocRegister();
    emit(TurboOpCode::LoadConst, closureChunkIndexReg, emitConstant(Value(ci)));
    
    emit(TurboOpCode::CreateClosure, closureChunkIndexReg);

    // Emit upvalue descriptors
    for (auto& uv : nested.upvalues) {
//        emitUint8(uv.isLocal ? 1 : 0);
//        emitUint8(uv.index);
        
        int isLocalReg = allocRegister();
        emit(TurboOpCode::LoadConst, isLocalReg, emitConstant(Value(uv.isLocal ? 1 : 0)));
        emit(TurboOpCode::SetClosureIsLocal, isLocalReg, closureChunkIndexReg);
        
        int indexReg = allocRegister();
        emit(TurboOpCode::LoadConst, indexReg, emitConstant(Value(uv.index)));
        emit(TurboOpCode::SetClosureIndex, indexReg, closureChunkIndexReg);

    }
    
    // Bind function to its name in the global environment
//    if (scopeDepth == 0) {
//        emit(OpCode::CreateGlobal);
//         int nameIdx = emitConstant(Value::str(stmt->id));
//         emitUint32(nameIdx);
//    } else {
//        declareLocal(stmt->id, BindingKind::Var);
//        int slot = paramSlot(stmt->id);
//        emit(OpCode::StoreLocal);
//        // int nameIdx = emitConstant(Value::str(stmt->id));
//        emitUint32(slot);
//    }
    
    // closure_infos[to_string(ci)] = closure_info;

    declareLocal(stmt->id);
    declareGlobal(stmt->id, BindingKind::Var);
    
    create(stmt->id, closureChunkIndexReg, BindingKind::Var);

    // disassemble the chunk for debugging
    disassembleChunk(nested.cur.get(), stmt->id/*nested.cur->name*/);
    
    freeRegister(closureChunkIndexReg);

    return true;
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

string TurboCodeGen::resolveImportPath(ImportDeclaration* stmt) {
    
    namespace fs = std::filesystem;
    
    // dir of the file that contained the import
    fs::path baseDir = fs::path(stmt->sourceFile).parent_path();
    
    string raw = stmt->path.lexeme;
    if (!raw.empty() && raw.front() == '"' && raw.back() == '"') {
        raw = raw.substr(1, raw.size() - 2);
    }
    
    fs::path resolved = fs::weakly_canonical(baseDir / raw);
    
    return resolved.string();
    
}

R TurboCodeGen::visitImportDeclaration(ImportDeclaration* stmt) {
    
    // Resolve the path (relative or absolute)
    std::string importPath = resolveImportPath(stmt);

    // Check if module is already loaded (avoid cycles/duplication)
    if (isModuleLoaded(importPath)) {
        // Already loaded, nothing to do (or optionally, re-bind exported symbols)
        return true;
    }

    std::string source = read_file(importPath);

    // Parse the imported source to AST
    Scanner scanner(source);
    auto tokens = scanner.getTokens();
    
    // pass the resolved file path into the parser
    Parser parser(tokens);
    parser.sourceFile = importPath;
    auto ast = parser.parse();

    // Register the imported module BEFORE compiling to handle cycles
    registerModule(importPath);

    for (const auto &s : ast) {
        s->accept(*this);
    }

    return true;
    
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
    if (loopStack.empty()) {
        throw ("Break outside loop");
        return false;
    }
    // Emit jump with unknown target
    emit(TurboOpCode::Nop);
    int jumpAddr = emitJump(TurboOpCode::Jump);
    emit(TurboOpCode::Nop);
    loopStack.back().breaks.push_back(jumpAddr);
    return true;
}

R TurboCodeGen::visitContinue(ContinueStatement*) {
    // Usually: emit a jump to loop start
    if (loopStack.empty()) {
        throw ("Continue outside loop");
        return false;
    }
    int loopStart = loopStack.back().loopStart;
    emitLoop(loopStart); // emit a backwards jump
    return true;
}

R TurboCodeGen::visitThrow(ThrowStatement* stmt) {
    // stmt->argument->accept(*this);
    emit(TurboOpCode::Throw);
    return 0;
}

R TurboCodeGen::visitEmpty(EmptyStatement*) { return 0; }

int TurboCodeGen::compileMethod(MethodDefinition& method) {
    // Create a nested CodeGen for the method body (closure)
    TurboCodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = std::make_shared<TurboChunk>();
    nested.beginScope();
    // nested.declareLocal("this");
    nested.classInfo = classInfo;

    std::vector<std::string> paramNames;
    std::vector<ParameterInfo> parameterInfos;

    // Collect parameter info (from method.params)
    for (auto& param : method.params) {
        if (auto* seq = dynamic_cast<SequenceExpression*>(param.get())) {
            for (auto& p : seq->expressions) {
                if (auto* rest = dynamic_cast<RestParameter*>(p.get())) {
                    paramNames.push_back(rest->token.lexeme);
                    parameterInfos.push_back(ParameterInfo{rest->token.lexeme, false, nullptr, true});
                } else if (auto* assign = dynamic_cast<BinaryExpression*>(p.get())) {
                    if (auto* ident = dynamic_cast<IdentifierExpression*>(assign->left.get())) {
                        paramNames.push_back(ident->name);
                        parameterInfos.push_back(ParameterInfo{ident->name, true, assign->right.get(), false});
                    }
                } else if (auto* ident = dynamic_cast<IdentifierExpression*>(p.get())) {
                    paramNames.push_back(ident->name);
                    parameterInfos.push_back(ParameterInfo{ident->name, false, nullptr, false});
                }
            }
        } else if (auto* rest = dynamic_cast<RestParameter*>(param.get())) {
            paramNames.push_back(rest->token.lexeme);
            parameterInfos.push_back(ParameterInfo{rest->token.lexeme, false, nullptr, true});
        } else if (auto* assign = dynamic_cast<BinaryExpression*>(param.get())) {
            if (auto* ident = dynamic_cast<IdentifierExpression*>(assign->left.get())) {
                paramNames.push_back(ident->name);
                parameterInfos.push_back(ParameterInfo{ident->name, true, assign->right.get(), false});
            }
        } else if (auto* ident = dynamic_cast<IdentifierExpression*>(param.get())) {
            paramNames.push_back(ident->name);
            parameterInfos.push_back(ParameterInfo{ident->name, false, nullptr, false});
        }
    }

    // Allocate local slots for parameters
    nested.resetLocalsForFunction((uint32_t)paramNames.size(), paramNames);

    // Emit parameter initialization logic (rest/default)
    for (size_t i = 0; i < parameterInfos.size(); ++i) {
        const auto& info = parameterInfos[i];
        // For rest parameter
        if (info.isRest) {
            
            // collect rest arguments as array: arguments.slice(i)
            
            int arg_array_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArguments, arg_array_reg); // Push arguments array
            
            int i_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, i_reg, nested.emitConstant(Value::number(i))); // Push i
            nested.emit(TurboOpCode::Slice, arg_array_reg, i_reg); // arguments.slice(i)
            
            nested.freeRegister(i_reg);
            
            nested.store(info.name, arg_array_reg);
            
            nested.freeRegister(arg_array_reg);
            
            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            
            int store_reg = nested.allocRegister();
     
            int args_len_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadArgumentsLength, args_len_reg);
            
            int index_reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, index_reg, nested.emitConstant(Value::number(i)));
            
            nested.emit(TurboOpCode::GreaterThan, args_len_reg, index_reg);
            int useArg = nested.emitJump(TurboOpCode::JumpIfFalse, args_len_reg); // false means to use default value
            
            // Use argument
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));
            nested.emit(TurboOpCode::LoadArgument, reg);
            nested.emit(TurboOpCode::Move, store_reg, reg);
            
            int setLocalJump = nested.emitJump(TurboOpCode::Jump);
            
            // Use default
            nested.patchJump(useArg);
            
            // Evaluate default expression (can reference previous params!)
            int default_expr_reg = get<int>(info.defaultExpr->accept(nested));
            nested.emit(TurboOpCode::Move, store_reg, default_expr_reg);
            
            // Set local either way
            nested.patchSingleJump(setLocalJump);

            nested.store(info.name, store_reg);
            
            nested.freeRegister(store_reg);
            nested.freeRegister(default_expr_reg);
            nested.freeRegister(reg);
            nested.freeRegister(args_len_reg);
            nested.freeRegister(index_reg);
            
        } else {
            
            // Direct: assign argument i to local slot
            
            int reg = nested.allocRegister();
            nested.emit(TurboOpCode::LoadConst, reg, nested.emitConstant(Value::number(i)));

            nested.emit(TurboOpCode::LoadArgument, reg);
            
            nested.store(info.name, reg);
            
            nested.freeRegister(reg);
            
        }
    }

    // Compile the method body
    if (method.methodBody) {
        method.methodBody->accept(nested);
        // Ensure OP_RETURN is emitted
        bool hasReturn = false;
        if (auto* block = dynamic_cast<BlockStatement*>(method.methodBody.get())) {
            for (auto& stmt : block->body) {
                if (dynamic_cast<ReturnStatement*>(stmt.get())) {
                    hasReturn = true;
                    break;
                }
            }
        }
        if (!hasReturn) {
            int reg = nested.allocRegister();
            int ud = nested.emitConstant(Value::undefined());
            nested.emit(TurboOpCode::LoadConst, reg, ud);

            nested.emit(TurboOpCode::Return, reg);
        }
    }

    // Register the method function as a constant for this module
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();
    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = std::make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = method.name;
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

    // Emit closure for this function (leaves closure object on stack)
//    emit(OpCode::CreateClosure);
//    emitUint8((uint8_t)ci);
    int closureChunkIndexReg = allocRegister();
    emit(TurboOpCode::LoadConst, closureChunkIndexReg, emitConstant(Value(ci)));
    
    emit(TurboOpCode::CreateClosure, closureChunkIndexReg);

    for (auto& uv : nested.upvalues) {
        //        emitUint8(uv.isLocal ? 1 : 0);
        //        emitUint8(uv.index);
        
        int isLocalReg = allocRegister();
        emit(TurboOpCode::LoadConst, isLocalReg, emitConstant(Value(uv.isLocal ? 1 : 0)));
        emit(TurboOpCode::SetClosureIsLocal, isLocalReg, closureChunkIndexReg);
        
        int indexReg = allocRegister();
        emit(TurboOpCode::LoadConst, indexReg, emitConstant(Value(uv.index)));
        emit(TurboOpCode::SetClosureIndex, indexReg, closureChunkIndexReg);
        
    }
    
    disassembleChunk(nested.cur.get(), method.name);
    
    return closureChunkIndexReg;

}

R TurboCodeGen::visitClass(ClassDeclaration* stmt) {
        
    // Evaluate superclass (if any)
    int super_class_reg = allocRegister();
    if (stmt->superClass) {
        super_class_reg = get<int>(stmt->superClass->accept(*this)); // [superclass]
    } else {
        emit(TurboOpCode::LoadConst, super_class_reg, emitConstant(Value::nullVal()));
    }

    // Create the class object (with superclass on stack)
    emit(TurboOpCode::NewClass, super_class_reg);

    // Define fields
    // A field can be var, let, const. private, public, protected
    for (auto& field : stmt->fields) {
        bool isStatic = false;
        bool isPrivate = false;
        bool isPublic = false;
        bool isProtected = false;
        
        for (const auto& mod : field->modifiers) {
            if (auto* staticKW = dynamic_cast<StaticKeyword*>(mod.get())) {
                isStatic = true;
            }
            if (auto* privateKW = dynamic_cast<PrivateKeyword*>(mod.get())) {
                isPrivate = true;
            }
            if (auto* publicKW = dynamic_cast<PublicKeyword*>(mod.get())) {
                isPublic = true;
            }
            if (auto* protectedKW = dynamic_cast<ProtectedKeyword*>(mod.get())) {
                isProtected = true;
            }
        }

        // Property is always a VariableStatement
        if (auto* varStmt = dynamic_cast<VariableStatement*>(field->property.get())) {
            
            string kind = varStmt->kind;
            
            for (const auto& decl : varStmt->declarations) {
                
                classInfo.fields.insert(decl.id);
                
                int initReg = allocRegister();
                
                if (decl.init) {
                    initReg = get<int>(decl.init->accept(*this)); // Evaluate initializer
                } else {
                    emit(TurboOpCode::LoadConst,
                         initReg,
                         emitConstant(Value::undefined()));
                }
                
                int nameIdx = emitConstant(Value::str(decl.id));
                int fieldNameReg = allocRegister();
                emit(TurboOpCode::LoadConst, fieldNameReg, nameIdx);
                TurboOpCode op;

                if (isStatic) {
                                        
                    switch (get_kind(kind)) {
                        case BindingKind::Var:
                            if (isPublic) {
                                op = TurboOpCode::CreateClassPublicStaticPropertyVar;
                            } else if (isPrivate) {
                                op = TurboOpCode::CreateClassPrivateStaticPropertyVar;
                            } else if (isProtected) {
                                op = TurboOpCode::CreateClassProtectedStaticPropertyVar;
                            } else {
                                op = TurboOpCode::CreateClassPublicStaticPropertyVar;
                            }
                            break;
                        case BindingKind::Const:
                            if (isPublic) {
                                op = TurboOpCode::CreateClassPublicStaticPropertyConst;
                            } else if (isPrivate) {
                                op = TurboOpCode::CreateClassPrivateStaticPropertyConst;
                            } else if (isProtected) {
                                op = TurboOpCode::CreateClassProtectedStaticPropertyConst;
                            } else {
                                op = TurboOpCode::CreateClassPublicStaticPropertyConst;
                            }
                            break;
                        default:
                            throw runtime_error("Fields in classes must have a Binding kind. e.g var.");
                            break;
                    }
                    
                    emit(op, super_class_reg, initReg, fieldNameReg);
                    
                } else {
                                        
                    switch (get_kind(kind)) {
                        case BindingKind::Var:
                            if (isPublic) {
                                op = TurboOpCode::CreateClassPublicPropertyVar;
                            } else if (isPrivate) {
                                op = TurboOpCode::CreateClassPrivatePropertyVar;
                            } else if (isProtected) {
                                op = TurboOpCode::CreateClassProtectedStaticPropertyVar;
                            } else {
                                op = TurboOpCode::CreateClassPublicPropertyVar;
                            }
                            break;
                        case BindingKind::Const:
                            if (isPublic) {
                                op = TurboOpCode::CreateClassPublicPropertyConst;
                            } else if (isPrivate) {
                                op = TurboOpCode::CreateClassPrivatePropertyConst;
                            } else if (isProtected) {
                                op = TurboOpCode::CreateClassProtectedPropertyConst;
                            } else {
                                op = TurboOpCode::CreateClassPublicPropertyConst;
                            }
                            break;
                        default:
                            throw runtime_error("Fields in classes must have a Binding kind. e.g var.");
                            break;
                    }
                    
                    emit(op, super_class_reg, initReg, fieldNameReg);
                }
                
                freeRegister(initReg);
                freeRegister(fieldNameReg);
                
            }
                        
        }
        
    }
    
    for (auto& method : stmt->body) {
        
        // Check if 'static' modifier is present
        bool isStatic = false;
        for (const auto& mod : method->modifiers) {
            if (auto* staticKW = dynamic_cast<StaticKeyword*>(mod.get())) {
                isStatic = true;
                break;
            }
        }

        if (!isStatic) {
            classInfo.fields.insert(method->name);
        }

    }

    // Define methods (attach to class or prototype as appropriate)
    for (auto& method : stmt->body) {
        
        bool isStatic = false;
        bool isPrivate = false;
        bool isPublic = false;
        bool isProtected = false;
        
        for (const auto& mod : method->modifiers) {
            if (auto* staticKW = dynamic_cast<StaticKeyword*>(mod.get())) {
                isStatic = true;
            }
            if (auto* privateKW = dynamic_cast<PrivateKeyword*>(mod.get())) {
                isPrivate = true;
            }
            if (auto* publicKW = dynamic_cast<PublicKeyword*>(mod.get())) {
                isPublic = true;
            }
            if (auto* protectedKW = dynamic_cast<ProtectedKeyword*>(mod.get())) {
                isProtected = true;
            }
        }

        // Compile the method as a function object
        int method_reg = compileMethod(*method); // leaves function object on reg

        // Get name of method
        int nameIdx = emitConstant(Value::str(method->name));
        int methodNameReg = allocRegister();
        emit(TurboOpCode::LoadConst, methodNameReg, nameIdx);
        TurboOpCode op;

        if (isStatic) {
            
            if (isPublic) {
                op = TurboOpCode::CreateClassPublicStaticMethod;
            } else if (isPrivate) {
                op = TurboOpCode::CreateClassPrivateStaticMethod;
            } else if (isProtected) {
                op = TurboOpCode::CreateClassProtectedStaticMethod;
            } else {
                op = TurboOpCode::CreateClassPublicStaticMethod;
            }
            
            emit(op, super_class_reg, method_reg, methodNameReg);
            
        } else {
            
            if (isPublic) {
                op = TurboOpCode::CreateClassPublicMethod;
            } else if (isPrivate) {
                op = TurboOpCode::CreateClassPrivateMethod;
            } else if (isProtected) {
                op = TurboOpCode::CreateClassProtectedMethod;
            } else {
                op = TurboOpCode::CreateClassPublicMethod;
            }
            
            emit(op, super_class_reg, method_reg, methodNameReg);
            
            freeRegister(method_reg);
            freeRegister(methodNameReg);
            
        }
    }

    // Bind class in the environment (global)
    // int classNameIdx = emitConstant(Value::str(stmt->id));

    int class_reg = allocRegister();
    declareLocal(stmt->id);
    declareGlobal(stmt->id, BindingKind::Var);
    
    create(stmt->id, super_class_reg, BindingKind::Var);

    // clear class info
    classInfo.fields.clear();
    
    freeRegister(class_reg);
    freeRegister(super_class_reg);

    return true;
}

R TurboCodeGen::visitMethodDefinition(MethodDefinition*) { return 0; }

R TurboCodeGen::visitDoWhile(DoWhileStatement*) { return 0; }

R TurboCodeGen::visitSwitchCase(SwitchCase*) { return 0; }

R TurboCodeGen::visitSwitch(SwitchStatement*) { return 0; }

R TurboCodeGen::visitCatch(CatchClause*) { return 0; }

R TurboCodeGen::visitTry(TryStatement*) { return 0; }

R TurboCodeGen::visitForIn(ForInStatement* stmt) {
    // Evaluate the object to iterate
    int objReg = get<int>(stmt->object->accept(*this));
    int keysReg = allocRegister();

    // Get keys array (assumes builtin/function to get keys)
    emit(TurboOpCode::EnumKeys, keysReg, objReg);

    // Prepare index and length
    int idxReg = allocRegister();
    emit(TurboOpCode::LoadConst, idxReg, emitConstant(Value(0)));

    int lenReg = allocRegister();
    emit(TurboOpCode::GetObjectLength, lenReg, keysReg);

    int loopStart = (int)cur->code.size();

    // if (idx >= len) break;
    int condReg = allocRegister();
    emit(TurboOpCode::LessThan, condReg, idxReg, lenReg);
    int breakJump = emitJump(TurboOpCode::JumpIfFalse, condReg);

    // Get current key: key = keys[idx]
    int keyReg = allocRegister();
    emit(TurboOpCode::GetPropertyDynamic, keyReg, keysReg, idxReg);

    // Assign keyReg to loop variable (var/let/const)
    // Assign value to loop variable
    if (auto* ident = dynamic_cast<IdentifierExpression*>(stmt->init.get())) {
        store(ident->name, keyReg);
    } else if (auto* var_stmt = dynamic_cast<VariableStatement*>(stmt->init.get())) {
        store(var_stmt->declarations[0].id, keyReg);
    } else if (auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt->init.get())) {
        
        if (auto* ident = dynamic_cast<IdentifierExpression*>(expr_stmt->expression.get())) {
            store(ident->name, keyReg);
        }
        
    } else {
        throw std::runtime_error("for-in only supports identifier/variable statement loop variables in codegen");
    }
    
    freeRegister(keyReg);

    // Loop body
    stmt->body->accept(*this);

    // idx++
    emit(TurboOpCode::Add, idxReg, idxReg, emitConstant(Value(1)));

    // Jump to loop start
    emit(TurboOpCode::Jump, 0, loopStart - (int)cur->code.size() - 1);

    // Patch break
    patchJump(breakJump, (int)cur->code.size());

    // Free registers
    freeRegister(objReg);
    freeRegister(keysReg);
    freeRegister(idxReg);
    freeRegister(lenReg);
    freeRegister(condReg);

    return 0;
}

R TurboCodeGen::visitForOf(ForOfStatement* stmt) {
    
    // Evaluate the iterable
    int arrReg = get<int>(stmt->right->accept(*this));

    // Get length
    int lenReg = allocRegister();
    emit(TurboOpCode::GetObjectLength, lenReg, arrReg);

    // Index register
    int idxReg = allocRegister();
    emit(TurboOpCode::LoadConst, idxReg, emitConstant(Value(0)));

    // Loop start
    int loopStart = (int)cur->code.size();

    // if (idx >= len) break;
    int condReg = allocRegister();
    emit(TurboOpCode::LessThan, condReg, idxReg, lenReg);
    int breakJump = emitJump(TurboOpCode::JumpIfFalse, condReg);

    // Get current element: arr[idx]
    int elemReg = allocRegister();
    emit(TurboOpCode::GetPropertyDynamic, elemReg, arrReg, idxReg);

    // Assign element to loop variable
    if (auto* ident = dynamic_cast<IdentifierExpression*>(stmt->left.get())) {
        store(ident->name, elemReg);
    } else if (auto* var_stmt = dynamic_cast<VariableStatement*>(stmt->left.get())) {
        store(var_stmt->declarations[0].id, elemReg);
    } else if (auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt->left.get())) {
        
        if (auto* ident = dynamic_cast<IdentifierExpression*>(expr_stmt->expression.get())) {
            store(ident->name, elemReg);
        }
        
    } else {
        throw std::runtime_error("for-of only supports identifier/variable statement loop variables in codegen");
    }
    
    freeRegister(elemReg);

    // Loop body
    stmt->body->accept(*this);

    // idx++
    emit(TurboOpCode::Add, idxReg, idxReg, emitConstant(Value(1)));

    // Jump back
    emit(TurboOpCode::Jump, 0, loopStart - (int)cur->code.size() - 1);

    // Patch break
    patchJump(breakJump, (int)cur->code.size());

    // Free registers
    freeRegister(arrReg);
    freeRegister(lenReg);
    freeRegister(idxReg);
    freeRegister(condReg);

    return 0;
}

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

int TurboCodeGen::allocRegister() {
    return registerAllocator->alloc();
}

void TurboCodeGen::freeRegister(uint32_t slot) {
    registerAllocator->free(slot);
}

// Jump helpers
void TurboCodeGen::emitLoop(uint32_t loopStart) {
    emit(TurboOpCode::Loop, ((int)cur->code.size() - (int)loopStart) + 1, 0, 0);
}

int TurboCodeGen::emitJump(TurboOpCode op, int cond_reg = 0) {
    // Reserve space, return jump location to patch
    // Implementation depends on code buffer layout
    // Emit the jump instruction with placeholder offset
    // We'll patch .b later to hold the actual jump offset
    Instruction instr(op, (uint8_t)cond_reg, 0, 0); // b is offset placeholder
    cur->code.push_back(instr);
    return (int)cur->code.size() - 1; // Return index of this jump instruction
}

int TurboCodeGen::emitJump(TurboOpCode op) {
    Instruction instr(op, 0, 0, 0); // a is offset placeholder
    cur->code.push_back(instr);
    return (int)cur->code.size() - 1; // Return index of this jump instruction
}

// for TurboCode::JumpIfFalse
void TurboCodeGen::patchJump(int jumpPos, int target) {
    // int offset = target - (jumpPos + 1);
    int offset = (target - 1) - (jumpPos);
    cur->code[jumpPos].b = (uint8_t)offset;
}

// for TurboCode::JumpIfFalse
void TurboCodeGen::patchJump(int jumpPos) {
    int offset = ((int)cur->code.size() - 1 ) - (jumpPos);
    cur->code[jumpPos].b = (uint8_t)offset;
}

// for TurboCode::Jump
void TurboCodeGen::patchSingleJump(int jumpPos) {
    int offset = ((int)cur->code.size() - 1 ) - (jumpPos);
    cur->code[jumpPos].a = (uint8_t)offset;
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

void TurboCodeGen::resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames) {
    locals.clear();
    nextLocalSlot = 0;
    for (uint32_t i = 0; i < paramCount; ++i) {
        string name = (i < paramNames.size()) ? paramNames[i] : ("_p" + std::to_string(i));
        Local local { name, /*depth=*/1, /*isCaptured=*/false, (uint32_t)i, BindingKind::Var }; // usually scopeDepth=1 for params
        locals.push_back(local);
        nextLocalSlot = i + 1;
    }
    
    if (cur) cur->maxLocals = nextLocalSlot;
}

void TurboCodeGen::declareLocal(const string& name) {
    if (scopeDepth == 0) return; // globals aren’t locals

    // prevent shadowing in same scope
    for (int i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].depth != -1 && locals[i].depth < scopeDepth) break;
        if (locals[i].name == name) {
            throw runtime_error("Variable already declared in this scope");
        }
    }

    uint32_t idx = (uint32_t)locals.size();

    Local local { name, scopeDepth, false, (uint32_t)locals.size() };
    locals.push_back(local);
    
    if (idx + 1 > cur->maxLocals) cur->maxLocals = idx + 1;

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

void TurboCodeGen::endLoop() {
    LoopContext ctx = loopStack.back();
    loopStack.pop_back();

    // Patch all breaks to jump here
    // int end = (int)cur->code.size() - 1;
    for (int breakAddr : ctx.breaks) {
        //patchJump(breakAddr, end);
        patchSingleJump(breakAddr);
    }
}

void TurboCodeGen::beginLoop() {
    LoopContext ctx;
    ctx.loopStart = (int)cur->code.size();
    loopStack.push_back(ctx);
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

bool TurboCodeGen::isModuleLoaded(string importPath) {
    
    for (auto& module_ : registered_modules) {
        if (module_ == importPath) {
            return true;
        }
    }

    return false;

}

void TurboCodeGen::registerModule(string importPath) {
    registered_modules.push_back(importPath);
}

size_t TurboCodeGen::disassembleInstruction(const TurboChunk* chunk, size_t offset) {
    if (offset >= chunk->code.size()) return offset + 1;

    const Instruction& instr = chunk->code[offset];
    std::cout << std::setw(4) << offset << " ";

    // Print opcode name
    std::string opName;
    switch (instr.op) {
        case TurboOpCode::Nop: opName = "Nop"; break;
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
        case TurboOpCode::PushArg: opName = "PushArg"; break;
        case TurboOpCode::CreateClosure: opName = "CreateClosure"; break;
        case TurboOpCode::GetProperty: opName = "GetProperty"; break;
        case TurboOpCode::Halt: opName = "Halt"; break;
        case TurboOpCode::Throw: opName = "Throw"; break;
        case TurboOpCode::Loop: opName = "Loop"; break;

        case TurboOpCode::LoadArgumentsLength: opName = "LoadArgumentsLength"; break;
        case TurboOpCode::LoadArguments: opName = "LoadArguments"; break;
        case TurboOpCode::LoadArgument: opName = "LoadArgument"; break;

        case TurboOpCode::NewClass: opName = "NewClass"; break;
        case TurboOpCode::CreateInstance: opName = "CreateInstance"; break;
        case TurboOpCode::InvokeConstructor: opName = "InvokeConstructor"; break;

        case TurboOpCode::CreateClassPrivatePropertyVar: opName = "CreateClassPrivatePropertyVar"; break;
        case TurboOpCode::CreateClassPublicPropertyVar: opName = "CreateClassPublicPropertyVar"; break;
        case TurboOpCode::CreateClassProtectedPropertyVar: opName = "CreateClassProtectedPropertyVar"; break;
        case TurboOpCode::CreateClassPrivatePropertyConst: opName = "CreateClassPrivatePropertyConst"; break;
        case TurboOpCode::CreateClassPublicPropertyConst: opName = "CreateClassPublicPropertyConst"; break;
        case TurboOpCode::CreateClassProtectedPropertyConst: opName = "CreateClassProtectedPropertyConst"; break;

        case TurboOpCode::CreateClassPrivateStaticPropertyVar: opName = "CreateClassPrivateStaticPropertyVar"; break;
        case TurboOpCode::CreateClassPublicStaticPropertyVar: opName = "CreateClassPublicStaticPropertyVar"; break;
        case TurboOpCode::CreateClassProtectedStaticPropertyVar: opName = "CreateClassProtectedStaticPropertyVar"; break;
        case TurboOpCode::CreateClassPrivateStaticPropertyConst: opName = "CreateClassPrivateStaticPropertyConst"; break;
        case TurboOpCode::CreateClassPublicStaticPropertyConst: opName = "CreateClassPublicStaticPropertyConst"; break;
        case TurboOpCode::CreateClassProtectedStaticPropertyConst: opName = "CreateClassProtectedStaticPropertyConst"; break;

        case TurboOpCode::CreateClassProtectedStaticMethod: opName = "CreateClassProtectedStaticMethod"; break;
        case TurboOpCode::CreateClassPrivateStaticMethod: opName = "CreateClassPrivateStaticMethod"; break;
        case TurboOpCode::CreateClassPublicStaticMethod: opName = "CreateClassPublicStaticMethod"; break;
        case TurboOpCode::CreateClassProtectedMethod: opName = "CreateClassProtectedMethod"; break;
        case TurboOpCode::CreateClassPrivateMethod: opName = "CreateClassPrivateMethod"; break;
        case TurboOpCode::CreateClassPublicMethod: opName = "CreateClassPublicMethod"; break;

        // Add more opcodes as needed
        default: opName = "Unknown"; break;
    }

    std::cout << std::left << std::setw(20) << opName;

    // Print operands
    std::cout << " a: " << (int)instr.a
              << " b: " << (int)instr.b
              << " c: " << (int)instr.c;

    // For LoadConst, print constant value
    if ((instr.op == TurboOpCode::LoadConst || instr.op == TurboOpCode::LoadGlobalVar || instr.op == TurboOpCode::LoadLocalVar) && instr.b < chunk->constants.size()) {
        std::cout << " [const: " << chunk->constants[instr.b].toString() << "]";
    }
    
    if ((instr.op == TurboOpCode::GetProperty) && instr.c < chunk->constants.size()) {
        std::cout << " [const: " << chunk->constants[instr.c].toString() << "]";
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

