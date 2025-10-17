```cpp
//R CodeGen::visitUpdate(UpdateExpression* expr) {
//    // Support both identifiers and member expressions
//    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
//        // Load current value
//        if (hasLocal(ident->name)) {
//            emit(OpCode::OP_GET_LOCAL);
//            emitUint32(getLocal(ident->name));
//        } else {
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::OP_GET_GLOBAL);
//            emitUint32(nameIdx);
//        }
//        // Increment or decrement
//        emit(OpCode::OP_CONSTANT);
//        emitUint32(emitConstant(Value::number(1)));
//        emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB);
//        // Store back
//        if (hasLocal(ident->name)) {
//            emit(OpCode::OP_SET_LOCAL);
//            emitUint32(getLocal(ident->name));
//        } else {
//            int nameIdx = emitConstant(Value::str(ident->name));
//            emit(OpCode::OP_SET_GLOBAL);
//            emitUint32(nameIdx);
//        }
//        return true;
//    }
//    
//    if (auto member = dynamic_cast<MemberExpression*>(expr->argument.get())) {
//        // Evaluate object (once)
//        member->object->accept(*this); // stack: [obj]
//        emit(OpCode::OP_DUP); // [obj, obj]
//        int nameIdx = emitConstant(Value::str(member->name.lexeme));
//        // GET_PROPERTY (consumes one obj)
//        emit(OpCode::OP_GET_PROPERTY);
//        emitUint32(nameIdx); // [obj, value]
//        // push 1
//        emit(OpCode::OP_CONSTANT);
//        emitUint32(emitConstant(Value::number(1)));
//        // apply ++ or --
//        emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB);
//        // SET_PROPERTY (consumes [obj, value])
//        emit(OpCode::OP_SET_PROPERTY);
//        emitUint32(nameIdx);
//        return true;
//    }
//    
//    throw std::runtime_error("Update target must be identifier or member expression");
//}

//R CodeGen::visitUpdate(UpdateExpression* expr) {
//    // Only support identifier for now
//    IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->argument.get());
//    if (!ident) throw std::runtime_error("Update target must be identifier");
//    // Load current value
//    if (hasLocal(ident->name)) {
//        emit(OpCode::OP_GET_LOCAL);
//        emitUint32(getLocal(ident->name));
//    } else {
//        int nameIdx = emitConstant(Value::str(ident->name));
//        emit(OpCode::OP_GET_GLOBAL);
//        emitUint32(nameIdx);
//    }
//    // Increment or decrement
//    emit(OpCode::OP_CONSTANT);
//    emitUint32(emitConstant(Value::number(1)));
//    emit(expr->op.type == TokenType::INCREMENT ? OpCode::OP_ADD : OpCode::OP_SUB);
//    // Store back
//    if (hasLocal(ident->name)) {
//        emit(OpCode::OP_SET_LOCAL);
//        emitUint32(getLocal(ident->name));
//    } else {
//        int nameIdx = emitConstant(Value::str(ident->name));
//        emit(OpCode::OP_SET_GLOBAL);
//        emitUint32(nameIdx);
//    }
//    return true;
//}

// R CodeGen::visitFunctionExpression(FunctionExpression* expr) {
    // compile similarly to FunctionDeclaration but produce value (not define global)
//    CodeGen nested;
//    nested.cur = std::make_shared<Chunk>();
//    vector<string> params;
//    for (auto &p : expr->params) {
//        if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(p.get()))
//            params.push_back(ident->token.lexeme);
//        else params.push_back("");
//    }
//    nested.resetLocalsForFunction((uint32_t)params.size(), params);
//    if (expr->body) expr->body->accept(nested);
//    nested.emit(OpCode::OP_CONSTANT);
//    int ud = nested.emitConstant(Value::undefined());
//    nested.emitUint32(ud);
//    nested.emit(OpCode::OP_RETURN);
//
//    shared_ptr<Chunk> fnChunk = nested.cur;
//    fnChunk->arity = (uint32_t)params.size();
//
//    Value fnValue = Value::function([fnChunk](vector<Value> args) -> Value {
//        VM vm;
//        return vm.run(fnChunk, args);
//    });
//
//    int ci = emitConstant(fnValue);
//    emit(OpCode::OP_CONSTANT);
//    emitUint32(ci);

//    return true;
// }

//R CodeGen::visitArrowFunction(ArrowFunction* expr) {
//    // create nested CodeGen to compile the function body
//    CodeGen nested(module_);
//    nested.cur = std::make_shared<Chunk>();
//    vector<string> params;
//    if (expr->parameters) {
//        if (SequenceExpression* seq = dynamic_cast<SequenceExpression*>(expr->parameters.get())) {
//            for (auto &p : seq->expressions) {
//                if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(p.get()))
//                    params.push_back(ident->token.lexeme);
//                else params.push_back("");
//            }
//        } else if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->parameters.get())) {
//            params.push_back(ident->token.lexeme);
//        }
//    }
//
//    nested.resetLocalsForFunction((uint32_t)params.size(), params);
//
//    if (expr->exprBody) {
//        expr->exprBody->accept(nested);
//        // nested.emit(OpCode::OP_RETURN);
//    } else if (expr->stmtBody) {
//        expr->stmtBody->accept(nested);
////        nested.emit(OpCode::OP_CONSTANT);
////        int ud = nested.emitConstant(Value::undefined());
////        nested.emitUint32(ud);
////        nested.emit(OpCode::OP_RETURN);
//    }
//
//    shared_ptr<Chunk> fnChunk = nested.cur;
//    fnChunk->arity = (uint32_t)params.size();
//
//    // === NEW: register chunk in Module, get chunk index ===
//    uint32_t chunkIndex = module_->addChunk(fnChunk); // module is the current module/context
//
//    // Create the serializable FunctionObject
//    auto fnObj = std::make_shared<FunctionObject>();
//    fnObj->chunkIndex = chunkIndex;
//    fnObj->arity = fnChunk->arity;
//    fnObj->name = "<arrow>"; // or derive from context
//
//    // Add to constants pool as a FUNCTION_REF
//    Value fnValue = Value::functionRef(fnObj);
//    int ci = module_->addConstant(fnValue);
//
//    // Emit OP_CONSTANT (index into module constants)
//    // emit(OpCode::OP_CONSTANT);
//    emit(OpCode::OP_LOAD_CHUNK_INDEX);
//    emitUint32((uint32_t)ci);
//
//    disassembleChunk(nested.cur.get(), nested.cur->name);
//
//    return true;
//}

//R CodeGen::visitArrowFunction(ArrowFunction* expr) {
//    // compile arrow to a function chunk similarly to visitFunction
//    // Only support simple param list (Identifier or sequence) and expression or statement body
//    CodeGen nested;
//    nested.cur = std::make_shared<Chunk>();
//    vector<string> params;
//    if (expr->parameters) {
//        if (SequenceExpression* seq = dynamic_cast<SequenceExpression*>(expr->parameters.get())) {
//            for (auto &p : seq->expressions) {
//                if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(p.get()))
//                    params.push_back(ident->token.lexeme);
//                else params.push_back("");
//            }
//        } else if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->parameters.get())) {
//            params.push_back(ident->token.lexeme);
//        }
//    }
//    nested.resetLocalsForFunction((uint32_t)params.size(), params);
//    if (expr->exprBody) {
//        expr->exprBody->accept(nested);
//        // leave value on stack and return
//        nested.emit(OpCode::OP_RETURN);
//    } else if (expr->stmtBody) {
//        expr->stmtBody->accept(nested);
//        // ensure return
//        nested.emit(OpCode::OP_CONSTANT);
//        int ud = nested.emitConstant(Value::undefined());
//        nested.emitUint32(ud);
//        nested.emit(OpCode::OP_RETURN);
//    }
//    
//    shared_ptr<Chunk> fnChunk = nested.cur;
//    fnChunk->arity = (uint32_t)params.size();
//    
//    Value fnValue = Value::function([fnChunk](vector<Value> args) -> Value {
//        VM vm;
//        return vm.run(fnChunk, args);
//    });
//    
//    int ci = emitConstant(fnValue);
//    emit(OpCode::OP_CONSTANT);
//    emitUint32(ci);
//    return true;
//}
R CodeGen::visitFunction(FunctionDeclaration* stmt) {
    // compile function body into its own chunk
    // create a new CodeGen instance to compile function separately
//    CodeGen nested;
//    // map params as locals in nested
//    vector<string> paramNames;
//    for (auto &p : stmt->params) {
//        if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(p.get())) {
//            paramNames.push_back(ident->token.lexeme);
//        } else {
//            paramNames.push_back(""); // placeholder
//        }
//    }
//    nested.locals.clear(); nested.nextLocalSlot = 0;
//    nested.cur = std::make_shared<Chunk>();
//    nested.cur->name = stmt->id;
//    // allocate local slots for params
//    nested.resetLocalsForFunction((uint32_t)paramNames.size(), paramNames);
//    // compile function body
//    if (stmt->body) stmt->body->accept(nested);
//    // ensure there's a return at end
//    nested.emit(OpCode::OP_CONSTANT);
//    int ud = nested.emitConstant(Value::undefined());
//    nested.emitUint32(ud);
//    nested.emit(OpCode::OP_RETURN);
//
//    // create a Value::function that runs this chunk
//    shared_ptr<Chunk> fnChunk = nested.cur;
//    fnChunk->arity = (uint32_t)paramNames.size();
//
//    Value fnValue = Value::function([fnChunk](vector<Value> args) -> Value {
//        VM vm;
//        // If function needs access to globals, you might want to set vm.globals from outer scope.
//        return vm.run(fnChunk, args);
//    });
//
//    int constIndex = emitConstant(fnValue);
//    emit(OpCode::OP_CONSTANT);
//    emitUint32(constIndex);
//
//    // define global under the function name
//    int nameIdx = emitConstant(Value::str(stmt->id));
//    emit(OpCode::OP_DEFINE_GLOBAL);
//    emitUint32(nameIdx);

    return true;
}

//int CodeGen::emitTryPlaceholder() {
//    emit(OpCode::OP_TRY);
//    // reserve 4 bytes for jump target (like your emitJump)
//    int pos = (int)cur->size();
//    emitUint32(0);
//    return pos; // position of placeholder
//}

//R CodeGen::visitTry(TryStatement* stmt) {
//    // mark start of try
//    int tryStart = (int)cur->size();
//
//    // Reserve handler jump
//    int handlerJump = emitTryPlaceholder();
//
//    // compile try block
//    stmt->block->accept(*this);
//
//    // leave try
//    emit(OpCode::OP_END_TRY);
//
//    int endJump = -1;
//    if (stmt->handler) {
//        endJump = emitJump(OpCode::OP_JUMP); // skip catch if no throw
//    }
//
//    // patch handler here
//    patchTry(handlerJump);
//
//    if (stmt->handler) {
//        // bind catch param (VM leaves exception value on stack)
//        declareLocal(stmt->handler->param);
//        emitSetLocal(paramSlot(stmt->handler->param));
//
//        stmt->handler->body->accept(*this);
//    }
//
//    if (endJump != -1) patchJump(endJump);
//
//    if (stmt->finalizer) {
//        stmt->finalizer->accept(*this);
//        emit(OpCode::OP_END_FINALLY);
//    }
//
//    return true;
//}

//R CodeGen::visitClass(ClassDeclaration* stmt) {
//    
//    // Evaluate superclass (if any)
//    if (stmt->superClass) {
//        stmt->superClass->accept(*this); // [superclass]
//    } else {
//        emit(OpCode::LoadConstant);
//        emitUint32(emitConstant(Value::nullVal())); // or Value::undefined()
//    }
//
//    // Create the class object (with superclass on stack)
//    emit(OpCode::OP_NEW_CLASS); // pops superclass, pushes new class object
//
//    // Define methods and fields
////    for (auto& member : stmt->body) {
////        if (auto* method = dynamic_cast<MethodDefinition*>(member.get())) {
////            // Compile method (could be static or instance)
////            // - Compile as a function object and attach to class
////            // - Convention: static methods go on class, instance methods go on prototype
////            if (method->isStatic) {
////                // Compile function and attach as static property
////                compileMethod(method); // leaves function object on stack
////                int nameIdx = emitConstant(Value::str(method->key));
////                emit(OpCode::OP_SET_STATIC_PROPERTY);
////                emitUint32(nameIdx); // Pops class and function, sets property on class object
////            } else {
////                // Compile as instance method (on prototype)
////                compileMethod(method); // leaves function on stack
////                int nameIdx = emitConstant(Value::str(method->key));
////                emit(OpCode::OP_SET_PROTO_PROPERTY);
////                emitUint32(nameIdx); // Pops class and function, sets on prototype
////            }
////        }
////        // Handle other member types as needed
////    }
//    
//    // vector<unique_ptr<PropertyDeclaration>> fields;
////    for (auto& field : stmt->fields) {
////        // Instance field: record for constructor
////        // Static field: evaluate initializer, set property
////        if (field->isStatic) {
////            if (field->initializer) {
////                field->initializer->accept(*this);
////            } else {
////                emit(OpCode::LoadConstant);
////                emitUint32(emitConstant(Value::undefined()));
////            }
////            int nameIdx = emitConstant(Value::str(field->key));
////            emit(OpCode::OP_SET_STATIC_PROPERTY);
////            emitUint32(nameIdx);
////        }
////
////    }
//
//    // Define class in environment (global/local)
//    int classNameIdx = emitConstant(Value::str(stmt->id));
//    emit(OpCode::OP_DEFINE_GLOBAL);
//    emitUint32(classNameIdx);
//
//    return true;
//}

```

```cpp
void CodeGen::_emitAssignment(BinaryExpression* expr) {
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
            
            if (member->computed) {
            } else {
                
            }
            
            // TODO: review
            // Push object first
            member->object->accept(*this);
            // Then RHS
            expr->right->accept(*this);

            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::SetProperty);
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
        
        if (member->computed) {
            member->object->accept(*this);     // [obj]
            emit(OpCode::Dup);             // [obj, obj]
            member->property->accept(*this);   // [obj, key]
            emit(OpCode::GetPropertyDynamic); // [obj, key, value]
        } else {
            
            // Push object, duplicate it for later use
            member->object->accept(*this);
            emit(OpCode::Dup);
            
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

        // TODO: review

        if (member->computed) {
            emit(OpCode::SetPropertyDynamic);
        } else {
            int nameIdx = emitConstant(Value::str(member->name.lexeme));
            emit(OpCode::SetProperty);
            emitUint32(nameIdx);
        }
        
    }

    // Optional: if compound assignments are statements, discard result:
    // emit(OpCode::OP_POP);
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

// Upvalue* openUpvalues = nullptr; // Head of list of open upvalues
// std::vector<std::shared_ptr<Upvalue>> closureUpvalues; // Optional for management

// Add members to VM class (assumed in VM.hpp):
// Upvalue* openUpvalues = nullptr; // Head of list of open upvalues
// std::vector<std::shared_ptr<Upvalue>> closureUpvalues; // Optional for management

```
