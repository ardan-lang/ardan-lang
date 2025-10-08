//
//  CodeGenerator.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "CodeGenerator.hpp"

CodeGen::CodeGen() : cur(nullptr), nextLocalSlot(0) { }

size_t CodeGen::generate(const vector<unique_ptr<Statement>> &program) {
    cur = std::make_shared<Chunk>();
    cur->name = "BYTECODE";
    locals.clear();
    nextLocalSlot = 0;

    for (const auto &s : program) {
        s->accept(*this);
    }

    emit(OpCode::OP_HALT);
    disassembleChunk(cur.get(), cur->name);
    
    uint32_t idx = module_->addChunk(cur);
    
    return idx;
}

// ------------------- Statements --------------------

R CodeGen::visitExpression(ExpressionStatement* stmt) {
    stmt->expression->accept(*this);
    emit(OpCode::OP_POP);
    return true;
}

R CodeGen::visitBlock(BlockStatement* stmt) {
    // naive: just compile statements in current context (no block-scoped locals handling)
    
    beginScope();
    
    for (auto& s : stmt->body) {
        s->accept(*this);
    }
    
    endScope();
    
    return true;
}

R CodeGen::define(string decl) {
    
    // decide local or global
    if (hasLocal(decl)) {
        uint32_t idx = getLocal(decl);
        emit(OpCode::OP_SET_LOCAL);
        emitUint32(idx);
    } else {

        if (classInfo.fields.count(decl)) {
            
            // Rewrite: legs = one;  ⇒  this.legs = one;
            // Rewrite: legs;  ⇒  this.legs;
            emit(OpCode::SetThisProperty);
            int nameIdx = emitConstant(Value::str(decl));
            emitUint32(nameIdx);
            return R();
        }

        int upvalue = resolveUpvalue(decl);
        if (upvalue != -1) {
            emit(OpCode::OP_SET_UPVALUE);
            emitUint32(upvalue);
            return R();
        }

        // top-level/global
        int nameIdx = emitConstant(Value::str(decl));
        emit(OpCode::OP_DEFINE_GLOBAL);
        emitUint32((uint32_t)nameIdx);
    }
    
    return true;

}

R CodeGen::visitVariable(VariableStatement* stmt) {
    
    // var, let and const
    string kind = stmt->kind;
    
    for (auto &decl : stmt->declarations) {

        declareLocal(decl.id);

        if (decl.init) {
            decl.init->accept(*this); // push init value
        } else {
            emit(OpCode::LoadConstant);
            int ci = emitConstant(Value::undefined());
            emitUint32(ci);
        }
                
        // decide local or global
        if (hasLocal(decl.id)) {
            uint32_t idx = getLocal(decl.id);
            emit(OpCode::OP_SET_LOCAL);
            emitUint32(idx);
        } else {

            int upvalue = resolveUpvalue(decl.id);
            if (upvalue != -1) {
                // emit(OpCode::OP_GET_UPVALUE);
                emit(OpCode::OP_SET_UPVALUE);
                emitUint32(upvalue);
                return R();
            }

            // top-level/global
            int nameIdx = emitConstant(Value::str(decl.id));
            emit(OpCode::OP_DEFINE_GLOBAL);
            emitUint32((uint32_t)nameIdx);
        }
        
    }
    
    return true;
    
}

R CodeGen::visitIf(IfStatement* stmt) {
    stmt->test->accept(*this);
    int elseJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emit(OpCode::OP_POP); // pop condition
    if (stmt->consequent) stmt->consequent->accept(*this);
    int endJump = emitJump(OpCode::OP_JUMP);
    patchJump(elseJump);
    emit(OpCode::OP_POP); // pop condition (if we branched)
    if (stmt->alternate) stmt->alternate->accept(*this);
    patchJump(endJump);
    return true;
}

R CodeGen::visitWhile(WhileStatement* stmt) {
    beginLoop();
    loopStack.back().loopStart = (int)cur->size();

    // loop start
    size_t loopStart = cur->size();
    stmt->test->accept(*this);
    int exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emit(OpCode::OP_POP);
    stmt->body->accept(*this);
    // jump back to loop start
    // uint32_t backOffset = (uint32_t)(cur->size() - loopStart + 4 + 1); // estimate; we'll write OP_LOOP with offset
    // simpler: compute distance as current ip - loopStart + 4?
    // but we have OP_LOOP that expects offset to subtract; we'll write offset below:
    emitLoop((uint32_t)(cur->size() - loopStart + 4 + 1));
    patchJump(exitJump);
    emit(OpCode::OP_POP);
    
    endLoop();
    
    return true;
}

R CodeGen::visitFor(ForStatement* stmt) {
    
    beginLoop();
    loopStack.back().loopStart = (int)cur->size();

    // For simplicity, translate to while:
    if (stmt->init) stmt->init->accept(*this);
    size_t loopStart = cur->size();
    if (stmt->test) {
        stmt->test->accept(*this);
        int exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
        emit(OpCode::OP_POP);
        stmt->body->accept(*this);
        if (stmt->update) stmt->update->accept(*this);
        // loop back
        emitLoop((uint32_t)(cur->size() - loopStart ) + 4 + 1);
        patchJump(exitJump);
        emit(OpCode::OP_POP);
    } else {
        // infinite loop
        stmt->body->accept(*this);
        if (stmt->update) stmt->update->accept(*this);
        emitLoop((uint32_t)(cur->size() - loopStart ) + 4 + 1);
    }
    
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
    emit(OpCode::OP_RETURN);
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
            emit(OpCode::OP_ADD); // stack: left, right -> left+right
            break;
        case TokenType::MINUS:
            emit(OpCode::OP_SUB);
            break;
        case TokenType::MUL:
            emit(OpCode::OP_MUL);
            break;
        case TokenType::DIV:
            emit(OpCode::OP_DIV);
            break;
        case TokenType::MODULI:
            emit(OpCode::OP_MOD);
            break;
        case TokenType::POWER:
            emit(OpCode::OP_POW);
            break;

        // --- Comparisons ---
        case TokenType::VALUE_EQUAL:
            emit(OpCode::OP_EQUAL);
            break;
        case TokenType::REFERENCE_EQUAL:
            emit(OpCode::REFERENCE_EQUAL);
            break;
        case TokenType::INEQUALITY:
            emit(OpCode::OP_NOTEQUAL);
            break;
        case TokenType::STRICT_INEQUALITY:
            emit(OpCode::STRICT_INEQUALITY);
            break;
        case TokenType::LESS_THAN:
            emit(OpCode::OP_LESS);
            break;
        case TokenType::LESS_THAN_EQUAL:
            emit(OpCode::OP_LESSEQUAL);
            break;
        case TokenType::GREATER_THAN:
            emit(OpCode::OP_GREATER);
            break;
        case TokenType::GREATER_THAN_EQUAL:
            emit(OpCode::OP_GREATEREQUAL);
            break;

        // --- Logical ---
        case TokenType::LOGICAL_AND:
            emit(OpCode::LOGICAL_AND);
            break;
        case TokenType::LOGICAL_OR:
            emit(OpCode::LOGICAL_OR);
            break;
        case TokenType::NULLISH_COALESCING:
            emit(OpCode::NULLISH_COALESCING);
            break;

        // --- Bitwise ---
        case TokenType::BITWISE_AND:
            emit(OpCode::OP_BIT_AND);
            break;
        case TokenType::BITWISE_OR:
            emit(OpCode::OP_BIT_OR);
            break;
        case TokenType::BITWISE_XOR:
            emit(OpCode::OP_BIT_XOR);
            break;
        case TokenType::BITWISE_LEFT_SHIFT:
            emit(OpCode::OP_SHL);
            break;
        case TokenType::BITWISE_RIGHT_SHIFT:
            emit(OpCode::OP_SHR);
            break;
        case TokenType::UNSIGNED_RIGHT_SHIFT:
            emit(OpCode::OP_USHR);
            break;

        default:
            throw std::runtime_error("Unknown binary operator in compiler: " + expr->op.lexeme);
    }
    
    return true;

}

void CodeGen::emitAssignment(BinaryExpression* expr) {
    auto left = expr->left.get();

    // --------------------
    // Plain assignment (=)
    // --------------------
    if (expr->op.type == TokenType::ASSIGN) {
        if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
            // Evaluate RHS first
            expr->right->accept(*this);
            
            //ident->accept(*this);
            define(ident->token.lexeme);
//            if (hasLocal(ident->name)) {
//                emit(OpCode::OP_SET_LOCAL);
//                emitUint32(getLocal(ident->name));
//            } else if (resolveUpvalue(ident->name) != - 1 ) {
//                emit(OpCode::OP_SET_UPVALUE);
//                emitUint32(resolveUpvalue(ident->name));
//            } else {
//                int nameIdx = emitConstant(Value::str(ident->name));
//                emit(OpCode::OP_SET_GLOBAL);
//                emitUint32(nameIdx);
//            }
        
        }
        else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
            // Push object first
            member->object->accept(*this);
            // Then RHS
            expr->right->accept(*this);

            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::OP_SET_PROPERTY);
            emitUint32(nameIdx);
        }
        else {
            throw std::runtime_error("Unsupported assignment target in CodeGen");
        }

        // If assignments are statements only, discard the result:
        // emit(OpCode::OP_POP);

        return;
    }

    // -----------------------------
    // Compound assignment (+=, etc.)
    // -----------------------------
    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
        ident->accept(*this);
//        if (hasLocal(ident->name)) {
//            emit(OpCode::OP_GET_LOCAL);
//            emitUint32(getLocal(ident->name));
//        } else {
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::OP_GET_GLOBAL);
//            emitUint32(nameIdx);
//        }
    }
    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
        // Push object, duplicate it for later use
        member->object->accept(*this);
        emit(OpCode::OP_DUP);

        int nameIdx = emitConstant(Value::str(member->name.lexeme));
        emit(OpCode::OP_GET_PROPERTY);
        emitUint32(nameIdx);
    }
    else {
        throw std::runtime_error("Unsupported assignment target in CodeGen");
    }

    // Evaluate RHS
    expr->right->accept(*this);

    // Apply compound operation
    switch (expr->op.type) {
        case TokenType::ASSIGN_ADD: emit(OpCode::OP_ADD); break;
        case TokenType::ASSIGN_MINUS: emit(OpCode::OP_SUB); break;
        case TokenType::ASSIGN_MUL: emit(OpCode::OP_MUL); break;
        case TokenType::ASSIGN_DIV: emit(OpCode::OP_DIV); break;
        case TokenType::MODULI_ASSIGN: emit(OpCode::OP_MOD); break;
        case TokenType::POWER_ASSIGN: emit(OpCode::OP_POW); break;
        case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: emit(OpCode::OP_SHL); break;
        case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: emit(OpCode::OP_SHR); break;
        case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: emit(OpCode::OP_USHR); break;
        case TokenType::BITWISE_AND_ASSIGN: emit(OpCode::OP_BIT_AND); break;
        case TokenType::BITWISE_OR_ASSIGN: emit(OpCode::OP_BIT_OR); break;
        case TokenType::BITWISE_XOR_ASSIGN: emit(OpCode::OP_BIT_XOR); break;
        case TokenType::LOGICAL_AND_ASSIGN: emit(OpCode::LOGICAL_AND); break;
        case TokenType::LOGICAL_OR_ASSIGN: emit(OpCode::LOGICAL_OR); break;
        case TokenType::NULLISH_COALESCING_ASSIGN: emit(OpCode::NULLISH_COALESCING); break;
        default: throw std::runtime_error("Unknown compound assignment operator in emitAssignment");
    }

    // Store result back
    if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
        define(ident->token.lexeme);
        // ident->accept(*this);
//        if (hasLocal(ident->name)) {
//            emit(OpCode::OP_SET_LOCAL);
//            emitUint32(getLocal(ident->name));
//        } else {
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::OP_SET_GLOBAL);
//            emitUint32(nameIdx);
//        }
    }
    else if (auto* member = dynamic_cast<MemberExpression*>(left)) {
        int nameIdx = emitConstant(Value::str(member->name.lexeme));
        emit(OpCode::OP_SET_PROPERTY);
        emitUint32(nameIdx);
    }

    // Optional: if compound assignments are statements, discard result:
    // emit(OpCode::OP_POP);
}

R CodeGen::visitUnary(UnaryExpression* expr) {
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
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB);
            
            // store back
            define(ident->token.lexeme);
//            if (hasLocal(ident->name)) {
//                emit(OpCode::OP_SET_LOCAL);
//                emitUint32(getLocal(ident->name));
//            } else {
//                int nameIdx = emitConstant(Value::str(ident->name));
//                emit(OpCode::OP_SET_GLOBAL);
//                emitUint32(nameIdx);
//            }
            
            return true;
            
        }

        if (auto member_expr = dynamic_cast<MemberExpression*>(expr->right.get())) {
            // Evaluate object once
            member_expr->object->accept(*this);
            emit(OpCode::OP_DUP); // [obj, obj]
            int nameIdx = emitConstant(Value::str(member_expr->name.lexeme));
            // GET_PROPERTY -> consumes one obj
            emit(OpCode::OP_GET_PROPERTY);
            emitUint32(nameIdx); // stack: [obj, value]
            // push 1
            emit(OpCode::LoadConstant);
            emitUint32(emitConstant(Value::number(1)));
            // apply
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB);
            // SET_PROPERTY (consumes [obj, value])
            emit(OpCode::OP_SET_PROPERTY);
            emitUint32(nameIdx);
            return true;
        }

        throw std::runtime_error("Unsupported unary increment/decrement target");
    }

    // non-targeted unary ops (prefix) evaluate their operand first
    expr->right->accept(*this);
    switch (expr->op.type) {
        case TokenType::LOGICAL_NOT: emit(OpCode::OP_NOT); break;
        case TokenType::MINUS: emit(OpCode::OP_NEGATE); break;
        case TokenType::BITWISE_NOT: emit(OpCode::OP_NOT); break;
        case TokenType::ADD: emit(OpCode::OP_POSITIVE); break;
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

R CodeGen::visitIdentifier(IdentifierExpression* expr) {
    string name = expr->name;
    if (hasLocal(name)) {
        emit(OpCode::OP_GET_LOCAL);
        emitUint32(getLocal(name));
    } else {
        
        if (classInfo.fields.count(name)) {
            
            // Rewrite: legs;  ⇒  this.legs;
            emit(OpCode::GetThisProperty);
            int nameIdx = emitConstant(Value::str(name));
            emitUint32(nameIdx);
            return R();
        }
        
        int upvalue = resolveUpvalue(expr->name);
        if (upvalue != -1) {
            emit(OpCode::OP_GET_UPVALUE);
            emitUint32(upvalue);
            return R();
        }
        
        int nameIdx = emitConstant(Value::str(name));
        emit(OpCode::OP_GET_GLOBAL);
        emitUint32(nameIdx);
        
    }
    
    return true;
    
}

R CodeGen::visitCall(CallExpression* expr) {
    // emit callee, then args left-to-right, then OP_CALL argc
    
    if (classInfo.fields.size() > 0 && classInfo.fields.count("constructor")) {
        // check if this is a super() call.
        if (auto ident = dynamic_cast<SuperExpression*>(expr->callee.get())) {
            
            for (auto &arg : expr->arguments) {
                arg->accept(*this);
            }
            uint8_t argc = (uint8_t)expr->arguments.size();
            
            emit(OpCode::SuperCall);
            emitUint8(argc);
            
            return true;
            
        }
    }
    
    expr->callee->accept(*this);
    for (auto &arg : expr->arguments) {
        arg->accept(*this);
    }
    uint8_t argc = (uint8_t)expr->arguments.size();
    emit(OpCode::OP_CALL);
    emitUint8(argc);
    return true;
}

R CodeGen::visitMember(MemberExpression* expr) {
    // produce (object) then OP_GET_PROPERTY name
    expr->object->accept(*this);

    string propName;
    if (expr->computed) {
        // compute property expression now
        expr->property->accept(*this);
        emit(OpCode::OP_GET_PROPERTY_DYNAMIC);
        return true;
        // pop the computed property -> evaluate to constant string at runtime not supported here
        // Simplify: only support non-computed for codegen for now
        // throw std::runtime_error("Computed member expressions not supported by this CodeGen yet.");
    } else {
        propName = expr->name.lexeme;
    }

    int nameIdx = emitConstant(Value::str(propName));
    emit(OpCode::OP_GET_PROPERTY);
    emitUint32(nameIdx);
    return true;
}

R CodeGen::visitArray(ArrayLiteralExpression* expr) {
    emit(OpCode::OP_NEW_ARRAY);
    for (auto &el : expr->elements) {
        el->accept(*this); // push element
        emit(OpCode::OP_ARRAY_PUSH);
    }
    return true;
}

R CodeGen::visitObject(ObjectLiteralExpression* expr) {
    emit(OpCode::OP_NEW_OBJECT);
    // For each prop: evaluate value, then OP_SET_PROPERTY with name const
    for (auto &prop : expr->props) {
        // evaluate value
        prop.second->accept(*this);
        int nameIdx = emitConstant(Value::str(prop.first.lexeme));
        emit(OpCode::OP_SET_PROPERTY);
        emitUint32(nameIdx);
    }
    return true;
}

R CodeGen::visitConditional(ConditionalExpression* expr) {
    expr->test->accept(*this);
    int elseJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    // emit(OpCode::OP_POP);
    expr->consequent->accept(*this);
    int endJump = emitJump(OpCode::OP_JUMP);
    patchJump(elseJump);
    // emit(OpCode::OP_POP);
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
            nested.emit(OpCode::OP_LOAD_ARGUMENTS);      // Push arguments array
            nested.emit(OpCode::LoadConstant);            // Push i
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_SLICE);               // arguments.slice(i)
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));

            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            nested.emit(OpCode::OP_LOAD_ARGUMENTS_LENGTH);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_GREATER);
            int useArg = nested.emitJump(OpCode::OP_JUMP_IF_FALSE);

            // Use argument
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::OP_JUMP);

            // Use default
            nested.patchJump(useArg);
            // Evaluate default expression (can reference previous params!)
            info.defaultExpr->accept(nested);

            // Set local either way
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));

        } else {
            // Direct: assign argument i to local slot
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
            
        }
    }

    // Compile function body
    if (expr->exprBody) {
        expr->exprBody->accept(nested);
        nested.emit(OpCode::OP_RETURN);
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
            nested.emit(OpCode::OP_RETURN);
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

    // emit(OpCode::OP_LOAD_CHUNK_INDEX);
    emit(OpCode::OP_CLOSURE);
    emitUint8((uint8_t)ci);

    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
    }

    // disassembleChunk(nested.cur.get(), nested.cur->name);

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
        } else if (auto* ident = dynamic_cast<IdentifierExpression*>(param.get())) {
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
            nested.emit(OpCode::OP_LOAD_ARGUMENTS);      // Push arguments array
            nested.emit(OpCode::LoadConstant);            // Push i
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_SLICE);               // arguments.slice(i)
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
            // nested.emit(OpCode::OP_DEFINE_GLOBAL);
            // nested.emitUint32(nested.emitConstant(Value::str(info.name)));
            continue;
        }
        // For parameters with default value
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            nested.emit(OpCode::OP_LOAD_ARGUMENTS_LENGTH);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_GREATER);
            int useArg = nested.emitJump(OpCode::OP_JUMP_IF_FALSE);

            // Use argument
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::OP_JUMP);

            // Use default
            nested.patchJump(useArg);
            // Evaluate default expression (can reference previous params!)
            info.defaultExpr->accept(nested);

            // Set local either way
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
            // nested.emit(OpCode::OP_DEFINE_GLOBAL);
            // nested.emitUint32(nested.emitConstant(Value::str(info.name)));
        } else {
            // Direct: assign argument i to local slot
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::OP_SET_LOCAL);
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
            nested.emit(OpCode::OP_RETURN);
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

    // emit(OpCode::OP_LOAD_CHUNK_INDEX);
    emit(OpCode::OP_CLOSURE);
    emitUint8((uint8_t)ci);

    // Emit upvalue descriptors
    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
    }
    
    // Bind function to its name in the global environment
    if (scopeDepth == 0) {
        // emit(OpCode::OP_DEFINE_GLOBAL);
        // int nameIdx = emitConstant(Value::str(stmt->id));
        // emitUint32(nameIdx);
    } else {
        // declareLocal(stmt->id);
        // emit(OpCode::OP_SET_LOCAL);
        // int nameIdx = emitConstant(Value::str(stmt->id));
        // emitUint32(nameIdx);
    }

    // disassembleChunk(nested.cur.get(), nested.cur->name);

    return true;
}

R CodeGen::visitFunction(FunctionDeclaration* stmt) {
    // Create a nested code generator for the function body
    CodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = std::make_shared<Chunk>();
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
        if (info.isRest) {
            // Collect rest arguments as array: arguments.slice(i)
            nested.emit(OpCode::OP_LOAD_ARGUMENTS);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_SLICE);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
            continue;
        }
        if (info.hasDefault) {
            // if (arguments.length > i) use argument; else use default expr
            nested.emit(OpCode::OP_LOAD_ARGUMENTS_LENGTH);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_GREATER);
            int useArg = nested.emitJump(OpCode::OP_JUMP_IF_FALSE);

            // Use argument
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::OP_JUMP);

            // Use default
            nested.patchJump(useArg);
            info.defaultExpr->accept(nested);

            // Set local either way
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
        } else {
            // Direct: assign argument i to local slot
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::OP_SET_LOCAL);
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
            nested.emit(OpCode::OP_RETURN);
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

    // emit(OpCode::OP_LOAD_CHUNK_INDEX);
    emit(OpCode::OP_CLOSURE);
    emitUint8((uint8_t)ci);

    // Emit upvalue descriptors
    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
    }
    
    // Bind function to its name in the global environment
    if (scopeDepth == 0) {
         emit(OpCode::OP_DEFINE_GLOBAL);
         int nameIdx = emitConstant(Value::str(stmt->id));
         emitUint32(nameIdx);
    } else {
        declareLocal(stmt->id);
        int slot = paramSlot(stmt->id);
        emit(OpCode::OP_SET_LOCAL);
        // int nameIdx = emitConstant(Value::str(stmt->id));
        emitUint32(slot);
    }
    
    // disassemble the chunk for debugging
    //disassembleChunk(nested.cur.get(),
    //                 stmt->id/*nested.cur->name*/);

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
        emit(OpCode::OP_ADD);
        // if expression exists
        if (i < esize) {
            expr->expressions[i]->accept(*this);
            emit(OpCode::OP_ADD);
        }
    }
    return true;
}

//R CodeGen::visitImportDeclaration(ImportDeclaration* stmt) {
//    // Keep simple: perform import at compile time by executing parser+interpreter OR
//    // emit runtime call to some import builtin. For now, call a builtin global "import" if present.
//    // Generate: push path string -> call import(path)
//    int ci = emitConstant(Value::str(stmt->path.lexeme));
//    emit(OpCode::LoadConstant);
//    emitUint32(ci);
//    // callee (import)
//    int importName = emitConstant(Value::str("import"));
//    emit(OpCode::OP_GET_GLOBAL);
//    emitUint32(importName);
//    emit(OpCode::OP_CALL);
//    emitUint8(1);
//    emit(OpCode::OP_POP);
//    return true;
//}

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
    
    // ✅ pass the resolved file path into the parser
    Parser parser(tokens);
    parser.sourceFile = importPath;
    auto ast = parser.parse();

    // Register the imported module BEFORE compiling to handle cycles
    registerModule(importPath);

    // Compile/generate code for the imported program
    // CodeGen importGen(module_);
    // importGen.modulePath = importPath;
    // importGen.generate(importedProgram->body);

    for (const auto &s : ast) {
        s->accept(*this);
    }

    // Optionally, bind imported symbols into the current module's scope
    // (e.g., handle named imports, 'import * as', etc.)
    // For simplicity, this is omitted here.

    // No code emission needed (import is compile-time)
    return true;
}

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
        emit(OpCode::OP_DUP);
        emit(OpCode::OP_POP);
        // Push property name
        int nameIdx = emitConstant(Value::str(member->name.lexeme));
        emit(OpCode::OP_SET_PROPERTY);
        emitUint32(nameIdx);
    } else {
        throw std::runtime_error("Unsupported assignment target in CodeGen");
    }
    return true;
}

R CodeGen::visitLogical(LogicalExpression* expr) {
    expr->left->accept(*this);
    if (expr->op.type == TokenType::LOGICAL_OR) {
        int endJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
        emit(OpCode::OP_POP);
        expr->right->accept(*this);
        patchJump(endJump);
    } else if (expr->op.type == TokenType::LOGICAL_AND) {
        int endJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
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
        if (i + 1 < n) emit(OpCode::OP_POP);
    }
    return true;
}

R CodeGen::visitUpdate(UpdateExpression* expr) {
    // Identifiers: x++
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
        ident->accept(*this);
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
        emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB);
        if (hasLocal(ident->name)) {
            emit(OpCode::OP_SET_LOCAL);
            emitUint32(getLocal(ident->name));
        } else {
            
            int upvalue = resolveUpvalue(ident->name);
            if (upvalue != -1) {
                emit(OpCode::OP_SET_UPVALUE);
                emitUint32(upvalue);
                return R();
            }

            int nameIdx = emitConstant(Value::str(ident->name));
            emit(OpCode::OP_SET_GLOBAL);
            emitUint32(nameIdx);
        }
        return true;
    }
    // Member expressions: obj.x++, arr[i]++, obj[prop]++
    if (auto member = dynamic_cast<MemberExpression*>(expr->argument.get())) {
        if (member->computed) {
            // Computed: arr[i]++ or obj[prop]++
            member->object->accept(*this);     // [obj]
            member->property->accept(*this);   // [obj, key]
            emit(OpCode::OP_DUP2);             // [obj, key, obj, key]
            emit(OpCode::OP_GET_PROPERTY_DYNAMIC); // [obj, key, value]
            emit(OpCode::LoadConstant);
            emitUint32(emitConstant(Value::number(1)));
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB); // [obj, key, result]
            emit(OpCode::OP_SET_PROPERTY_DYNAMIC); // [result]
            return true;
        } else {
            // Non-computed: obj.x++
            member->object->accept(*this); // [obj]
            emit(OpCode::OP_DUP);          // [obj, obj]
            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::OP_GET_PROPERTY);
            emitUint32(nameIdx);           // [obj, value]
            emit(OpCode::LoadConstant);
            emitUint32(emitConstant(Value::number(1)));
            emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB); // [obj, result]
            emit(OpCode::OP_SET_PROPERTY);
            emitUint32(nameIdx);           // [result]
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
    int jumpAddr = emitJump(OpCode::OP_JUMP);
    loopStack.back().breaks.push_back(jumpAddr);
    return true;
}

R CodeGen::visitContinue(ContinueStatement* stmt) {
    if (loopStack.empty()) {
        throw ("Continue outside loop");
        return false;
    }
    int loopStart = loopStack.back().loopStart;
    emitLoop(loopStart); // emit a backwards jump
    return true;
}

R CodeGen::visitThrow(ThrowStatement* stmt) {
    // Evaluate the exception value
    stmt->argument->accept(*this);
    emit(OpCode::OP_THROW);
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
    // create new object, push, then call constructor? For now create object and set properties
    // expr->arguments // vector
    // expr->callee //
        
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
    
    // set up constructor invocation by calling callee with object? Simpler: if args exist, ignore
    // To support calling constructor I'd have to add OP_INVOKE or convention; skip constructor calls for now.
    return true;
}

void CodeGen::compileMethod(MethodDefinition& method) {
    // Create a nested CodeGen for the method body (closure)
    CodeGen nested(module_);
    nested.enclosing = this;
    nested.cur = std::make_shared<Chunk>();
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
        if (info.isRest) {
            nested.emit(OpCode::OP_LOAD_ARGUMENTS);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_SLICE);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
            continue;
        }
        if (info.hasDefault) {
            nested.emit(OpCode::OP_LOAD_ARGUMENTS_LENGTH);
            nested.emit(OpCode::LoadConstant);
            nested.emitUint32(nested.emitConstant(Value::number(i)));
            nested.emit(OpCode::OP_GREATER);
            int useArg = nested.emitJump(OpCode::OP_JUMP_IF_FALSE);
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            int setLocalJump = nested.emitJump(OpCode::OP_JUMP);
            nested.patchJump(useArg);
            info.defaultExpr->accept(nested);
            nested.patchJump(setLocalJump);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
        } else {
            nested.emit(OpCode::OP_LOAD_ARGUMENT);
            nested.emitUint32((uint32_t)i);
            nested.emit(OpCode::OP_SET_LOCAL);
            nested.emitUint32(nested.getLocal(info.name));
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
            nested.emit(OpCode::LoadConstant);
            int ud = nested.emitConstant(Value::undefined());
            nested.emitUint32(ud);
            nested.emit(OpCode::OP_RETURN);
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
    emit(OpCode::OP_CLOSURE);
    emitUint8((uint8_t)ci);

    for (auto& uv : nested.upvalues) {
        emitUint8(uv.isLocal ? 1 : 0);
        emitUint8(uv.index);
    }
    
    disassembleChunk(nested.cur.get(), method.name);

}

R CodeGen::visitClass(ClassDeclaration* stmt) {
        
    // Evaluate superclass (if any)
    if (stmt->superClass) {
        stmt->superClass->accept(*this); // [superclass]
    } else {
        emit(OpCode::LoadConstant);
        emitUint32(emitConstant(Value::nullVal())); // or Value::undefined()
    }

    // Create the class object (with superclass on stack)
    emit(OpCode::OP_NEW_CLASS); // pops superclass, pushes new class object

    // Define fields
    for (auto& field : stmt->fields) {
        // Only handle static fields during class definition codegen
        bool isStatic = false;
        for (const auto& mod : field->modifiers) {
            if (auto* staticKW = dynamic_cast<StaticKeyword*>(mod.get())) {
                isStatic = true;
                break;
            }
        }
        // if (!isStatic)
            // continue;

        // Property is always a VariableStatement
        if (auto* varStmt = dynamic_cast<VariableStatement*>(field->property.get())) {
            for (const auto& decl : varStmt->declarations) {
                
                classInfo.fields.insert(decl.id);
                
                if (decl.init) {
                    decl.init->accept(*this); // Evaluate initializer
                } else {
                    emit(OpCode::LoadConstant);
                    emitUint32(emitConstant(Value::undefined()));
                }
                int nameIdx = emitConstant(Value::str(decl.id));
                
                if (isStatic) {
                    emit(OpCode::OP_SET_STATIC_PROPERTY);
                } else {
                    emit(OpCode::OP_SET_PROPERTY);
                }
                
                emitUint32(nameIdx);
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
        // Compile the method as a function object
        compileMethod(*method); // leaves function object on stack

        // Get name of method
        int nameIdx = emitConstant(Value::str(method->name));
        
        // Check if 'static' modifier is present
        bool isStatic = false;
        for (const auto& mod : method->modifiers) {
            if (auto* staticKW = dynamic_cast<StaticKeyword*>(mod.get())) {
                isStatic = true;
                break;
            }
        }

        if (isStatic) {
            emit(OpCode::OP_SET_STATIC_PROPERTY);
            emitUint32(nameIdx); // Pops class and function, sets property on class object
        } else {
            emit(OpCode::OP_SET_PROPERTY);
            emitUint32(nameIdx); // Pops class and function, sets on prototype
        }
    }

    // Bind class in the environment (global)
    int classNameIdx = emitConstant(Value::str(stmt->id));
    emit(OpCode::OP_DEFINE_GLOBAL);
    emitUint32(classNameIdx);

    // clear class info
    classInfo.fields.clear();

    return true;
}

R CodeGen::visitMethodDefinition(MethodDefinition* stmt) {
    return true;
}

R CodeGen::visitDoWhile(DoWhileStatement* stmt) {

    beginLoop();
    loopStack.back().loopStart = (int)cur->size();
    
    // Mark loop start
    size_t loopStart = cur->size();
    
    // Compile loop body
    stmt->body->accept(*this);

    // Compile the condition
    stmt->condition->accept(*this);

    // Jump back to loop start if condition is true
    int condJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emit(OpCode::OP_POP); // pop condition

    // Emit loop back
    emitLoop((uint32_t)(cur->size() - loopStart + 4 + 1));

    // Patch the jump to after the loop if condition is false
    patchJump(condJump);
    emit(OpCode::OP_POP); // pop condition
    
    endLoop();

    return true;
}

R CodeGen::visitSwitchCase(SwitchCase* stmt) {
    // Each case's test should have already been checked in visitSwitch
    // Emit the body of this case
    for (auto& s : stmt->consequent) {
        if (auto break_stmt = dynamic_cast<BreakStatement*>(s.get())) {
            continue;
        } else {
            s->accept(*this);

        }
    }
    return true;
}

R CodeGen::visitSwitch(SwitchStatement* stmt) {
    std::vector<int> caseJumps;
    int defaultJump = -1;

    // Evaluate the discriminant and leave on stack
    stmt->discriminant->accept(*this);

    // Emit checks for each case
    for (size_t i = 0; i < stmt->cases.size(); ++i) {
        SwitchCase* scase = stmt->cases[i].get();
        if (scase->test) {
            // Duplicate discriminant for comparison
            emit(OpCode::OP_DUP);
            scase->test->accept(*this);
            emit(OpCode::OP_EQUAL);

            // If not equal, jump to next
            int jump = emitJump(OpCode::OP_JUMP_IF_FALSE);
            emit(OpCode::OP_POP); // pop comparison result

            // If equal, pop discriminant, emit case body, and jump to end
            emit(OpCode::OP_POP);
            scase->accept(*this);
            caseJumps.push_back(emitJump(OpCode::OP_JUMP));
            patchJump(jump);
        } else {
            // Default case: remember its position
            defaultJump = (int)cur->size();
            // pop discriminant for default
            emit(OpCode::OP_POP);
            scase->accept(*this);
            // No jump needed after default
        }
    }

    // Patch all jumps to here (after switch body)
    for (int jmp : caseJumps) {
        patchJump(jmp);
    }

    return true;
}

R CodeGen::visitCatch(CatchClause* stmt) {
    stmt->body->accept(*this);
    return true;
}

R CodeGen::visitTry(TryStatement* stmt) {
    // Mark start of try
    int tryPos = emitTryPlaceholder();

    // Compile try block
    stmt->block->accept(*this);

    // End of try
    emit(OpCode::OP_END_TRY);

    int endJump = -1;

    // If there's a catch, emit jump over it for normal completion
    if (stmt->handler) {
        endJump = emitJump(OpCode::OP_JUMP);
        // Patch catch offset
        patchTryCatch(tryPos, (int)cur->size());

        beginScope();
        
        // Bind catch parameter (VM leaves exception value on stack)
        declareLocal(stmt->handler->param);
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
        emit(OpCode::OP_END_FINALLY);
    }

    return true;
}

R CodeGen::visitForIn(ForInStatement* stmt) {
    
    // object, body, init
    
    // loop through the keys of the object
    
    // load object to stack
    stmt->object->accept(*this);
    
    emit(OpCode::OP_DUP);
    // get keys of object
    emit(OpCode::OP_ENUM_KEYS);
    uint32_t keys_slot = makeLocal("__for_in_keys");
    emit(OpCode::OP_SET_LOCAL);
    emitUint32(keys_slot);
    // emit(OpCode::OP_POP);

    emit(OpCode::OP_DUP);
    emit(OpCode::OP_GET_OBJ_LENGTH);
    
    uint32_t length_slot = makeLocal("__for_in_length");
    emit(OpCode::OP_SET_LOCAL);
    emitUint32(length_slot);

    uint32_t idx_slot = makeLocal("__for_in_idx");
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::number(0)));
    emit(OpCode::OP_SET_LOCAL);
    emitUint32(idx_slot);
    
    // emit(OpCode::OP_POP);
    // emit(OpCode::OP_POP);
    
    size_t loop_start = cur->size();
    beginLoop();
    loopStack.back().loopStart = (int)loop_start;

    // get both len and idx
    emit(OpCode::OP_GET_LOCAL);
    emitUint32(idx_slot);

    emit(OpCode::OP_GET_LOCAL);
    emitUint32(length_slot);

    emit(OpCode::OP_NOTEQUAL);
    int jump_if_false = emitJump(OpCode::OP_JUMP_IF_FALSE);
    
    emit(OpCode::OP_POP);

    emit(OpCode::OP_DUP);

    // Get key at idx: keys[idx]
    emit(OpCode::OP_GET_LOCAL);
    emitUint32(keys_slot);
    emit(OpCode::OP_GET_LOCAL);
    emitUint32(idx_slot);
    emit(OpCode::OP_GET_PROPERTY_DYNAMIC);

    // Assign value to loop variable
    if (auto* ident = dynamic_cast<IdentifierExpression*>(stmt->init.get())) {
        uint32_t slot = makeLocal(ident->name);
        emit(OpCode::OP_SET_LOCAL);
        emitUint32(slot);
    } else if (auto* var_stmt = dynamic_cast<VariableStatement*>(stmt->init.get())) {
        uint32_t slot = makeLocal(var_stmt->declarations[0].id);
        emit(OpCode::OP_SET_LOCAL);
        emitUint32(slot);
    } else {
        throw std::runtime_error("for-in only supports identifier/variable statement loop variables in codegen");
    }
    
    // get size of the keys
    emit(OpCode::OP_NOP);
    stmt->body->accept(*this);
    emit(OpCode::OP_NOP);
    
    // Increment idx
    emit(OpCode::OP_GET_LOCAL);
    emitUint32(idx_slot);
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::number(1)));
    emit(OpCode::OP_ADD);
    emit(OpCode::OP_SET_LOCAL);
    emitUint32(idx_slot);
    
    // loop till stack is empty
    emit(OpCode::OP_LOOP);
    emitUint32((int)cur->size() - (uint32_t)loop_start + 4 + 1);
    
    patchJump(jump_if_false);
    emit(OpCode::OP_POP);
    
    endLoop();
    emit(OpCode::OP_CLEAR_STACK);
    return true;
    
}

// TODO: we need to make sure statement leaves nothing on stack
// TODO: for..of reads starts off by 1
R CodeGen::visitForOf(ForOfStatement* stmt) {

    // std::unique_ptr<Statement> left; // variable declaration or expression
    // std::unique_ptr<Expression> right; // iterable expression
    // std::unique_ptr<Statement> body;
    emit(OpCode::OP_CLEAR_STACK);

    // assume, right must directly hold an object literal
    stmt->right->accept(*this);
        
    emit(OpCode::OP_DUP);

    size_t __for_of_array_slot = makeLocal("__for_of_array");
    emit(OpCode::OP_SET_LOCAL);
    emitUint32((uint32_t)__for_of_array_slot);

    emit(OpCode::OP_GET_OBJ_LENGTH);
    // get array length
    size_t length_slot = makeLocal("__for_of_length");
    emit(OpCode::OP_SET_LOCAL);
    emitUint32((uint32_t)length_slot);
    // emit(OpCode::OP_POP);

    size_t idx_slot = makeLocal("__for_of_index");
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value(0)));
    emit(OpCode::OP_SET_LOCAL);
    emitUint32((uint32_t)idx_slot);

    // emit(OpCode::OP_POP);

    int loop_start = (int)cur->size();
    
    beginLoop();
    loopStack.back().loopStart = loop_start;

    // get both len and idx
    emit(OpCode::OP_GET_LOCAL);
    emitUint32((uint32_t)idx_slot);

    emit(OpCode::OP_GET_LOCAL);
    emitUint32((uint32_t)length_slot);
    
    emit(OpCode::OP_NOTEQUAL);
    
    int jump_if_false = emitJump(OpCode::OP_JUMP_IF_FALSE);

    // emit(OpCode::OP_DUP);
    emit(OpCode::OP_GET_LOCAL);
    emitUint32((uint32_t)__for_of_array_slot);

    emit(OpCode::OP_GET_LOCAL);
    emitUint32((uint32_t)idx_slot);
    emit(OpCode::OP_GET_PROPERTY_DYNAMIC);

    // Assign value to loop variable
    if (auto* ident = dynamic_cast<IdentifierExpression*>(stmt->left.get())) {
        uint32_t slot = makeLocal(ident->name);
        emit(OpCode::OP_SET_LOCAL);
        emitUint32(slot);
    } else if (auto* var_stmt = dynamic_cast<VariableStatement*>(stmt->left.get())) {
        uint32_t slot = makeLocal(var_stmt->declarations[0].id);
        emit(OpCode::OP_SET_LOCAL);
        emitUint32(slot);
    } else {
        throw std::runtime_error("for-of only supports identifier/variable statement loop variables in codegen");
    }

    stmt->body->accept(*this);

    // emit(OpCode::OP_POP);
    
    // increment idx
    // push idx
    emit(OpCode::OP_GET_LOCAL);
    emitUint32((uint32_t)idx_slot);
    emit(OpCode::LoadConstant);
    emitUint32(emitConstant(Value::number(1)));
    emit(OpCode::OP_ADD);
    emit(OpCode::OP_SET_LOCAL);
    emitUint32((uint32_t)idx_slot);

    // emit(OpCode::OP_POP);
    // emit(OpCode::OP_POP);

    emitLoop((int)cur->size() - loop_start + 4 + 1);
    patchJump(jump_if_false);
    // emit(OpCode::OP_POP);
    
    endLoop();
    
    emit(OpCode::OP_CLEAR_STACK);
    
    // we need to clear locals.
    // emit(OpCode::OP_CLEAR_LOCALS);

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

R CodeGen::visitClassExpression(ClassExpression* expr) {
    return true;
}

// --------------------- Utils ----------------------

//void CodeGen::resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames) {
//    locals.clear();
//    nextLocalSlot = 0;
//    // reserve param slots as locals 0..paramCount-1
//    for (uint32_t i = 0; i < paramCount; ++i) {
//        string name = (i < paramNames.size()) ? paramNames[i] : ("_p" + std::to_string(i));
//        locals[name] = i;
//        nextLocalSlot = i + 1;
//    }
//    if (cur) cur->maxLocals = nextLocalSlot;
//}

void CodeGen::resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames) {
    locals.clear();
    nextLocalSlot = 0;
    for (uint32_t i = 0; i < paramCount; ++i) {
        string name = (i < paramNames.size()) ? paramNames[i] : ("_p" + std::to_string(i));
        Local local{name, /*depth=*/1, /*isCaptured=*/false, (uint32_t)i}; // usually scopeDepth=1 for params
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

//uint32_t CodeGen::makeLocal(const string &name) {
//    auto it = locals.find(name);
//    if (it != locals.end()) return it->second;
//    uint32_t idx = nextLocalSlot++;
//    locals[name] = idx;
//    if (idx + 1 > cur->maxLocals) cur->maxLocals = idx + 1;
//    return idx;
//}

uint32_t CodeGen::makeLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return locals[i].slot_index;
    }
    uint32_t idx = (uint32_t)locals.size();
    Local local{name, scopeDepth, false, idx};
    locals.push_back(local);
    if (idx + 1 > cur->maxLocals) cur->maxLocals = idx + 1;
    return idx;
}

//bool CodeGen::hasLocal(const string &name) {
//    return locals.find(name) != locals.end();
//}

bool CodeGen::hasLocal(const std::string& name) {
    for (int i = (int)locals.size() - 1; i >= 0; --i) {
        if (locals[i].name == name) return true;
    }
    return false;
}

//uint32_t CodeGen::getLocal(const string &name) {
//    return locals.at(name);
//}

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
    emit(OpCode::OP_LOOP);
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
    emit(OpCode::OP_TRY);

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

//int CodeGen::declareLocal(const string& name) {
//    int slot = nextLocalSlot++;
//    locals[name].slot_index = slot;
//    return slot;
//}

void CodeGen::declareLocal(const string& name) {
    if (scopeDepth == 0) return; // globals aren’t locals

    // prevent shadowing in same scope
    for (int i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].depth != -1 && locals[i].depth < scopeDepth) break;
        if (locals[i].name == name) {
            throw runtime_error("Variable already declared in this scope");
        }
    }

    Local local{ name, scopeDepth, false, (uint32_t)locals.size() };
    locals.push_back(local);
}

void CodeGen::emitSetLocal(int slot) {
    emit(OpCode::OP_SET_LOCAL);
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

int CodeGen::addUpvalue(bool isLocal, int index) {
    for (int i = 0; i < (int)upvalues.size(); i++) {
        if (upvalues[i].isLocal == isLocal && upvalues[i].index == index) {
            return i;
        }
    }
    upvalues.push_back({isLocal, (uint32_t)index});
    return (int)upvalues.size() - 1;
}

    // resolve variable
int CodeGen::resolveUpvalue(const std::string& name) {
    if (enclosing) {
        int localIndex = enclosing->resolveLocal(name);
        if (localIndex != -1) {
            enclosing->locals[localIndex].isCaptured = true;
            return addUpvalue(true, localIndex);
        }
        
        int upIndex = enclosing->resolveUpvalue(name);
        if (upIndex != -1) {
            return addUpvalue(false, upIndex);
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
            emit(OpCode::OP_CLOSE_UPVALUE);
        } else {
            // Normal local → just pop
            emit(OpCode::OP_POP);
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

inline uint32_t readUint32(const Chunk* chunk, size_t offset) {
    return (uint32_t)chunk->code[offset] |
           ((uint32_t)chunk->code[offset + 1] << 8) |
           ((uint32_t)chunk->code[offset + 2] << 16) |
           ((uint32_t)chunk->code[offset + 3] << 24);
}

size_t CodeGen::disassembleInstruction(const Chunk* chunk, size_t offset) {
    std::cout << std::setw(4) << offset << " ";

    uint8_t instruction = chunk->code[offset];
    OpCode op = static_cast<OpCode>(instruction);

    switch (op) {
            // no operands
        case OpCode::OP_NOP:
        case OpCode::OP_POP:
        case OpCode::OP_DUP:
        case OpCode::OP_ADD:
        case OpCode::OP_SUB:
        case OpCode::OP_MUL:
        case OpCode::OP_DIV:
        case OpCode::OP_MOD:
        case OpCode::OP_POW:
        case OpCode::OP_NEGATE:
        case OpCode::OP_NOT:
        case OpCode::OP_EQUAL:
        case OpCode::OP_NOTEQUAL:
        case OpCode::OP_LESS:
        case OpCode::OP_LESSEQUAL:
        case OpCode::OP_GREATER:
        case OpCode::OP_GREATEREQUAL:
        case OpCode::OP_NEW_OBJECT:
        case OpCode::OP_NEW_ARRAY:
        case OpCode::OP_RETURN:
        case OpCode::OP_HALT:
            
        case OpCode::OP_INCREMENT:
            
        case OpCode::LOGICAL_AND:
        case OpCode::LOGICAL_OR:
        case OpCode::NULLISH_COALESCING:
        case OpCode::REFERENCE_EQUAL:
        case OpCode::STRICT_INEQUALITY:
        case OpCode::OP_DECREMENT:
            
            // bitwise
        case OpCode::OP_BIT_AND:
        case OpCode::OP_BIT_OR:
        case OpCode::OP_BIT_XOR:
        case OpCode::OP_SHL:
        case OpCode::OP_SHR:
        case OpCode::OP_USHR:
        case OpCode::OP_POSITIVE:
        case OpCode::OP_DUP2:
        case OpCode::OP_SET_PROPERTY_DYNAMIC:
        case OpCode::OP_GET_PROPERTY_DYNAMIC:
            
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
            
        case OpCode::OP_GET_LOCAL:
        case OpCode::OP_SET_LOCAL: {
            uint32_t slot = readUint32(chunk, offset + 1);
            std::cout << opcodeToString(op) << " " << slot << "\n";
            return offset + 1 + 4;
        }
            
        case OpCode::GetThisProperty:
        case OpCode::SetThisProperty:
        case OpCode::OP_GET_UPVALUE:
        case OpCode::OP_SET_UPVALUE:
        case OpCode::OP_GET_GLOBAL:
        case OpCode::OP_SET_GLOBAL:
        case OpCode::OP_DEFINE_GLOBAL:
        case OpCode::OP_SET_PROPERTY:
        case OpCode::OP_SET_STATIC_PROPERTY:
        case OpCode::OP_LOAD_CHUNK_INDEX:
        case OpCode::OP_LOAD_ARGUMENT:
        case OpCode::OP_GET_PROPERTY: {
            uint32_t nameIndex = readUint32(chunk, offset + 1);
            std::cout << opcodeToString(op) << " constant[" << nameIndex << "]";
            if (nameIndex < chunk->constants.size()) {
                std::cout << " \"" << chunk->constants[nameIndex].toString() << "\"";
            }
            std::cout << "\n";
            return offset + 1 + 4;
        }
            
        case OpCode::OP_CLOSURE: {
            //            emit(OpCode::OP_CLOSURE);
            //            emitUint8((uint8_t)ci);
            //
            //            // Emit upvalue descriptors
            //            for (auto& uv : nested.upvalues) {
            //                emitUint8(uv.isLocal ? 1 : 0);
            //                emitUint8(uv.index);
            //            }
            
            cout << opcodeToString(op);

            // index: 8 uint
            cout << " module-constant-index: [" << (int)chunk->code[offset + 1] << "]";

            if (upvalues.size() > 0) {
                for (auto& uv : upvalues) {
                    
                    // is local:
                    cout << "isLocal: [" << chunk->code[offset + 1 + 1];
                    cout << "] ";
                    // idx
                    cout << "Index: [" << chunk->code[offset + 1 + 1 + 1] << "]";
                    
                }
            } else {
                cout << endl;
                return offset + 2;
            }
            cout << endl;
            
            return offset + 1 + 1 + 1 + 1;
            
        }
            
        case OpCode::OP_ARRAY_PUSH: {
            std::cout << opcodeToString(op) << "\n";
            return offset + 1;
        }
            
            // jumps
        case OpCode::OP_JUMP:
        case OpCode::OP_JUMP_IF_FALSE:
        case OpCode::OP_LOOP: {
            uint32_t jump = readUint32(chunk, offset + 1);
            size_t target = (op == OpCode::OP_LOOP) ? offset - jump : offset + 5 + jump;
            std::cout << opcodeToString(op) << " " << target << "\n";
            return offset + 1 + 4;
        }
            
        case OpCode::CreateInstance:
            // calls
        case OpCode::SuperCall:
        case OpCode::OP_CALL: {
            uint8_t argCount = chunk->code[offset + 1];
            std::cout << opcodeToString(op) << " " << (int)argCount << " args\n";
            return offset + 2;
        }
            
        case OpCode::GetThis:
        case OpCode::OP_NEW_CLASS:
        case OpCode::OP_TRY:
        case OpCode::OP_END_TRY:
        case OpCode::OP_END_FINALLY:
        case OpCode::OP_THROW:
        case OpCode::OP_ENUM_KEYS:
        case OpCode::OP_GET_OBJ_LENGTH:
        case OpCode::OP_GET_INDEX_PROPERTY_DYNAMIC:
        case OpCode::OP_DEBUG:
        case OpCode::OP_LOAD_ARGUMENTS:
        case OpCode::OP_SLICE:
        case OpCode::OP_LOAD_ARGUMENTS_LENGTH:
        case OpCode::OP_CLOSE_UPVALUE:
        case OpCode::OP_CLEAR_STACK:
        case OpCode::OP_CLEAR_LOCALS:
        case OpCode::InvokeConstructor:
        case OpCode::GetParentObject:
            cout << opcodeToString(op) << "\n";
            return offset + 1;
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

Token createToken(TokenType type) {
    Token token;
    token.type = type;
    return token;
}
