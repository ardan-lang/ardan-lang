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

```
