//
//  CodeGenerator.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "CodeGenerator.hpp"

class StringPool {
    unordered_map<string, shared_ptr<string>> pool;
public:
    shared_ptr<string> intern(const string& s) {
        auto [it, inserted] = pool.emplace(s, nullptr);
        if (inserted) it->second = make_shared<string>(s);
        return it->second;
    }
};

CodeGen::CodeGen() : cur(nullptr), nextLocalSlot(0) { }

size_t CodeGen::generate(const vector<unique_ptr<Statement>> &program) {
    cur = std::make_shared<Chunk>();
    cur->name = "BYTECODE";
    locals.clear();
    nextLocalSlot = 0;

    for (const auto &s : program) {
        s->accept(*this);
    }

    emit(OpCode::Halt);
    disassembleChunk(cur.get(), cur->name);
    
    uint32_t idx = module_->addChunk(cur);
    module_->entryChunkIndex = idx;
    
    return idx;
}

// ------------------- Statements --------------------

R CodeGen::visitExpression(ExpressionStatement* stmt) {
    stmt->expression->accept(*this);
    emit(OpCode::Pop);
    return true;
}

R CodeGen::visitBlock(BlockStatement* stmt) {
    
    beginScope();
    
    for (auto& s : stmt->body) {
        s->accept(*this);
    }
    
    endScope();
    
    return true;
}

//R CodeGen::_define(string decl) {
//    
//    // decide local or global
//    if (hasLocal(decl)) {
//        uint32_t idx = getLocal(decl);
//        
//        // auto localIndex = resolveLocal(decl);
//        if (locals[idx].kind == BindingKind::Const) {
//            throw std::runtime_error("Assignment to constant variable.");
//        }
//        
//        emit(OpCode::StoreLocal);
//        emitUint32(idx);
//    } else {
//
//        if (classInfo.fields.count(decl)) {
//            
//            // Rewrite: legs = one;  ⇒  this.legs = one;
//            // Rewrite: legs;  ⇒  this.legs;
//            emit(OpCode::SetThisProperty);
//            int nameIdx = emitConstant(Value::str(decl));
//            emitUint32(nameIdx);
//            return R();
//        }
//
//        int upvalue = resolveUpvalue(decl);
//        if (upvalue != -1) {
//            emit(OpCode::SetUpvalue);
//            emitUint32(upvalue);
//            return R();
//        }
//
//        // top-level/global
//        int nameIdx = emitConstant(Value::str(decl));
//        // emit(OpCode::CreateGlobal);
//        emit(OpCode::StoreGlobal);
//        emitUint32((uint32_t)nameIdx);
//    }
//    
//    return true;
//
//}

// creates data into local/global
R CodeGen::create(string decl, BindingKind kind) {
    
    OpCode op;

    // decide local or global
    if (hasLocal(decl)) {
        uint32_t idx = getLocal(decl);
                
        switch (kind) {
            case BindingKind::Var:
                
                if (enclosing == nullptr) {
                    op = OpCode::CreateGlobalVar;
                } else {
                    op = OpCode::CreateLocalVar;
                }
                
                break;
            case BindingKind::Let:
                op = OpCode::CreateLocalLet;
                break;
            case BindingKind::Const:
                op = OpCode::CreateLocalConst;
                break;
            default:
                op = OpCode::CreateLocalVar;
                break;
        }

        emit(op);
        emitUint32(idx);

    } else {
        
        // This has been done when the class was being created.
        if (classInfo.fields.count(decl)) {
            return R();
        }
        
        // TODO: check if we need to create upvalues.
        int upvalue = resolveUpvalue(decl);
        if (upvalue != -1) {
            return R();
        }
        
        // top-level/global
        int nameIdx = emitConstant(Value::str(decl));
        switch (kind) {
            case BindingKind::Var:
                op = OpCode::CreateGlobalVar;
                break;
            case BindingKind::Let:
                op = OpCode::CreateGlobalLet;
                break;
            case BindingKind::Const:
                op = OpCode::CreateGlobalConst;
                break;
            default:
                op = OpCode::CreateGlobalVar;
                break;
        }

        emit(op);
        emitUint32((uint32_t)nameIdx);
                
    }
    
    return true;

}

// moves data from stack into local/global
R CodeGen::store(string decl) {
    
    // decide local or global
    if (hasLocal(decl)) {
        
        uint32_t idx = getLocal(decl);
        
        Local local = locals[idx];
        
        if (locals[idx].kind == BindingKind::Const) {
            throw std::runtime_error("Cannot assign value to constant variable.");
        }
        
        if (local.kind == BindingKind::Var) {
            emit(OpCode::StoreLocalVar);
            emitUint32(idx);
        } else if (local.kind == BindingKind::Let) {
            emit(OpCode::StoreLocalLet);
            emitUint32(idx);
        }
        
    } else {
        
        PropertyLookup classProperty = lookupClassProperty(decl);
        if (classProperty.level > 0 && classProperty.meta.kind == BindingKind::Const) {
            throw runtime_error("Cannot assign value to a const field.");
        }
        
        if (classProperty.level == 1) {
            
            // Rewrite: legs = one;  ⇒  this.legs = one;
            // Rewrite: legs;  ⇒  this.legs;
            int nameIdx = emitConstant(Value::str(decl));
            emit(OpCode::StoreThisProperty);
            emitUint32(nameIdx);
            
            // emit(TurboOpCode::SetThisProperty);
            // int nameIdx = emitConstant(Value::str(decl));
            // emitUint32(nameIdx);
            
            return true;
            
        } else if (classProperty.level == 2) {
            
            int nameIdx = emitConstant(Value::str(decl));
            
            emit(OpCode::GetParentObject);
            emit(OpCode::SetProperty);
            emitUint32(nameIdx);
            
            return true;

        }
        
        int upvalue = resolveUpvalue(decl);
        if (upvalue != -1) {
            
            UpvalueMeta upvalueMeta = upvalues[upvalue];
            // emit(TurboOpCode::SetUpvalue);
            // emitUint32(upvalue);
            
            if (upvalueMeta.kind == BindingKind::Var) {
                emit(OpCode::StoreUpvalueVar);
                emitUint32(upvalue);
            } else if (upvalueMeta.kind == BindingKind::Let) {
                emit(OpCode::StoreUpvalueLet);
                emitUint32(upvalue);
            } else if (upvalueMeta.kind == BindingKind::Const) {
                emit(OpCode::StoreUpvalueConst);
                emitUint32(upvalue);
            }
            
            return true;
        }
        
        // top-level/global
        int nameIdx = emitConstant(Value::str(decl));
        Global global = globals[lookupGlobal(decl)];
        
        if (global.kind == BindingKind::Const) {
            throw runtime_error("Cannot assign value to a const expression");
        }
        
        if (global.kind == BindingKind::Var) {
            emit(OpCode::StoreGlobalVar);
            emitUint32((uint32_t)nameIdx);
        } else if (global.kind == BindingKind::Let) {
            emit(OpCode::StoreGlobalLet);
            emitUint32((uint32_t)nameIdx);
        }
        
    }
    
    return true;

}

// moves data from local/global into stack
R CodeGen::load(string decl) {
    
    // decide local or global
    if (hasLocal(decl)) {
        uint32_t idx = getLocal(decl);
        emit(OpCode::LoadLocalVar);
        emitUint32(idx);
    } else {
        
        // search if decl is in class fields
        PropertyLookup result = lookupClassProperty(decl);
        if (result.level == 1) {
            
            // Rewrite: legs = one;  ⇒  this.legs = one;
            // Rewrite: legs;  ⇒  this.legs;
            int nameIdx = emitConstant(Value::str(decl));
            emit(OpCode::LoadThisProperty);
            emitUint32(nameIdx);
            
            // emit(TurboOpCode::SetThisProperty);
            // int nameIdx = emitConstant(Value::str(decl));
            // emitUint32(nameIdx);
            return true;
        } else if (result.level == 2) {
            // exists in parent class
            
            int nameIdx = emitConstant(Value::str(decl));

            emit(OpCode::GetParentObject);
            emit(OpCode::GetProperty);
            emitUint32(nameIdx);
            return true;
        }
        
        int upvalue = resolveUpvalue(decl);
        if (upvalue != -1) {
            // emit(TurboOpCode::SetUpvalue);
            // emitUint32(upvalue);
            emit(OpCode::LoadUpvalue);
            emitUint32(upvalue);
            return true;
        }
        
        int nameIdx = emitConstant(Value::str(decl));
        emit(OpCode::LoadGlobalVar);
        emitUint32((uint32_t)nameIdx);
        
    }
    
    return true;

}

R CodeGen::visitVariable(VariableStatement* stmt) {
    
    // var, let and const
    string kind = stmt->kind;
    BindingKind bindingKind = get_kind(kind);

    for (auto &decl : stmt->declarations) {

        if (bindingKind == BindingKind::Const && decl.init == nullptr) {
            throw runtime_error("Const variables must be initialized.");
            return true;
        }

        if (decl.init) {
            decl.init->accept(*this); // push init value
            
            if (auto classExpr = dynamic_cast<ClassExpression*>(decl.init.get())) {
                classExpr->name = decl.id;
            } else if (auto functionExpr = dynamic_cast<FunctionExpression*>(decl.init.get())) {
                functionExpr->name = decl.id;
            } else if (auto arrowFunctionExpr = dynamic_cast<ArrowFunction*>(decl.init.get())) {
                arrowFunctionExpr->name = decl.id;
            }

        } else {
            emit(OpCode::LoadConstant);
            int ci = emitConstant(Value::undefined());
            emitUint32(ci);
        }
                
        // define(decl.id);

        declareVariableScoping(decl.id, bindingKind);

        // TODO: add bits for var, let or const.
        create(decl.id, bindingKind);
        
    }
    
    return true;
    
}

R CodeGen::visitIf(IfStatement* stmt) {

    beginScope();

    stmt->test->accept(*this);
    int elseJump = emitJump(OpCode::JumpIfFalse);
    emit(OpCode::Pop); // pop condition
    if (stmt->consequent) stmt->consequent->accept(*this);
    int endJump = emitJump(OpCode::Jump);
    patchJump(elseJump);
    emit(OpCode::Pop); // pop condition (if we branched)
    if (stmt->alternate) stmt->alternate->accept(*this);
    patchJump(endJump);
    
    endScope();

    return true;
}

R CodeGen::visitWhile(WhileStatement* stmt) {
    beginLoop();
    loopStack.back().loopStart = (int)cur->size();
    beginScope();

    // loop start
    size_t loopStart = cur->size();
    stmt->test->accept(*this);
    int exitJump = emitJump(OpCode::JumpIfFalse);
    emit(OpCode::Pop);
    stmt->body->accept(*this);
    
    if (loopStack.back().continues.size() > 0) {
        for (auto& continueAddr : loopStack.back().continues) {
            patchJump(continueAddr);
        }
        
        loopStack.back().continues.clear();
    }

    emitLoop((uint32_t)(cur->size() - loopStart + 4 + 1));
    patchJump(exitJump);
    emit(OpCode::Pop);
    
    endScope();

    endLoop();
    
    return true;
}

R CodeGen::visitFor(ForStatement* stmt) {
    
    beginLoop();
    loopStack.back().loopStart = (int)cur->size();
    beginScope();
    
    // For simplicity, translate to while:
    if (stmt->init) stmt->init->accept(*this);
    size_t loopStart = cur->size();
    if (stmt->test) {
        stmt->test->accept(*this);
        int exitJump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);
        stmt->body->accept(*this);
        
        if (loopStack.back().continues.size() > 0) {
            for (auto& continueAddr : loopStack.back().continues) {
                patchJump(continueAddr);
            }
            
            loopStack.back().continues.clear();
        }

        if (stmt->update) stmt->update->accept(*this);
        // loop back
        emitLoop((uint32_t)(cur->size() - loopStart ) + 4 + 1);
        patchJump(exitJump);
        emit(OpCode::Pop);
    } else {
        // infinite loop
        stmt->body->accept(*this);
        if (stmt->update) stmt->update->accept(*this);
        emitLoop((uint32_t)(cur->size() - loopStart ) + 4 + 1);
    }
    endScope();
    endLoop();
    
    return true;
}

R CodeGen::visitReturn(ReturnStatement* stmt) {
    if (stmt->argument) {
        stmt->argument->accept(*this);
    } else {
        int ci = emitConstant(Value::undefined());
        emit(OpCode::LoadConstant);
        emitUint32(ci);
    }
    emit(OpCode::Return);
    return true;
}

// ------------------- Expressions --------------------

R CodeGen::visitBinary(BinaryExpression* expr) {
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
    expr->left->accept(*this);
    expr->right->accept(*this);

    // Emit the appropriate operation instruction
    switch (expr->op.type) {
        // --- Arithmetic ---
        case TokenType::ADD:
            emit(OpCode::Add); // stack: left, right -> left+right
            break;
        case TokenType::MINUS:
            emit(OpCode::Subtract);
            break;
        case TokenType::MUL:
            emit(OpCode::Multiply);
            break;
        case TokenType::DIV:
            emit(OpCode::Divide);
            break;
        case TokenType::MODULI:
            emit(OpCode::Modulo);
            break;
        case TokenType::POWER:
            emit(OpCode::Power);
            break;

        // --- Comparisons ---
        case TokenType::VALUE_EQUAL:
            emit(OpCode::Equal);
            break;
        case TokenType::REFERENCE_EQUAL:
            emit(OpCode::StrictEqual);
            break;
        case TokenType::INEQUALITY:
            emit(OpCode::NotEqual);
            break;
        case TokenType::STRICT_INEQUALITY:
            emit(OpCode::StrictNotEqual);
            break;
        case TokenType::LESS_THAN:
            emit(OpCode::LessThan);
            break;
        case TokenType::LESS_THAN_EQUAL:
            emit(OpCode::LessThanOrEqual);
            break;
        case TokenType::GREATER_THAN:
            emit(OpCode::GreaterThan);
            break;
        case TokenType::GREATER_THAN_EQUAL:
            emit(OpCode::GreaterThanOrEqual);
            break;

        // --- Logical ---
        case TokenType::LOGICAL_AND:
            emit(OpCode::LogicalAnd);
            break;
        case TokenType::LOGICAL_OR:
            emit(OpCode::LogicalOr);
            break;
        case TokenType::NULLISH_COALESCING:
            emit(OpCode::NullishCoalescing);
            break;

        // --- Bitwise ---
        case TokenType::BITWISE_AND:
            emit(OpCode::BitAnd);
            break;
        case TokenType::BITWISE_OR:
            emit(OpCode::BitOr);
            break;
        case TokenType::BITWISE_XOR:
            emit(OpCode::BitXor);
            break;
        case TokenType::BITWISE_LEFT_SHIFT:
            emit(OpCode::ShiftLeft);
            break;
        case TokenType::BITWISE_RIGHT_SHIFT:
            emit(OpCode::ShiftRight);
            break;
        case TokenType::UNSIGNED_RIGHT_SHIFT:
            emit(OpCode::UnsignedShiftRight);
            break;
            
        case TokenType::INSTANCEOF:
            emit(OpCode::InstanceOf);
            break;

        default:
            throw std::runtime_error("Unknown binary operator in compiler: " + expr->op.lexeme);
    }
    
    return true;

}

void CodeGen::emitAssignment(BinaryExpression* expr) {
    auto left = expr->left.get();

    // Plain assignment (=)
    if (expr->op.type == TokenType::ASSIGN) {
        
        if (auto classExpr = dynamic_cast<ClassExpression*>(expr->right.get())) {
            // Try to infer name from left
            if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
                classExpr->name = idExpr->name;
            } else if (auto memberExpr = dynamic_cast<MemberExpression*>(expr->left.get())) {
                classExpr->name = evaluate_property(memberExpr); // e.g. obj.B
            }
        } else if (auto functionExpr = dynamic_cast<FunctionExpression*>(expr->right.get())) {
            if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
                functionExpr->name = idExpr->name;
            } else if (auto memberExpr = dynamic_cast<MemberExpression*>(expr->left.get())) {
                functionExpr->name = evaluate_property(memberExpr);
            }
        } else if (auto arrowFunctionExpr = dynamic_cast<ArrowFunction*>(expr->right.get())) {
            if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
                arrowFunctionExpr->name = idExpr->name;
            } else if (auto memberExpr = dynamic_cast<MemberExpression*>(expr->left.get())) {
                arrowFunctionExpr->name = evaluate_property(memberExpr);
            }
        }
        
        if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
            // Evaluate RHS first
            expr->right->accept(*this);

            // Assign to variable (local/global/class field/upvalue)
            // define(ident->token.lexeme);
            store(ident->token.lexeme);
            
        }
        else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
            // Assignment to property (obj.prop = ...)

            // Push object first
            member->object->accept(*this);

            if (member->computed) {
                // Computed property: obj[expr] = rhs
                member->property->accept(*this); // push property key

                // Evaluate RHS
                expr->right->accept(*this);

                emit(OpCode::SetPropertyDynamic);
            } else {

                // Evaluate RHS
                expr->right->accept(*this);

                // Static property name (e.g. obj.x = rhs)
                int nameIdx = emitConstant(Value::str(member->name.lexeme));
                emit(OpCode::SetProperty);
                emitUint32(nameIdx);
            }
        }
        else {
            throw std::runtime_error("Unsupported assignment target in CodeGen");
        }

        // Assignments as statements typically discard result
        // emit(OpCode::Pop);

        return;
    }

    // Compound assignment (+=, -=, etc.)
    
    if (auto classExpr = dynamic_cast<ClassExpression*>(expr->right.get())) {
        // Try to infer name from left
        if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
            classExpr->name = idExpr->name;
        } else if (auto memberExpr = dynamic_cast<MemberExpression*>(expr->left.get())) {
            classExpr->name = evaluate_property(memberExpr); // e.g. obj.B
        }
    } else if (auto functionExpr = dynamic_cast<FunctionExpression*>(expr->right.get())) {
        if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
            functionExpr->name = idExpr->name;
        } else if (auto memberExpr = dynamic_cast<MemberExpression*>(expr->left.get())) {
            functionExpr->name = evaluate_property(memberExpr);
        }
    } else if (auto arrowFunctionExpr = dynamic_cast<ArrowFunction*>(expr->right.get())) {
        if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
            arrowFunctionExpr->name = idExpr->name;
        } else if (auto memberExpr = dynamic_cast<MemberExpression*>(expr->left.get())) {
            arrowFunctionExpr->name = evaluate_property(memberExpr);
        }

    }

    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
        // Load current value
        ident->accept(*this);
    }
    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
        // Load current property value
        member->object->accept(*this);
        if (member->computed) {
            member->property->accept(*this);
            emit(OpCode::Dup2);
            emit(OpCode::GetPropertyDynamic);
        } else {
            emit(OpCode::Dup); // [obj, obj]
            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::GetProperty);
            emitUint32(nameIdx);
        }
    }
    else {
        throw std::runtime_error("Unsupported assignment target in CodeGen");
    }

    // Evaluate RHS
    expr->right->accept(*this);

    // Apply compound operation
    switch (expr->op.type) {
        case TokenType::ASSIGN_ADD: emit(OpCode::Add); break;
        case TokenType::ASSIGN_MINUS: emit(OpCode::Subtract); break;
        case TokenType::ASSIGN_MUL: emit(OpCode::Multiply); break;
        case TokenType::ASSIGN_DIV: emit(OpCode::Divide); break;
        case TokenType::MODULI_ASSIGN: emit(OpCode::Modulo); break;
        case TokenType::POWER_ASSIGN: emit(OpCode::Power); break;
        case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: emit(OpCode::ShiftLeft); break;
        case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: emit(OpCode::ShiftRight); break;
        case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: emit(OpCode::UnsignedShiftRight); break;
        case TokenType::BITWISE_AND_ASSIGN: emit(OpCode::BitAnd); break;
        case TokenType::BITWISE_OR_ASSIGN: emit(OpCode::BitOr); break;
        case TokenType::BITWISE_XOR_ASSIGN: emit(OpCode::BitXor); break;
        case TokenType::LOGICAL_AND_ASSIGN: emit(OpCode::LogicalAnd); break;
        case TokenType::LOGICAL_OR_ASSIGN: emit(OpCode::LogicalOr); break;
        case TokenType::NULLISH_COALESCING_ASSIGN: emit(OpCode::NullishCoalescing); break;
        default: throw std::runtime_error("Unknown compound assignment operator in emitAssignment");
    }

    // Store the result back
    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
        // define(ident->token.lexeme);
        store(ident->token.lexeme);
    }
    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
        if (member->computed) {
            emit(OpCode::SetPropertyDynamic);
        } else {
            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::SetProperty);
            emitUint32(nameIdx);
        }
    }

}

// returns new value
// --x, ++x, !x, -x
R CodeGen::visitUnary(UnaryExpression* expr) {
    
    // DELETE
    if (auto member = dynamic_cast<MemberExpression*>(expr->right.get())) {
        
        (member->object->accept(*this));
        
        if (member->computed) {

            (member->property->accept(*this));

            emit(OpCode::Delete);
            
        } else {
            
            int nameConst = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::LoadConstant);
            emitUint32(nameConst);

            emit(OpCode::Delete);
            
        }
        
    } else {
        throw runtime_error("delete operator works on objects and arrays.");
    }
    
    // For prefix unary ops that target identifiers or members, we need special handling.
    if (expr->op.type == TokenType::INCREMENT || expr->op.type == TokenType::DECREMENT) {
        // ++x or --x
        if (auto ident = dynamic_cast<IdentifierExpression*>(expr->right.get())) {
            // load current value
            ident->accept(*this);
//            if (hasLocal(ident->name)) {
//                emit(OpCode::OP_GET_LOCAL);
//                emitUint32(getLocal(ident->name));
//            } else {
//                int nameIdx = emitConstant(Value::str(ident->name));
//                emit(OpCode::OP_GET_GLOBAL);
//                emitUint32(nameIdx);
//            }
            // push 1
            emit(OpCode::LoadConstant);
            emitUint32(emitConstant(Value::number(1)));
            // apply
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::Add : OpCode::Subtract);
            
            // store back
            // define(ident->token.lexeme);
            store(ident->token.lexeme);
            
            return true;
            
        }

        if (auto member_expr = dynamic_cast<MemberExpression*>(expr->right.get())) {
            
            if (member_expr->computed) {
                // Computed: arr[i]++ or obj[prop]++
                member_expr->object->accept(*this);     // [obj]
                member_expr->property->accept(*this);   // [obj, key]
                emit(OpCode::Dup2);             // [obj, key, obj, key]
                emit(OpCode::GetPropertyDynamic); // [obj, key, value]
                emit(OpCode::LoadConstant);
                emitUint32(emitConstant(Value::number(1)));
                emit(expr->op.type == TokenType::INCREMENT ? OpCode::Add : OpCode::Subtract); // [obj, key, result]
                emit(OpCode::SetPropertyDynamic); // [result]
            } else {
                // Non-computed: obj.x++
                member_expr->object->accept(*this); // [obj]
                emit(OpCode::Dup);          // [obj, obj]
                int nameIdx = emitConstant(Value::str(member_expr->name.lexeme));
                emit(OpCode::GetProperty);
                emitUint32(nameIdx);           // [obj, value]
                emit(OpCode::LoadConstant);
                emitUint32(emitConstant(Value::number(1)));
                emit(expr->op.type == TokenType::INCREMENT ? OpCode::Add : OpCode::Subtract); // [obj, result]
                emit(OpCode::SetProperty);
                emitUint32(nameIdx);           // [result]
            }
            
            return true;

        }

        throw std::runtime_error("Unsupported unary increment/decrement target");
    }

    // non-targeted unary ops (prefix) evaluate their operand first
    expr->right->accept(*this);
    switch (expr->op.type) {
        case TokenType::LOGICAL_NOT: emit(OpCode::LogicalNot); break;
        case TokenType::MINUS: emit(OpCode::Negate); break;
        case TokenType::BITWISE_NOT: emit(OpCode::LogicalNot); break;
        case TokenType::ADD: emit(OpCode::Positive); break;
        case TokenType::TYPEOF: emit(OpCode::TypeOf); break;
        default: throw std::runtime_error("Unsupported unary op in CodeGen");
    }
    return true;
}

R CodeGen::visitLiteral(LiteralExpression* expr) {
    int idx = emitConstant(Value::str(expr->token.lexeme));
    emit(OpCode::LoadConstant);
    emitUint32(idx);
    return true;
}

R CodeGen::visitNumericLiteral(NumericLiteral* expr) {
    Value v = toValue(expr->value);
    int idx = emitConstant(v);
    emit(OpCode::LoadConstant);
    emitUint32(idx);
    return true;
}

R CodeGen::visitStringLiteral(StringLiteral* expr) {
    int idx = emitConstant(Value::str(expr->text)); // sets the text to the constant array
    emit(OpCode::LoadConstant); // bytecode that indicates push constant to the stack.
    emitUint32(idx); // the index of the constant in the constants array to push to the stack
    return true;
}

// Here, we retrieve the identifier from its storage: local, upvalue or global.
R CodeGen::visitIdentifier(IdentifierExpression* expr) {
    string name = expr->name;
    
    load(name);

    return true;
    
}

R CodeGen::visitCall(CallExpression* expr) {
    // emit callee, then args left-to-right, then OP_CALL argc
    
//    if (classInfo.fields.size() > 0 && classInfo.fields.count("constructor")) {
//        // check if this is a super() call.
//        if (auto ident = dynamic_cast<SuperExpression*>(expr->callee.get())) {
//            
//            for (auto &arg : expr->arguments) {
//                arg->accept(*this);
//            }
//            uint8_t argc = (uint8_t)expr->arguments.size();
//            
//            emit(OpCode::SuperCall);
//            emitUint8(argc);
//            
//            return true;
//            
//        }
//    }
    
    bool isSuperCall = false;
    if (auto ident = dynamic_cast<SuperExpression*>(expr->callee.get())) {
        isSuperCall = true;
    }
    
    !isSuperCall ? expr->callee->accept(*this) : NULL;
    
    for (auto &arg : expr->arguments) {
        arg->accept(*this);
    }
    uint8_t argc = (uint8_t)expr->arguments.size();

    // check if this is a super() call.
    if (isSuperCall) {
        emit(OpCode::SuperCall);
    } else {
        emit(OpCode::Call);
    }
    
    emitUint8(argc);
    return true;
}

R CodeGen::visitMember(MemberExpression* expr) {
    // produce (object) then GetProperty name
    expr->object->accept(*this);

    if (expr->computed) {
        // compute property expression now
        expr->property->accept(*this);
        emit(OpCode::GetPropertyDynamic);
    } else {
        string propName = expr->name.lexeme;
        int nameIdx = emitConstant(Value::str(propName));
        emit(OpCode::GetProperty);
        emitUint32(nameIdx);
    }

    return true;
    
}

R CodeGen::visitArray(ArrayLiteralExpression* expr) {
    emit(OpCode::NewArray);
    for (auto &el : expr->elements) {
        el->accept(*this); // push element
        emit(OpCode::ArrayPush);
    }
    return true;
}

R CodeGen::visitObject(ObjectLiteralExpression* expr) {
    emit(OpCode::NewObject);
    emit(OpCode::CreateObjectLiteral);

    for (auto &prop : expr->props) {
        // evaluate value
        prop.second->accept(*this);
        int nameIdx = emitConstant(Value::str(prop.first.lexeme));
        emit(OpCode::CreateObjectLiteralProperty);
        emitUint32(nameIdx);
    }
        
    return true;
}

R CodeGen::visitConditional(ConditionalExpression* expr) {
    expr->test->accept(*this);
    int elseJump = emitJump(OpCode::JumpIfFalse);
    expr->consequent->accept(*this);
    int endJump = emitJump(OpCode::Jump);
    patchJump(elseJump);
    expr->alternate->accept(*this);
    patchJump(endJump);
    return true;
}

R CodeGen::visitArrowFunction(ArrowFunction* expr) {
    
    // Create a nested CodeGen for the function body
    CodeGen nested(module_);
    nested.cur = make_shared<Chunk>();
    nested.enclosing = this;
    nested.beginScope();
    nested.cur->name = expr->name;

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
            nested.emit(OpCode::LoadArguments);      // Push arguments array
            nested.emit(OpCode::LoadConstant);            // Push i
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::Slice);               // arguments.slice(i)
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));

            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            nested.emit(OpCode::LoadArgumentsLength);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::GreaterThan);
            int useArg = nested.emitJump(OpCode::JumpIfFalse);

            // Use argument
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::Jump);

            // Use default
            nested.patchJump(useArg);
            // Evaluate default expression (can reference previous params!)
            info.defaultExpr->accept(nested);

            // Set local either way
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));

        } else {
            // Direct: assign argument i to local slot
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
            
        }
    }

    // Compile function body
    if (expr->exprBody) {
        expr->exprBody->accept(nested);
        nested.emit(OpCode::Return);
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
            nested.emit(OpCode::LoadConstant);
            int ud = nested.emitConstant(Value::undefined());
            nested.emitUint32(ud);
            nested.emit(OpCode::Return);
        }
        
    }

    // Register chunk & emit as constant
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();

    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = expr->name;
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

    // emit(OpCode::OP_LOAD_CHUNK_INDEX);
    emit(OpCode::CreateClosure);
    emitUint8((uint8_t)ci);
    
    ClosureInfo closure_info = { (uint8_t)ci, nested.upvalues };

    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
    }

    // gather createclosure info for dissaemble
    closure_infos[to_string(ci)] = closure_info;
    
    if (scopeDepth == 0) {
        
    } else {
        // declareLocal(stmt->id);
        // int slot = paramSlot(stmt->id);
        // emit(OpCode::StoreLocal);
        // int nameIdx = emitConstant(Value::str(stmt->id));
        // emitUint32(slot);
    }
    
    disassembleChunk(nested.cur.get(), nested.cur->name);

    return true;
    
}

R CodeGen::visitFunctionExpression(FunctionExpression* expr) {
    
//    Token token;
//    vector<unique_ptr<Expression>> params;
//    unique_ptr<Statement> body;
//    bool is_async;

    // Create a nested CodeGen for the function body
    CodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = make_shared<Chunk>();
    nested.beginScope();
    nested.cur->name = expr->name;

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
            nested.emit(OpCode::LoadArguments);      // Push arguments array
            nested.emit(OpCode::LoadConstant);            // Push i
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::Slice);               // arguments.slice(i)
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
            // nested.emit(OpCode::OP_DEFINE_GLOBAL);
            // nested.emitUint32(nested.emitConstant(Value::str(info.name)));
            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            nested.emit(OpCode::LoadArgumentsLength);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::GreaterThan);
            int useArg = nested.emitJump(OpCode::JumpIfFalse);

            // Use argument
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::Jump);

            // Use default
            nested.patchJump(useArg);
            // Evaluate default expression (can reference previous params!)
            info.defaultExpr->accept(nested);

            // Set local either way
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
            // nested.emit(OpCode::OP_DEFINE_GLOBAL);
            // nested.emitUint32(nested.emitConstant(Value::str(info.name)));
        } else {
            // Direct: assign argument i to local slot
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
            // nested.emit(OpCode::OP_DEFINE_GLOBAL);
            // nested.emitUint32(nested.emitConstant(Value::str(info.name)));

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
            nested.emit(OpCode::LoadConstant);
            int ud = nested.emitConstant(Value::undefined());
            nested.emitUint32(ud);
            nested.emit(OpCode::Return);
        }

    }

    // Register chunk & emit as constant
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();

    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = std::make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = expr->name;
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

    // emit(OpCode::OP_LOAD_CHUNK_INDEX);
    emit(OpCode::CreateClosure);
    emitUint8((uint8_t)ci);

    ClosureInfo closure_info = { (uint8_t)ci, nested.upvalues };

    // Emit upvalue descriptors
    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
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

    closure_infos[to_string(ci)] = closure_info;
    disassembleChunk(nested.cur.get(), nested.cur->name);

    return true;
}

R CodeGen::visitFunction(FunctionDeclaration* stmt) {
    // Create a nested code generator for the function body
    CodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = std::make_shared<Chunk>();
    nested.beginScope();
    nested.cur->name = stmt->id;
    
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
        if (info.isRest) {
            // Collect rest arguments as array: arguments.slice(i)
            nested.emit(OpCode::LoadArguments);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::Slice);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
            continue;
        }
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            nested.emit(OpCode::LoadArgumentsLength);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::GreaterThan);
            int useArg = nested.emitJump(OpCode::JumpIfFalse);

            // Use argument
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::Jump);

            // Use default
            nested.patchJump(useArg);
            info.defaultExpr->accept(nested);

            // Set local either way
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
        } else {
            // Direct: assign argument i to local slot
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));

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
            nested.emit(OpCode::LoadConstant);
            int ud = nested.emitConstant(Value::undefined());
            nested.emitUint32(ud);
            nested.emit(OpCode::Return);
        }

    }

    // Register chunk & emit as constant
    auto fnChunk = nested.cur;
    fnChunk->arity = (uint32_t)paramNames.size();

    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = std::make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = stmt->id;
    fnObj->upvalues_size = (uint32_t)nested.upvalues.size();

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);

    emit(OpCode::CreateClosure);
    emitUint8((uint8_t)ci);

    ClosureInfo closure_info = { (uint8_t)ci, nested.upvalues };

    // Emit upvalue descriptors
    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
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
    
    BindingKind functionBinding = scopeDepth == 0 ? BindingKind::Var : BindingKind::Let;
    declareLocal(stmt->id, functionBinding);
    declareGlobal(stmt->id, functionBinding);
    
    create(stmt->id, functionBinding);

    closure_infos[to_string(ci)] = closure_info;
    
    // disassemble the chunk for debugging
    disassembleChunk(nested.cur.get(),
                     stmt->id/*nested.cur->name*/);

    return true;
}

R CodeGen::visitTemplateLiteral(TemplateLiteral* expr) {
    // produce string by concatenating quasis and evaluated expressions.
    // Simple approach: compute at runtime building string.
    // push empty string
    int emptyIdx = emitConstant(Value::str(""));
    emit(OpCode::LoadConstant);
    emitUint32(emptyIdx);

    size_t qsize = expr->quasis.size();
    size_t esize = expr->expressions.size();

    for (size_t i = 0; i < qsize; ++i) {
        // push quasi
        int qi = emitConstant(Value::str(expr->quasis[i]->text));
        emit(OpCode::LoadConstant);
        emitUint32(qi);
        emit(OpCode::Add);
        // if expression exists
        if (i < esize) {
            expr->expressions[i]->accept(*this);
            emit(OpCode::Add);
        }
    }
    return true;
}

string CodeGen::resolveImportPath(ImportDeclaration* stmt) {
    
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

R CodeGen::visitImportDeclaration(ImportDeclaration* stmt) {
    
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

// TODO: check to make sure this is never used.
// It is like assignments is visitBinary.
R CodeGen::visitAssignment(AssignmentExpression* expr) {
    // Evaluate right-hand side
    expr->right->accept(*this);

    // Assign to variable or property
    if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->left.get())) {
        ident->accept(*this);
//        if (hasLocal(ident->name)) {
//            emit(OpCode::OP_SET_LOCAL);
//            emitUint32(getLocal(ident->name));
//        } else {
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::OP_SET_GLOBAL);
//            emitUint32(nameIdx);
//        }
    } else if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->left.get())) {
        // obj.prop = value
        // Evaluate object
        member->object->accept(*this);
        // Swap stack: [value,obj] -> [obj,value]
        emit(OpCode::Dup);
        emit(OpCode::Pop);
        // Push property name
        int nameIdx = emitConstant(Value::str(member->name.lexeme));
        emit(OpCode::SetProperty);
        emitUint32(nameIdx);
    } else {
        throw std::runtime_error("Unsupported assignment target in CodeGen");
    }
    return true;
}

R CodeGen::visitLogical(LogicalExpression* expr) {
    expr->left->accept(*this);
    if (expr->op.type == TokenType::LOGICAL_OR) {
        int endJump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);
        expr->right->accept(*this);
        patchJump(endJump);
    } else if (expr->op.type == TokenType::LOGICAL_AND) {
        int endJump = emitJump(OpCode::JumpIfFalse);
        expr->right->accept(*this);
        patchJump(endJump);
    } else {
        throw std::runtime_error("Unsupported logical operator");
    }
    return true;
}

R CodeGen::visitThis(ThisExpression* expr) {
    // Assumes 'this' is always local 0 in method frames
    emit(OpCode::GetThis);
    // emit(OpCode::OP_GET_LOCAL);
    // emitUint32(0); // slot 0 reserved for 'this'
    return true;
}

// TODO: check to make sure this is never used.
R CodeGen::visitProperty(PropertyExpression* expr) {
//    expr->object->accept(*this);
//    int nameIdx = emitConstant(Value::str(expr->name.lexeme));
//    emit(OpCode::OP_GET_PROPERTY);
//    emitUint32(nameIdx);
    return true;
}

R CodeGen::visitSequence(SequenceExpression* expr) {
    size_t n = expr->expressions.size();
    for (size_t i = 0; i < n; ++i) {
        expr->expressions[i]->accept(*this);
        if (i + 1 < n) emit(OpCode::Pop);
    }
    return true;
}

// returns old value
R CodeGen::visitUpdate(UpdateExpression* expr) {
    // Identifiers: x++
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
        ident->accept(*this);
        emit(OpCode::Dup); // did this, so we leave the old alue on stack
//        if (hasLocal(ident->name)) {
//            emit(OpCode::OP_GET_LOCAL);
//            emitUint32(getLocal(ident->name));
//        } else {
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::OP_GET_GLOBAL);
//            emitUint32(nameIdx);
//        }
        emit(OpCode::LoadConstant);
        emitUint32(emitConstant(Value::number(1)));
        emit(expr->op.type == TokenType::INCREMENT ? OpCode::Add : OpCode::Subtract);
        // define(ident->name);
        store(ident->name);
//        if (hasLocal(ident->name)) {
//            emit(OpCode::StoreLocal);
//            emitUint32(getLocal(ident->name));
//        } else {
//            
//            int upvalue = resolveUpvalue(ident->name);
//            if (upvalue != -1) {
//                emit(OpCode::SetUpvalue);
//                emitUint32(upvalue);
//                return R();
//            }
//
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::StoreGlobal);
//            emitUint32(nameIdx);
//        }
        
        return true;
    }
    // Member expressions: obj.x++, arr[i]++, obj[prop]++
    if (auto member = dynamic_cast<MemberExpression*>(expr->argument.get())) {
        if (member->computed) {
            // Computed: arr[i]++ or obj[prop]++
            member->object->accept(*this);     // [obj]
            member->property->accept(*this);   // [obj, key]
            emit(OpCode::GetPropertyDynamic); // [value]

            member->object->accept(*this);     // [obj]
            member->property->accept(*this);   // [obj, key]
            emit(OpCode::Dup2);             // [obj, key, obj, key]
            emit(OpCode::GetPropertyDynamic); // [obj, key, value]
            emit(OpCode::LoadConstant);
            emitUint32(emitConstant(Value::number(1))); // [obj, key, value, 1]
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::Add : OpCode::Subtract); // [obj, key, result]
            emit(OpCode::SetPropertyDynamic); // [result]
            
            emit(OpCode::Pop);

            return true;
        } else {
            // Non-computed: obj.x++
            member->object->accept(*this); // [obj]
            emit(OpCode::GetProperty);
            emitUint32(emitConstant(Value::str(member->name.lexeme)));           // [old_value]

            member->object->accept(*this); // [old_value, obj]
            emit(OpCode::Dup);          // [old_value, obj, obj]
            emit(OpCode::GetProperty);
            emitUint32(emitConstant(Value::str(member->name.lexeme)));           // [old_value, obj, value]
            emit(OpCode::LoadConstant);
            emitUint32(emitConstant(Value::number(1))); // [old_value, obj, value, 1]
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::Add : OpCode::Subtract); // [old_value, obj, result]
            emit(OpCode::SetProperty);
            emitUint32(emitConstant(Value::str(member->name.lexeme)));           // [old_value, result]
            
            emit(OpCode::Pop); // [old_value]

            return true;
        }
    }
    throw std::runtime_error("Update target must be identifier or member expression (including computed/array member)");
}

R CodeGen::visitFalseKeyword(FalseKeyword* expr) {
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::boolean(false)));
    return true;
}

R CodeGen::visitTrueKeyword(TrueKeyword* expr) {
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::boolean(true)));
    return true;
}

R CodeGen::visitNullKeyword(NullKeyword* expr) {
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::nullVal()));
    return true;
}

R CodeGen::visitUndefinedKeyword(UndefinedKeyword* expr) {
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::undefined()));
    return true;
}

R CodeGen::visitAwaitExpression(AwaitExpression* expr) {
    // Evaluate argument
    //expr->argument->accept(*this);
    // Call a built-in 'await' handler or emit await-specific opcode, e.g.:
    //emit(OpCode::OP_AWAIT);
    return true;
}

R CodeGen::visitBreak(BreakStatement* stmt) {
    if (loopStack.empty()) {
        throw ("Break outside loop");
        return false;
    }
    // Emit jump with unknown target
    int jumpAddr = emitJump(OpCode::Jump);
    loopStack.back().breaks.push_back(jumpAddr);
    return true;
}

R CodeGen::visitContinue(ContinueStatement* stmt) {
    if (loopStack.empty()) {
        throw ("Continue outside loop");
        return false;
    }

    // int loopStart = loopStack.back().loopStart;
    // emitLoop(loopStart); // emit a backwards jump

    int jumpAddr = emitJump(OpCode::Jump);
    loopStack.back().continues.push_back(jumpAddr);

    return true;
}

R CodeGen::visitThrow(ThrowStatement* stmt) {
    // Evaluate the exception value
    stmt->argument->accept(*this);
    emit(OpCode::Throw);
    return true;
}

R CodeGen::visitEmpty(EmptyStatement* stmt) {
    return true;
}

R CodeGen::visitSuper(SuperExpression* stmt) {
    // super points to the parent js_object
    emit(OpCode::GetParentObject);
    return true;
}

// TODO: figure out how to use arguments and call constructor
R CodeGen::visitNew(NewExpression* expr) {
    
    // create new object, push, then call constructor
        
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->callee.get())) {
                
        ident->accept(*this);
        
        emit(OpCode::CreateInstance);

        for (auto& arg : expr->arguments) {
            arg->accept(*this);
        }
        
        emit(OpCode::InvokeConstructor);

        // emit args count
        emitUint8((uint8_t)expr->arguments.size());

    }
    
    return true;
}

void CodeGen::compileMethod(MethodDefinition& method) {
    
    // Create a nested CodeGen for the method body (closure)
    CodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = std::make_shared<Chunk>();
    nested.beginScope();
    nested.classInfo = classInfo;
    nested.classes = classes;

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
        if (info.isRest) {
            nested.emit(OpCode::LoadArguments);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::Slice);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
            continue;
        }
        if (info.hasDefault) {
            nested.emit(OpCode::LoadArgumentsLength);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::GreaterThan);
            int useArg = nested.emitJump(OpCode::JumpIfFalse);
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::Jump);
            nested.patchJump(useArg);
            info.defaultExpr->accept(nested);
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
        } else {
            nested.emit(OpCode::LoadArgument);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::StoreLocal);
            nested.emitUint32(nested.getLocal(info.name));
        }
    }

    // Compile the method body
    if (method.methodBody) {
        method.methodBody->accept(nested);
        // Ensure Return is emitted
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
            nested.emit(OpCode::LoadConstant);
            int ud = nested.emitConstant(Value::undefined());
            nested.emitUint32(ud);
            nested.emit(OpCode::Return);
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
    emit(OpCode::CreateClosure);
    emitUint8((uint8_t)ci);

    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
    }
    
    disassembleChunk(nested.cur.get(), method.name);

}

R CodeGen::visitClass(ClassDeclaration* stmt) {
        
    classInfo.name = stmt->id;

    // Evaluate superclass (if any)
    if (stmt->superClass) {
        stmt->superClass->accept(*this); // [superclass]
        
        auto ident = dynamic_cast<IdentifierExpression*>((stmt->superClass.get()));
        
        if (ident) {
            classInfo.super_class_name = ident->name;
        }

    } else {
        emit(OpCode::LoadConstant);
        emitUint32(emitConstant(Value::nullVal())); // or Value::undefined()
    }

    // Create the class object (with superclass on stack)
    emit(OpCode::NewClass); // pops superclass, pushes new class object

    // Define fields
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
                
                // ************ Collect property meta ****************
                auto visibility = isProtected ? Visibility::Protected : isPrivate ? Visibility::Private : Visibility::Public;
                auto binding = get_kind(kind);
                
                PropertyMeta prop_meta = { visibility, binding, isStatic };
                classInfo.fields[decl.id] = prop_meta;
                // ****************************************************

                if (isStatic) {
                    
                    if (decl.init) {
                        decl.init->accept(*this); // Evaluate initializer
                    } else {
                        emit(OpCode::LoadConstant);
                        emitUint32(emitConstant(Value::undefined()));
                    }
                    
                } else {
                    recordInstanceField(stmt->id, decl.id, decl.init.get(), prop_meta);
                }
                
                int nameIdx = emitConstant(Value::str(decl.id));
                
                OpCode op;

                if (isStatic) {
                                        
                    switch (get_kind(kind)) {
                        case BindingKind::Var:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicStaticPropertyVar;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivateStaticPropertyVar;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedStaticPropertyVar;
                            } else {
                                op = OpCode::CreateClassPublicStaticPropertyVar;
                            }
                            break;
                        case BindingKind::Const:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicStaticPropertyConst;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivateStaticPropertyConst;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedStaticPropertyConst;
                            } else {
                                op = OpCode::CreateClassPublicStaticPropertyConst;
                            }
                            break;
                        default:
                            throw runtime_error("Fields in classes must have a Binding kind. e.g var.");
                            break;
                    }
                    
                    // emit(OpCode::SetStaticProperty);
                    emit(op);

                } else {
                    
                    switch (get_kind(kind)) {
                        case BindingKind::Var:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicPropertyVar;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivatePropertyVar;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedPropertyVar;
                            } else {
                                op = OpCode::CreateClassPublicPropertyVar;
                            }
                            break;
                        case BindingKind::Const:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicPropertyConst;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivatePropertyConst;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedPropertyConst;
                            } else {
                                op = OpCode::CreateClassPublicPropertyConst;
                            }
                            break;
                        default:
                            throw runtime_error("Fields in classes must have a Binding kind. e.g var.");
                            break;
                    }

                    // emit(OpCode::SetProperty);
                    emit(op);
                }
                
                emitUint32(nameIdx);
            }
        }
    }
    
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

        // ************* Collecting properrty meta ***************
        auto visibility = isProtected ? Visibility::Protected : isPrivate ? Visibility::Private : Visibility::Public;
        
        PropertyMeta prop_meta = { visibility, BindingKind::Var, isStatic };
        
        classInfo.fields[method->name] = prop_meta;
        // ****************************

    }

    // Define methods (attach to class or prototype as appropriate)
    for (auto& method : stmt->body) {
        // Compile the method as a function object
        compileMethod(*method); // leaves function object on stack

        // Get name of method
        int nameIdx = emitConstant(Value::str(method->name));
        
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
        
        OpCode op;

        if (isStatic) {
            
            if (isPublic) {
                op = OpCode::CreateClassPublicStaticMethod;
            } else if (isPrivate) {
                op = OpCode::CreateClassPrivateStaticMethod;
            } else if (isProtected) {
                op = OpCode::CreateClassProtectedStaticMethod;
            } else {
                op = OpCode::CreateClassPublicStaticMethod;
            }
            
            // emit(OpCode::SetStaticProperty);
        } else {
            
            if (isPublic) {
                op = OpCode::CreateClassPublicMethod;
            } else if (isPrivate) {
                op = OpCode::CreateClassPrivateMethod;
            } else if (isProtected) {
                op = OpCode::CreateClassProtectedMethod;
            } else {
                op = OpCode::CreateClassPublicMethod;
            }
            
            // emit(OpCode::SetProperty);
        }
        
        emit(op);
        emitUint32(nameIdx); // Pops class and function, sets on prototype

    }

    // TODO: need to fix this to store not only in global
    // Bind class in the environment (global)
    // int classNameIdx = emitConstant(Value::str(stmt->id));
    // emit(OpCode::CreateGlobal);
    // emitUint32(classNameIdx);

    // add claas to classes
    classes[stmt->id] = std::move(classInfo);
    // clear class info
    classInfo.fields.clear();

    BindingKind classBinding = scopeDepth == 0 ? BindingKind::Var : BindingKind::Let;

    declareLocal(stmt->id, classBinding);
    declareGlobal(stmt->id, classBinding);
    create(stmt->id, classBinding);

    return true;
    
}

R CodeGen::visitMethodDefinition(MethodDefinition* stmt) {
    return true;
}

R CodeGen::visitDoWhile(DoWhileStatement* stmt) {

    beginLoop();
    beginScope();
    loopStack.back().loopStart = (int)cur->size();
    
    // Mark loop start
    size_t loopStart = cur->size();
    
    // Compile loop body
    stmt->body->accept(*this);

    if (loopStack.back().continues.size() > 0) {
        for (auto& continueAddr : loopStack.back().continues) {
            patchJump(continueAddr);
        }
        
        loopStack.back().continues.clear();
    }

    // Compile the condition
    stmt->condition->accept(*this);

    // Jump back to loop start if condition is true
    int condJump = emitJump(OpCode::JumpIfFalse);
    emit(OpCode::Pop); // pop condition

    // Emit loop back
    emitLoop((uint32_t)(cur->size() - loopStart + 4 + 1));

    // Patch the jump to after the loop if condition is false
    patchJump(condJump);
    emit(OpCode::Pop); // pop condition
    
    endScope();
    endLoop();

    return true;
}

R CodeGen::visitSwitchCase(SwitchCase* stmt) {
    // Each case's test should have already been checked in visitSwitch
    // Emit the body of this case
    beginScope();
    for (auto& s : stmt->consequent) {
        if (auto break_stmt = dynamic_cast<BreakStatement*>(s.get())) {
            continue;
        } else {
            s->accept(*this);

        }
    }
    endScope();
    return true;
}

R CodeGen::visitSwitch(SwitchStatement* stmt) {
    
    beginScope();

    std::vector<int> caseJumps;
    int defaultJump = -1;

    // Evaluate the discriminant and leave on stack
    stmt->discriminant->accept(*this);

    // Emit checks for each case
    for (size_t i = 0; i < stmt->cases.size(); ++i) {
        SwitchCase* scase = stmt->cases[i].get();
        if (scase->test) {
            // Duplicate discriminant for comparison
            emit(OpCode::Dup);
            scase->test->accept(*this);
            emit(OpCode::Equal);

            // If not equal, jump to next
            int jump = emitJump(OpCode::JumpIfFalse);
            emit(OpCode::Pop); // pop comparison result

            // If equal, pop discriminant, emit case body, and jump to end
            emit(OpCode::Pop);
            scase->accept(*this);
            caseJumps.push_back(emitJump(OpCode::Jump));
            patchJump(jump);
        } else {
            // Default case: remember its position
            defaultJump = (int)cur->size();
            // pop discriminant for default
            emit(OpCode::Pop);
            scase->accept(*this);
            // No jump needed after default
        }
    }

    // Patch all jumps to here (after switch body)
    for (int jmp : caseJumps) {
        patchJump(jmp);
    }

    endScope();

    return true;
}

R CodeGen::visitCatch(CatchClause* stmt) {
    beginScope();
    stmt->body->accept(*this);
    endScope();
    return true;
}

R CodeGen::visitTry(TryStatement* stmt) {
    beginScope();
    // Mark start of try
    int tryPos = emitTryPlaceholder();

    // Compile try block
    stmt->block->accept(*this);

    // End of try
    emit(OpCode::EndTry);

    int endJump = -1;

    // If there's a catch, emit jump over it for normal completion
    if (stmt->handler) {
        endJump = emitJump(OpCode::Jump);
        // Patch catch offset
        patchTryCatch(tryPos, (int)cur->size());

        beginScope();
        
        // Bind catch parameter (VM leaves exception value on stack)
        declareLocal(stmt->handler->param, BindingKind::Var);
        emitSetLocal(paramSlot(stmt->handler->param));

        stmt->handler->body->accept(*this);
        
        endScope();
        
    }

    // Jump over finally if we had a catch
    if (endJump != -1) {
        patchJump(endJump);
    }

    // If there's a finally, patch and emit it
    if (stmt->finalizer) {
        patchTryFinally(tryPos, (int)cur->size());

        stmt->finalizer->accept(*this);
        emit(OpCode::EndFinally);
    }
    endScope();

    return true;
}

R CodeGen::visitForIn(ForInStatement* stmt) {
    
    // object, body, init
    
    // loop through the keys of the object
    beginScope();
    stmt->init->accept(*this);
    // load object to stack
    stmt->object->accept(*this);
    
    emit(OpCode::Dup);
    // get keys of object
    emit(OpCode::EnumKeys);
    uint32_t keys_slot = makeLocal("__for_in_keys", BindingKind::Let);
    emit(OpCode::StoreLocalLet);
    emitUint32(keys_slot);

    emit(OpCode::Dup);
    emit(OpCode::GetObjectLength);
    
    uint32_t length_slot = makeLocal("__for_in_length", BindingKind::Let);
    emit(OpCode::StoreLocalLet);
    emitUint32(length_slot);

    uint32_t idx_slot = makeLocal("__for_in_idx", BindingKind::Let);
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::number(0)));
    emit(OpCode::StoreLocalLet);
    emitUint32(idx_slot);
        
    size_t loop_start = cur->size();
    beginLoop();
    loopStack.back().loopStart = (int)loop_start;

    // get both len and idx
    emit(OpCode::LoadLocalVar);
    emitUint32(idx_slot);

    emit(OpCode::LoadLocalVar);
    emitUint32(length_slot);

    emit(OpCode::NotEqual);
    int jump_if_false = emitJump(OpCode::JumpIfFalse);
    
    emit(OpCode::Pop);

    emit(OpCode::Dup);

    // Get key at idx: keys[idx]
    emit(OpCode::LoadLocalVar);
    emitUint32(keys_slot);
    emit(OpCode::LoadLocalVar);
    emitUint32(idx_slot);
    emit(OpCode::GetPropertyDynamic);

    // Assign value to loop variable
    if (auto* ident = dynamic_cast<IdentifierExpression*>(stmt->init.get())) {
        // uint32_t slot = makeLocal(ident->name, BindingKind::Let);
        // emit(OpCode::StoreLocalLet);
        // emitUint32(slot);
        store(ident->name);
    } else if (auto* var_stmt = dynamic_cast<VariableStatement*>(stmt->init.get())) {
        // uint32_t slot = makeLocal(var_stmt->declarations[0].id, get_kind(var_stmt->kind));
        // emit(OpCode::StoreLocal);
        // emitUint32(slot);
        store(var_stmt->declarations[0].id);
    } else if (auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt->init.get())) {
        
        if (auto* ident = dynamic_cast<IdentifierExpression*>(expr_stmt->expression.get())) {
            // uint32_t slot = makeLocal(ident->name, BindingKind::Let);
            // emit(OpCode::StoreLocalLet);
            // emitUint32(slot);
            store(ident->name);
        }
        
    } else {
        throw std::runtime_error("For...in only supports identifier/variable statement loop variables in codegen.");
    }
    
    // get size of the keys
    stmt->body->accept(*this);
    
    if (loopStack.back().continues.size() > 0) {
        for (auto& continueAddr : loopStack.back().continues) {
            patchJump(continueAddr);
        }
        
        loopStack.back().continues.clear();
    }

    // Increment idx
    emit(OpCode::LoadLocalVar);
    emitUint32(idx_slot);
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::number(1)));
    emit(OpCode::Add);
    emit(OpCode::StoreLocalLet);
    emitUint32(idx_slot);
    
    // loop till stack is empty
    emit(OpCode::Loop);
    emitUint32((int)cur->size() - (uint32_t)loop_start + 4 + 1);
    
    patchJump(jump_if_false);
    emit(OpCode::Pop);
    endScope();
    endLoop();
    emit(OpCode::ClearStack);
    return true;
    
}

// TODO: we need to make sure statement leaves nothing on stack
// TODO: for..of reads starts off by 1
R CodeGen::visitForOf(ForOfStatement* stmt) {

    // std::unique_ptr<Statement> left; // variable declaration or expression
    // std::unique_ptr<Expression> right; // iterable expression
    // std::unique_ptr<Statement> body;
    emit(OpCode::ClearStack);
    beginScope();
    
    stmt->left->accept(*this);

    // assume, right must directly hold an object literal
    stmt->right->accept(*this);
        
    emit(OpCode::Dup);

    size_t __for_of_array_slot = makeLocal("__for_of_array", BindingKind::Let);
    emit(OpCode::StoreLocalLet);
    emitUint32((uint32_t)__for_of_array_slot);

    emit(OpCode::GetObjectLength);
    // get array length
    size_t length_slot = makeLocal("__for_of_length", BindingKind::Let);
    emit(OpCode::StoreLocalLet);
    emitUint32((uint32_t)length_slot);
    // emit(OpCode::OP_POP);

    size_t idx_slot = makeLocal("__for_of_index", BindingKind::Let);
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value(0)));
    emit(OpCode::StoreLocalLet);
    emitUint32((uint32_t)idx_slot);

    int loop_start = (int)cur->size();
    
    beginLoop();
    loopStack.back().loopStart = loop_start;

    // get both len and idx
    emit(OpCode::LoadLocalVar);
    emitUint32((uint32_t)idx_slot);

    emit(OpCode::LoadLocalVar);
    emitUint32((uint32_t)length_slot);
    
    emit(OpCode::NotEqual);
    
    int jump_if_false = emitJump(OpCode::JumpIfFalse);

    // emit(OpCode::OP_DUP);
    emit(OpCode::LoadLocalVar);
    emitUint32((uint32_t)__for_of_array_slot);

    emit(OpCode::LoadLocalVar);
    emitUint32((uint32_t)idx_slot);
    emit(OpCode::GetPropertyDynamic);

    // Assign value to loop variable
    if (auto* ident = dynamic_cast<IdentifierExpression*>(stmt->left.get())) {
        // uint32_t slot = makeLocal(ident->name, BindingKind::Var);
        // emit(OpCode::StoreLocal);
        // emitUint32(slot);
        store(ident->name);
    } else if (auto* var_stmt = dynamic_cast<VariableStatement*>(stmt->left.get())) {
        // uint32_t slot = makeLocal(var_stmt->declarations[0].id, get_kind(var_stmt->kind));
        // emit(OpCode::StoreLocal);
        // emitUint32(slot);
        store(var_stmt->declarations[0].id);
    } else if (auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt->left.get())) {
        
        if (auto* ident = dynamic_cast<IdentifierExpression*>(expr_stmt->expression.get())) {
            // uint32_t slot = makeLocal(ident->name, BindingKind::Var);
            // emit(OpCode::StoreLocal);
            // emitUint32(slot);
            store(ident->name);
        }
        
    } else {
        throw std::runtime_error("For...of only supports identifier/variable statement loop variables in codegen.");
    }

    stmt->body->accept(*this);
    
    if (loopStack.back().continues.size() > 0) {
        for (auto& continueAddr : loopStack.back().continues) {
            patchJump(continueAddr);
        }
        
        loopStack.back().continues.clear();
    }

    // increment idx
    // push idx
    emit(OpCode::LoadLocalVar);
    emitUint32((uint32_t)idx_slot);
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::number(1)));
    emit(OpCode::Add);
    emit(OpCode::StoreLocalLet);
    emitUint32((uint32_t)idx_slot);

    emitLoop((int)cur->size() - loop_start + 4 + 1);
    patchJump(jump_if_false);
    
    endScope();
    endLoop();
    
    emit(OpCode::ClearStack);
    
    // we need to clear locals.

    return true;
    
}

R CodeGen::visitClassExpression(ClassExpression* expr) {
    
    auto stmt = expr;
    
    classInfo.name = expr->name;

    // Evaluate superclass (if any)
    if (stmt->superClass) {
        stmt->superClass->accept(*this); // [superclass]
        
        auto ident = dynamic_cast<IdentifierExpression*>((stmt->superClass.get()));
        
        if (ident) {
            classInfo.super_class_name = ident->name;
        }

    } else {
        emit(OpCode::LoadConstant);
        emitUint32(emitConstant(Value::nullVal())); // or Value::undefined()
    }

    // Create the class object (with superclass on stack)
    emit(OpCode::NewClass); // pops superclass, pushes new class object

    // Define fields
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
                
                // ************ Collect property meta ****************
                auto visibility = isProtected ? Visibility::Protected : isPrivate ? Visibility::Private : Visibility::Public;
                auto binding = get_kind(kind);
                
                PropertyMeta prop_meta = { visibility, binding, isStatic };
                classInfo.fields[decl.id] = prop_meta;
                // ****************************************************

                if (isStatic) {
                    
                    if (decl.init) {
                        decl.init->accept(*this); // Evaluate initializer
                    } else {
                        emit(OpCode::LoadConstant);
                        emitUint32(emitConstant(Value::undefined()));
                    }
                    
                } else {
                    recordInstanceField(stmt->name,
                                        decl.id,
                                        decl.init.get(),
                                        prop_meta);
                }
                
                int nameIdx = emitConstant(Value::str(decl.id));
                
                OpCode op;

                if (isStatic) {
                                        
                    switch (get_kind(kind)) {
                        case BindingKind::Var:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicStaticPropertyVar;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivateStaticPropertyVar;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedStaticPropertyVar;
                            } else {
                                op = OpCode::CreateClassPublicStaticPropertyVar;
                            }
                            break;
                        case BindingKind::Const:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicStaticPropertyConst;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivateStaticPropertyConst;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedStaticPropertyConst;
                            } else {
                                op = OpCode::CreateClassPublicStaticPropertyConst;
                            }
                            break;
                        default:
                            throw runtime_error("Fields in classes must have a Binding kind. e.g var.");
                            break;
                    }
                    
                    // emit(OpCode::SetStaticProperty);
                    emit(op);

                } else {
                    
                    switch (get_kind(kind)) {
                        case BindingKind::Var:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicPropertyVar;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivatePropertyVar;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedPropertyVar;
                            } else {
                                op = OpCode::CreateClassPublicPropertyVar;
                            }
                            break;
                        case BindingKind::Const:
                            if (isPublic) {
                                op = OpCode::CreateClassPublicPropertyConst;
                            } else if (isPrivate) {
                                op = OpCode::CreateClassPrivatePropertyConst;
                            } else if (isProtected) {
                                op = OpCode::CreateClassProtectedPropertyConst;
                            } else {
                                op = OpCode::CreateClassPublicPropertyConst;
                            }
                            break;
                        default:
                            throw runtime_error("Fields in classes must have a Binding kind. e.g var.");
                            break;
                    }

                    // emit(OpCode::SetProperty);
                    emit(op);
                }
                
                emitUint32(nameIdx);
            }
        }
    }
    
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

        // ************* Collecting properrty meta ***************
        auto visibility = isProtected ? Visibility::Protected : isPrivate ? Visibility::Private : Visibility::Public;
        
        PropertyMeta prop_meta = { visibility, BindingKind::Var, isStatic };
        
        classInfo.fields[method->name] = prop_meta;
        // ****************************

    }

    // Define methods (attach to class or prototype as appropriate)
    for (auto& method : stmt->body) {
        // Compile the method as a function object
        compileMethod(*method); // leaves function object on stack

        // Get name of method
        int nameIdx = emitConstant(Value::str(method->name));
        
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
        
        OpCode op;

        if (isStatic) {
            
            if (isPublic) {
                op = OpCode::CreateClassPublicStaticMethod;
            } else if (isPrivate) {
                op = OpCode::CreateClassPrivateStaticMethod;
            } else if (isProtected) {
                op = OpCode::CreateClassProtectedStaticMethod;
            } else {
                op = OpCode::CreateClassPublicStaticMethod;
            }
            
            // emit(OpCode::SetStaticProperty);
        } else {
            
            if (isPublic) {
                op = OpCode::CreateClassPublicMethod;
            } else if (isPrivate) {
                op = OpCode::CreateClassPrivateMethod;
            } else if (isProtected) {
                op = OpCode::CreateClassProtectedMethod;
            } else {
                op = OpCode::CreateClassPublicMethod;
            }
            
        }
        
        emit(op);
        emitUint32(nameIdx); // Pops class and function, sets on prototype

    }

    // add claas to classes
    classes[stmt->name] = std::move(classInfo);

    // clear class info
    classInfo.fields.clear();
    
    return true;
    
}

R CodeGen::visitUIExpression(UIViewExpression* visitor) {
    return true;
}

R CodeGen::visitEnumDeclaration(EnumDeclaration* stmt) {
    
    int nameIdx = emitConstant(Value::str(stmt->name));
    
    emit(OpCode::LoadConstant);
    emitUint32(nameIdx);
    
    emit(OpCode::CreateEnum);
    
    for(auto& member : stmt->members) {
        
        int propIndex = emitConstant(Value::str(member.name));
        emit(OpCode::LoadConstant);
        emitUint32(propIndex);
        
        if (member.value == nullptr) {
            
            int idx = emitConstant(Value::str(to_string(member.computedValue)));
            emit(OpCode::LoadConstant);
            emitUint32(idx);
            
        } else {
            member.value->accept(*this);
        }
        
        // stack: obj, property, value
        emit(OpCode::SetEnumProperty);
    }
    
    BindingKind enumBinding = scopeDepth == 0 ? BindingKind::Var : BindingKind::Let;
    
    declareLocal(stmt->name, enumBinding);
    declareGlobal(stmt->name, enumBinding);
    
    create(stmt->name, enumBinding);
    
    return true;
    
}

R CodeGen::visitInterfaceDeclaration(InterfaceDeclaration* stmt) {
    return true;
}

R CodeGen::visitPublicKeyword(PublicKeyword* expr) {
    
    return true;
}

R CodeGen::visitPrivateKeyword(PrivateKeyword* expr) {
    return true;
}

R CodeGen::visitProtectedKeyword(ProtectedKeyword* expr) {
    return true;
}

R CodeGen::visitStaticKeyword(StaticKeyword* expr) {
    return true;
}

R CodeGen::visitRestParameter(RestParameter* expr) {
    return true;
}

// --------------------- Utils ----------------------

int CodeGen::recordInstanceField(const string& classId, const string& fieldId, Expression* initExpr, const PropertyMeta& propMeta) {
    
    CodeGen nested(module_);
    nested.cur = make_shared<Chunk>();
        
    if (initExpr == nullptr) {
        
        int idx = nested.emitConstant(Value::undefined());
        nested.emit(OpCode::LoadConstant);
        nested.emitUint32(idx);
        
    } else {
        (initExpr->accept(nested));
    }
    
    nested.emit(OpCode::Return);
    
    // Register the init as a constant for this module
    auto fnChunk = nested.cur;
    uint32_t chunkIndex = module_->addChunk(fnChunk);
    nested.cur->name = fieldId;
    
    disassembleChunk(nested.cur.get(), classId + " : " + fieldId);

    auto fnObj = make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->name = fieldId;

    Value fnValue = Value::functionRef(fnObj);
    int ci = module_->addConstant(fnValue);
    
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value(ci)));

    return 0;

}

void CodeGen::resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames) {
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

void CodeGen::emit(OpCode op) {
    cur->writeByte(static_cast<uint8_t>(op));
}

void CodeGen::emitUint32(uint32_t v) {
    cur->writeUint32(v);
}
void CodeGen::emitUint8(uint8_t v) {
    cur->writeUint8(v);
}

int CodeGen::emitConstant(const Value &v) {
    return cur->addConstant(v);
}

uint32_t CodeGen::makeLocal(const std::string& name, BindingKind kind) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return locals[i].slot_index;
    }
    uint32_t idx = (uint32_t)locals.size();
    Local local { name, scopeDepth, false, idx, kind };
    locals.push_back(local);
    if (idx + 1 > cur->maxLocals) cur->maxLocals = idx + 1;
    return idx;
}

bool CodeGen::hasLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return true;
    }
    return false;
}

uint32_t CodeGen::getLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return locals[i].slot_index;
    }
    throw std::runtime_error("Local not found: " + name);
}

int CodeGen::emitJump(OpCode op) {
    emit(op);
    // write placeholder uint32
    int pos = (int)cur->size();
    emitUint32(0);
    return pos;
}

void CodeGen::patchJump(int jumpPos) {
    // write offset from after placeholder to current ip
    uint32_t offset = (uint32_t)(cur->size() - (jumpPos + 4));
    // overwrite bytes at jumpPos with offset (little-endian)
    cur->code[jumpPos + 0] = (offset >> 0) & 0xFF;
    cur->code[jumpPos + 1] = (offset >> 8) & 0xFF;
    cur->code[jumpPos + 2] = (offset >> 16) & 0xFF;
    cur->code[jumpPos + 3] = (offset >> 24) & 0xFF;
}

void CodeGen::patchJump(int jumpPos, int target) {
    uint32_t offset = (uint32_t)(target - (jumpPos + 4));
    cur->code[jumpPos + 0] = (offset >> 0) & 0xFF;
    cur->code[jumpPos + 1] = (offset >> 8) & 0xFF;
    cur->code[jumpPos + 2] = (offset >> 16) & 0xFF;
    cur->code[jumpPos + 3] = (offset >> 24) & 0xFF;
}

void CodeGen::emitLoop(uint32_t offset) {
    emit(OpCode::Loop);
    emitUint32(offset);
}

void CodeGen::beginLoop() {
    LoopContext ctx;
    ctx.loopStart = (int)cur->size();
    loopStack.push_back(ctx);
}

void CodeGen::endLoop() {
    LoopContext ctx = loopStack.back();
    loopStack.pop_back();

    // Patch all breaks to jump here
    int end = (int)cur->size();
    for (int breakAddr : ctx.breaks) {
        patchJump(breakAddr, end);
    }
}

int CodeGen::emitTryPlaceholder() {
    emit(OpCode::Try);

    // Reserve 8 bytes (two u32: catchOffset, finallyOffset)
    int pos = (int)cur->size();
    for (int i = 0; i < 8; i++) {
        cur->code.push_back(0);
    }
    return pos; // return position where we wrote the offsets
}

void CodeGen::patchTryCatch(int tryPos, int target) {
    uint32_t offset = (uint32_t)(target - (tryPos + 8));
    cur->code[tryPos + 0] = (offset >> 0) & 0xFF;
    cur->code[tryPos + 1] = (offset >> 8) & 0xFF;
    cur->code[tryPos + 2] = (offset >> 16) & 0xFF;
    cur->code[tryPos + 3] = (offset >> 24) & 0xFF;
}

void CodeGen::patchTryFinally(int tryPos, int target) {
    uint32_t offset = (uint32_t)(target - (tryPos + 8));
    cur->code[tryPos + 4] = (offset >> 0) & 0xFF;
    cur->code[tryPos + 5] = (offset >> 8) & 0xFF;
    cur->code[tryPos + 6] = (offset >> 16) & 0xFF;
    cur->code[tryPos + 7] = (offset >> 24) & 0xFF;
}

void CodeGen::patchTry(int pos) {
    uint32_t offset = (uint32_t)(cur->size() - (pos + 4));
    cur->code[pos + 0] = (offset >> 0) & 0xFF;
    cur->code[pos + 1] = (offset >> 8) & 0xFF;
    cur->code[pos + 2] = (offset >> 16) & 0xFF;
    cur->code[pos + 3] = (offset >> 24) & 0xFF;
}

void CodeGen::declareLocal(const string& name, BindingKind kind) {
    if (scopeDepth == 0) return; // globals aren’t locals

    // prevent shadowing in same scope
    for (int i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].depth != -1 && locals[i].depth < scopeDepth) break;
        if (locals[i].name == name) {
            throw runtime_error("Variable" + name + " already declared in this scope");
        }
    }

    uint32_t idx = (uint32_t)locals.size();

    Local local { name, scopeDepth, false, (uint32_t)locals.size(), kind };
    locals.push_back(local);
    
    if (idx + 1 > cur->maxLocals) cur->maxLocals = idx + 1;
}

void CodeGen::declareGlobal(const string& name, BindingKind kind) {
    
    if (scopeDepth > 0) return; // locals aren’t globals

    for (int i = (int)globals.size() - 1; i >= 0; i--) {
        if (globals[i].name == name) {
            throw runtime_error("Variable " + name + " already declared in this scope.");
        }
    }

    Global global { name, kind };
    globals.push_back(global);
    
}

void CodeGen::emitSetLocal(int slot) {
    emit(OpCode::StoreLocal);
    emitUint32(slot);
}

int CodeGen::paramSlot(const string& name) {
    return resolveLocal(name); //locals[name].slot_index;
}

int CodeGen::resolveLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].name == name) {
            return locals[i].slot_index; // return slot index
        }
    }
    return -1;
}

int CodeGen::addUpvalue(bool isLocal, int index, string name, BindingKind kind) {
    for (int i = 0; i < (int)upvalues.size(); i++) {
        if (upvalues[i].isLocal == isLocal && upvalues[i].index == index) {
            return i;
        }
    }
    upvalues.push_back({isLocal, (uint32_t)index, name, kind});
    return (int)upvalues.size() - 1;
}

    // resolve variable
int CodeGen::resolveUpvalue(const std::string& name) {
    
    if (enclosing) {
        int localIndex = enclosing->resolveLocal(name);
        if (localIndex != -1) {
            enclosing->locals[localIndex].isCaptured = true;
            return addUpvalue(true,
                              localIndex,
                              name,
                              enclosing->locals[localIndex].kind);
        }
        
        int upIndex = enclosing->resolveUpvalue(name);
        if (upIndex != -1) {
            
            return addUpvalue(false,
                              upIndex,
                              enclosing->upvalues[upIndex].name,
                              enclosing->upvalues[upIndex].kind);
        }
    }
    
    return -1;
    
}

void CodeGen::beginScope() {
    scopeDepth++;
}

void CodeGen::endScope() {
    // Pop locals declared in this scope
    while (!locals.empty() && locals.back().depth == scopeDepth) {
        if (locals.back().isCaptured) {
            // Local captured by closure → close it
            emit(OpCode::CloseUpvalue);
        } else {
            // Normal local → just pop
            emit(OpCode::Pop);
        }
        locals.pop_back();
    }
    scopeDepth--;
}

bool CodeGen::isModuleLoaded(string importPath) {
    
    for (auto& module_ : registered_modules) {
        if (module_ == importPath) {
            return true;
        }
    }

    return false;

}

void CodeGen::registerModule(string importPath) {
    registered_modules.push_back(importPath);
}

BindingKind CodeGen::get_kind(string kind) {
    if (kind == "CONST") {
        return BindingKind::Const;
    }
    
    if (kind == "LET") {
        return BindingKind::Let;
    }
    
    return BindingKind::Var;
}

inline uint32_t CodeGen::readUint32(const Chunk* chunk, size_t offset) {
    return (uint32_t)chunk->code[offset] |
           ((uint32_t)chunk->code[offset + 1] << 8) |
           ((uint32_t)chunk->code[offset + 2] << 16) |
           ((uint32_t)chunk->code[offset + 3] << 24);
}

int CodeGen::lookupGlobal(const string& name) {
    for (int i = (int)globals.size() - 1; i >= 0; i--) {
        if (globals[i].name == name) {
            return i;
        }
    }
    return -1;
}

PropertyLookup CodeGen::lookupClassProperty(string prop_name) {
    
    if(classInfo.fields.count(prop_name)) {
        return { 1, classInfo.fields[prop_name] };
    }
    
    string super_class_name = classInfo.super_class_name;
    auto super_class_info = classes[super_class_name];

    if (super_class_info.fields.count(prop_name)) {
        return { 2, super_class_info.fields[prop_name] };
    }
    
    return { -1 , {} };
}

void CodeGen::declareVariableScoping(const string& name, BindingKind kind) {
    
    // do not declare as local if Var and its not inside a function body
    // do not declare as local if its var and the scopedepth is > 0

    // --------- let scoping check-----------
    bool IsVar = (kind == BindingKind::Var) ? true : false;
    
    if (IsVar && scopeDepth > 0 && enclosing == nullptr) {
        
        int previousScopeDepth = scopeDepth;
        scopeDepth = 0;
        declareGlobal(name, kind);
        scopeDepth = previousScopeDepth;

        return;
    }
    
    if (enclosing) {
        if (IsVar) {
            declareLocal(name, (kind));
            int idx = resolveLocal(name);
            locals[idx].depth = locals[idx].depth - 1;
            return;
        }
    }
    // --------- end of let scoping check-----------
    
    declareLocal(name, (kind));
    declareGlobal(name, (kind));
    
    // if the current scope depth is greater than 0, we must declare local

}

string CodeGen::evaluate_property(Expression* expr) {
    if (auto member = dynamic_cast<MemberExpression*>(expr)) {
        if (member->computed) {
            return evaluate_property(member->property.get());
        } else {
            return member->name.lexeme;
        }
    }
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr)) {
        return ident->name;
    }
    if (auto literal = dynamic_cast<LiteralExpression*>(expr)) {
        return literal->token.lexeme; // Or whatever holds the property key
    }
    throw std::runtime_error("Unsupported expression type in evaluate_property");
}

size_t CodeGen::disassembleInstruction(const Chunk* chunk, size_t offset) {
    std::cout << std::setw(4) << offset << " ";

    uint8_t instruction = chunk->code[offset];
    OpCode op = static_cast<OpCode>(instruction);

    switch (op) {
            // no operands
        case OpCode::Nop:
        case OpCode::Pop:
        case OpCode::Dup:
        case OpCode::Add:
        case OpCode::Subtract:
        case OpCode::Multiply:
        case OpCode::Divide:
        case OpCode::Modulo:
        case OpCode::Power:
        case OpCode::Negate:
        case OpCode::LogicalNot:
        case OpCode::Equal:
        case OpCode::NotEqual:
        case OpCode::LessThan:
        case OpCode::LessThanOrEqual:
        case OpCode::GreaterThan:
        case OpCode::GreaterThanOrEqual:
        case OpCode::NewObject:
        case OpCode::NewArray:
        case OpCode::Return:
        case OpCode::Halt:
            
        case OpCode::Increment:
            
        case OpCode::LogicalAnd:
        case OpCode::LogicalOr:
        case OpCode::NullishCoalescing:
        case OpCode::StrictEqual:
        case OpCode::StrictNotEqual:
        case OpCode::Decrement:
            
            // bitwise
        case OpCode::BitAnd:
        case OpCode::BitOr:
        case OpCode::BitXor:
        case OpCode::ShiftLeft:
        case OpCode::ShiftRight:
        case OpCode::UnsignedShiftRight:
        case OpCode::Positive:
        case OpCode::Dup2:
        case OpCode::SetPropertyDynamic:
        case OpCode::GetPropertyDynamic:
            
            std::cout << opcodeToString(op) << "\n";
            return offset + 1;
            
            // u32 operands
        case OpCode::LoadConstant: {
            uint32_t index = readUint32(chunk, offset + 1);
            std::cout << opcodeToString(op) << " " << index << " (";
            if (index < chunk->constants.size()) {
                std::cout << chunk->constants[index].toString();
            }
            std::cout << ")\n";
            return offset + 1 + 4;
        }
            
        case OpCode::LoadLocal:
        case OpCode::StoreLocal: {
            uint32_t slot = readUint32(chunk, offset + 1);
            std::cout << opcodeToString(op) << " " << slot << "\n";
            return offset + 1 + 4;
        }
            
        case OpCode::GetThisProperty:
        case OpCode::SetThisProperty:
        case OpCode::GetUpvalue:
        case OpCode::SetUpvalue:
        case OpCode::LoadGlobal:
        case OpCode::StoreGlobal:
        case OpCode::CreateGlobal:
        case OpCode::SetProperty:
        case OpCode::SetStaticProperty:
        case OpCode::LoadChunkIndex:
        case OpCode::LoadArgument:
            
        case OpCode::GetProperty:
        case OpCode::LoadLocalVar:
        case OpCode::LoadGlobalVar:
        case OpCode::StoreLocalVar:
        case OpCode::StoreGlobalVar:
        case OpCode::StoreLocalLet:
        case OpCode::StoreGlobalLet:
        case OpCode::CreateLocalVar:
        case OpCode::CreateLocalLet:
        case OpCode::CreateLocalConst:
        case OpCode::CreateGlobalVar:
        case OpCode::CreateGlobalLet:
        case OpCode::CreateGlobalConst:
        case OpCode::StoreThisProperty:
        case OpCode::LoadUpvalue:
        case OpCode::StoreUpvalueVar:
        case OpCode::StoreUpvalueLet:
        case OpCode::StoreUpvalueConst:
        case OpCode::LoadThisProperty:
{
            uint32_t nameIndex = readUint32(chunk, offset + 1);
            std::cout << opcodeToString(op) << " constant[" << nameIndex << "]";
            if (nameIndex < chunk->constants.size()) {
                std::cout << " \"" << chunk->constants[nameIndex].toString() << "\"";
            }
            std::cout << "\n";
            return offset + 1 + 4;
        }
            
        case OpCode::CreateClosure: {
            
            cout << opcodeToString(op);

            size_t index = offset;

            index++;
            uint8_t ci = chunk->code[index];
            
            cout << " module-constant-index: [" << (int)ci << "]";
            Value fnVal = module_->constants[ci];
            auto fnRef = fnVal.fnRef;
            
            if (fnRef->upvalues_size > 0) {
                for (size_t i = 0; i < fnRef->upvalues_size; ++i) {
                    // Read upvalue info (isLocal, index)
                    
                    index++;
                    uint32_t isLocal = chunk->code[index];
                    
                    index++;
                    uint32_t idx = chunk->code[index];
                    
                    cout << " upvalue[" << i << "]: isLocal=" << (isLocal ? "true" : "false") << " index=" << (int)idx;
                    cout << "\n";
                    
                }
                
            } else {
                cout << " (closure metadata not found)\n";
                return index + 1;
            }
            
            return index + 1;
            
        }
            
        case OpCode::ArrayPush: {
            std::cout << opcodeToString(op) << "\n";
            return offset + 1;
        }
            
            // jumps
        case OpCode::Jump:
        case OpCode::JumpIfFalse:
        case OpCode::Loop: {
            uint32_t jump = readUint32(chunk, offset + 1);
            size_t target = (op == OpCode::Loop) ? offset - jump : offset + 5 + jump;
            std::cout << opcodeToString(op) << " " << target << "\n";
            return offset + 1 + 4;
        }
            
            // calls
        case OpCode::InvokeConstructor:
        case OpCode::SuperCall:
        case OpCode::Call: {
            uint8_t argCount = chunk->code[offset + 1];
            std::cout << opcodeToString(op) << " " << (int)argCount << " args\n";
            return offset + 2;
        }
            
        case OpCode::CreateInstance:
        case OpCode::GetThis:
        case OpCode::NewClass:
        case OpCode::Try:
        case OpCode::EndTry:
        case OpCode::EndFinally:
        case OpCode::Throw:
        case OpCode::EnumKeys:
        case OpCode::GetObjectLength:
        case OpCode::GetIndexPropertyDynamic:
        case OpCode::Debug:
        case OpCode::LoadArguments:
        case OpCode::Slice:
        case OpCode::LoadArgumentsLength:
        case OpCode::CloseUpvalue:
        case OpCode::ClearStack:
        case OpCode::ClearLocals:
        case OpCode::GetParentObject:
        case OpCode::CreateObjectLiteral:
        case OpCode::TypeOf:
        case OpCode::Delete:
        case OpCode::InstanceOf:
            cout << opcodeToString(op) << "\n";
            return offset + 1;
        case OpCode::CreateObjectLiteralProperty:
        case OpCode::CreateClassPrivatePropertyVar:
        case OpCode::CreateClassPublicPropertyVar:
        case OpCode::CreateClassProtectedPropertyVar:
        case OpCode::CreateClassPrivatePropertyConst:
        case OpCode::CreateClassPublicPropertyConst:
        case OpCode::CreateClassProtectedPropertyConst:
        case OpCode::CreateClassPrivateStaticPropertyVar:
        case OpCode::CreateClassPublicStaticPropertyVar:
        case OpCode::CreateClassProtectedStaticPropertyVar:
        case OpCode::CreateClassPrivateStaticPropertyConst:
        case OpCode::CreateClassPublicStaticPropertyConst:
        case OpCode::CreateClassProtectedStaticPropertyConst:
        case OpCode::CreateClassProtectedStaticMethod:
        case OpCode::CreateClassPrivateStaticMethod:
        case OpCode::CreateClassPublicStaticMethod:
        case OpCode::CreateClassProtectedMethod:
        case OpCode::CreateClassPrivateMethod:
        case OpCode::CreateClassPublicMethod: {
           uint32_t nameIndex = readUint32(chunk, offset + 1);
           std::cout << opcodeToString(op) << " constant[" << nameIndex << "]";
           if (nameIndex < chunk->constants.size()) {
               std::cout << " \"" << chunk->constants[nameIndex].toString() << "\"";
           }
           std::cout << "\n";
           return offset + 1 + 4;
       }
    }

    std::cout << "UNKNOWN " << (int)instruction << "\n";
    return offset + 1;
}

void CodeGen::disassembleChunk(const Chunk* chunk, const std::string& name) {
    std::cout << "== " << name << " ==\n";
    for (size_t offset = 0; offset < chunk->code.size();) {
        offset = disassembleInstruction(chunk, offset);
    }
}

Token CodeGen::createToken(TokenType type) {
    Token token;
    token.type = type;
    return token;
}
