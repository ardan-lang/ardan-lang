//
//  CodeGenerator.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "CodeGenerator.hpp"

CodeGen::CodeGen() : cur(nullptr), nextLocalSlot(0) { }

shared_ptr<Chunk> CodeGen::generate(const vector<unique_ptr<Statement>> &program) {
    cur = std::make_shared<Chunk>();
    cur->name = "BYTECODE";
    locals.clear();
    nextLocalSlot = 0;

    for (const auto &s : program) {
        s->accept(*this);
    }

    emit(OpCode::OP_HALT);
    disassembleChunk(cur.get(), cur->name);
    return cur;
}

// ------------------- Statements --------------------

R CodeGen::visitExpression(ExpressionStatement* stmt) {
    stmt->expression->accept(*this);
    emit(OpCode::OP_POP);
    return true;
}

R CodeGen::visitBlock(BlockStatement* stmt) {
    // naive: just compile statements in current context (no block-scoped locals handling)
    for (auto& s : stmt->body) {
        s->accept(*this);
    }
    return true;
}

R CodeGen::visitVariable(VariableStatement* stmt) {
    // currently, only support single declarator for simplicity (like interpreter)
    
    // var, let and const
    string kind = stmt->kind;
    
    for (auto &decl : stmt->declarations) {
        
        if (decl.init) {
            decl.init->accept(*this); // push init value
        } else {
            emit(OpCode::OP_CONSTANT);
            int ci = emitConstant(Value::undefined());
            emitUint32(ci);
        }
        
        // decide local or global
        if (hasLocal(decl.id)) {
            uint32_t idx = getLocal(decl.id);
            emit(OpCode::OP_SET_LOCAL);
            emitUint32(idx);
        } else {
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
    // loop start
    size_t loopStart = cur->size();
    stmt->test->accept(*this);
    int exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);
    emit(OpCode::OP_POP);
    stmt->body->accept(*this);
    // jump back to loop start
    uint32_t backOffset = (uint32_t)(cur->size() - loopStart + 4); // estimate; we'll write OP_LOOP with offset
    // simpler: compute distance as current ip - loopStart + 4?
    // but we have OP_LOOP that expects offset to subtract; we'll write offset below:
    emitLoop((uint32_t)(cur->size() - loopStart + 4));
    patchJump(exitJump);
    emit(OpCode::OP_POP);
    return true;
}

R CodeGen::visitFor(ForStatement* stmt) {
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
        emitLoop((uint32_t)(cur->size() - loopStart + 4));
        patchJump(exitJump);
        emit(OpCode::OP_POP);
    } else {
        // infinite loop
        stmt->body->accept(*this);
        if (stmt->update) stmt->update->accept(*this);
        emitLoop((uint32_t)(cur->size() - loopStart + 4));
    }
    return true;
}

R CodeGen::visitReturn(ReturnStatement* stmt) {
    if (stmt->argument) {
        stmt->argument->accept(*this);
    } else {
        int ci = emitConstant(Value::undefined());
        emit(OpCode::OP_CONSTANT);
        emitUint32(ci);
    }
    emit(OpCode::OP_RETURN);
    return true;
}

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

// ------------------- Expressions --------------------

R CodeGen::visitBinary(BinaryExpression* expr) {
    expr->left->accept(*this);
    expr->right->accept(*this);

    switch (expr->op.type) {
        case TokenType::ADD: emit(OpCode::OP_ADD); break;
        case TokenType::MINUS: emit(OpCode::OP_SUB); break;
        case TokenType::MUL: emit(OpCode::OP_MUL); break;
        case TokenType::DIV: emit(OpCode::OP_DIV); break;
        case TokenType::MODULI: emit(OpCode::OP_MOD); break;
        case TokenType::POWER: emit(OpCode::OP_POW); break;
        case TokenType::VALUE_EQUAL: emit(OpCode::OP_EQUAL); break;
        case TokenType::INEQUALITY: emit(OpCode::OP_NOTEQUAL); break;
        case TokenType::LESS_THAN: emit(OpCode::OP_LESS); break;
        case TokenType::LESS_THAN_EQUAL: emit(OpCode::OP_LESSEQUAL); break;
        case TokenType::GREATER_THAN: emit(OpCode::OP_GREATER); break;
        case TokenType::GREATER_THAN_EQUAL: emit(OpCode::OP_GREATEREQUAL); break;
        default:
            throw std::runtime_error("Unsupported binary op in CodeGen: " + expr->op.lexeme);
    }
    return true;
}

R CodeGen::visitLiteral(LiteralExpression* expr) {
    // literal token lexeme (strings / keywords etc)
    // we'll treat only some literals; numeric literals handled separately
    int idx = emitConstant(Value::str(expr->token.lexeme));
    emit(OpCode::OP_CONSTANT);
    emitUint32(idx);
    return true;
}

R CodeGen::visitNumericLiteral(NumericLiteral* expr) {
    Value v = toValue(expr->value);
    int idx = emitConstant(v);
    emit(OpCode::OP_CONSTANT);
    emitUint32(idx);
    return true;
}

R CodeGen::visitStringLiteral(StringLiteral* expr) {
    int idx = emitConstant(Value::str(expr->text)); // sets the text to the constant array
    emit(OpCode::OP_CONSTANT); // bytecode that indicates push constant to the stack.
    emitUint32(idx); // the index of the constant in the constants array to push to the stack
    return true;
}

R CodeGen::visitIdentifier(IdentifierExpression* expr) {
    string name = expr->name;
    if (hasLocal(name)) {
        emit(OpCode::OP_GET_LOCAL);
        emitUint32(getLocal(name));
    } else {
        int nameIdx = emitConstant(Value::str(name));
        emit(OpCode::OP_GET_GLOBAL);
        emitUint32(nameIdx);
    }
    return true;
}

R CodeGen::visitCall(CallExpression* expr) {
    // emit callee, then args left-to-right, then OP_CALL argc
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
        // pop the computed property -> evaluate to constant string at runtime not supported here
        // Simplify: only support non-computed for codegen for now
        throw std::runtime_error("Computed member expressions not supported by this CodeGen yet.");
    } else {
        propName = expr->name.lexeme;
    }

    int nameIdx = emitConstant(Value::str(propName));
    emit(OpCode::OP_GET_PROPERTY);
    emitUint32(nameIdx);
    return true;
}

R CodeGen::visitNew(NewExpression* expr) {
    // create new object, push, then call constructor? For now create object and set properties
    emit(OpCode::OP_NEW_OBJECT);
    // set up constructor invocation by calling callee with object? Simpler: if args exist, ignore
    // To support calling constructor I'd have to add OP_INVOKE or convention; skip constructor calls for now.
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
    emit(OpCode::OP_POP);
    expr->consequent->accept(*this);
    int endJump = emitJump(OpCode::OP_JUMP);
    patchJump(elseJump);
    emit(OpCode::OP_POP);
    expr->alternate->accept(*this);
    patchJump(endJump);
    return true;
}

R CodeGen::visitUnary(UnaryExpression* expr) {
    expr->right->accept(*this);
    switch (expr->op.type) {
        case TokenType::LOGICAL_NOT: emit(OpCode::OP_NOT); break;
        case TokenType::MINUS: emit(OpCode::OP_NEGATE); break;
        default:
            throw std::runtime_error("Unsupported unary op in CodeGen");
    }
    return true;
}

R CodeGen::visitArrowFunction(ArrowFunction* expr) {
    // compile arrow to a function chunk similarly to visitFunction
    // Only support simple param list (Identifier or sequence) and expression or statement body
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
    return true;
}

R CodeGen::visitFunctionExpression(FunctionExpression* expr) {
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

    return true;
}

R CodeGen::visitTemplateLiteral(TemplateLiteral* expr) {
    // produce string by concatenating quasis and evaluated expressions.
    // Simple approach: compute at runtime building string.
    // push empty string
    int emptyIdx = emitConstant(Value::str(""));
    emit(OpCode::OP_CONSTANT);
    emitUint32(emptyIdx);

    size_t qsize = expr->quasis.size();
    size_t esize = expr->expressions.size();

    for (size_t i = 0; i < qsize; ++i) {
        // push quasi
        int qi = emitConstant(Value::str(expr->quasis[i]->text));
        emit(OpCode::OP_CONSTANT);
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

R CodeGen::visitImportDeclaration(ImportDeclaration* stmt) {
    // Keep simple: perform import at compile time by executing parser+interpreter OR
    // emit runtime call to some import builtin. For now, call a builtin global "import" if present.
    // Generate: push path string -> call import(path)
    int ci = emitConstant(Value::str(stmt->path.lexeme));
    emit(OpCode::OP_CONSTANT);
    emitUint32(ci);
    // callee (import)
    int importName = emitConstant(Value::str("import"));
    emit(OpCode::OP_GET_GLOBAL);
    emitUint32(importName);
    emit(OpCode::OP_CALL);
    emitUint8(1);
    emit(OpCode::OP_POP);
    return true;
}

R CodeGen::visitAssignment(AssignmentExpression* expr) {
    return true;
}

R CodeGen::visitLogical(LogicalExpression* expr) {
    return true;
}

R CodeGen::visitThis(ThisExpression* expr) {
    return true;
}

R CodeGen::visitSuper(SuperExpression* expr) {
    return true;
}

R CodeGen::visitProperty(PropertyExpression* expr) {
    return true;
}

R CodeGen::visitSequence(SequenceExpression* expr) {
    return true;
}

R CodeGen::visitUpdate(UpdateExpression* expr) {
    return true;
}

R CodeGen::visitFalseKeyword(FalseKeyword* expr) {
    return true;
}

R CodeGen::visitTrueKeyword(TrueKeyword* expr) {

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

R CodeGen::visitNullKeyword(NullKeyword* expr) {
    return true;
}

R CodeGen::visitUndefinedKeyword(UndefinedKeyword* expr) {
    
    return true;
}

R CodeGen::visitAwaitExpression(AwaitExpression* expr) {
    
    return true;
}

R CodeGen::visitBreak(BreakStatement* stmt) {
    
    return true;
}

R CodeGen::visitContinue(ContinueStatement* stmt) {
    return true;
}

R CodeGen::visitThrow(ThrowStatement* stmt) {
    return true;
}

R CodeGen::visitEmpty(EmptyStatement* stmt) {
    return true;
}

R CodeGen::visitClass(ClassDeclaration* stmt) {
    return true;
}

R CodeGen::visitMethodDefinition(MethodDefinition* stmt) {
    return true;
}

R CodeGen::visitDoWhile(DoWhileStatement* stmt) {
    return true;
}

R CodeGen::visitSwitchCase(SwitchCase* stmt) {
    return true;
}

R CodeGen::visitSwitch(SwitchStatement* stmt) {
    return true;
}

R CodeGen::visitCatch(CatchClause* stmt) {
    return true;
}

R CodeGen::visitTry(TryStatement* stmt) {
    return true;
}

R CodeGen::visitForIn(ForInStatement* stmt) {
    return true;
}

R CodeGen::visitForOf(ForOfStatement* stmt) {
    return true;
}

// --------------------- Utils ----------------------

void CodeGen::resetLocalsForFunction(uint32_t paramCount, const vector<string>& paramNames) {
    locals.clear();
    nextLocalSlot = 0;
    // reserve param slots as locals 0..paramCount-1
    for (uint32_t i = 0; i < paramCount; ++i) {
        string name = (i < paramNames.size()) ? paramNames[i] : ("_p" + std::to_string(i));
        locals[name] = i;
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

uint32_t CodeGen::makeLocal(const string &name) {
    auto it = locals.find(name);
    if (it != locals.end()) return it->second;
    uint32_t idx = nextLocalSlot++;
    locals[name] = idx;
    if (idx + 1 > cur->maxLocals) cur->maxLocals = idx + 1;
    return idx;
}

bool CodeGen::hasLocal(const string &name) {
    return locals.find(name) != locals.end();
}

uint32_t CodeGen::getLocal(const string &name) {
    return locals.at(name);
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

void CodeGen::emitLoop(uint32_t offset) {
    emit(OpCode::OP_LOOP);
    emitUint32(offset);
}

inline uint32_t readUint32(const Chunk* chunk, size_t offset) {
    return (uint32_t)chunk->code[offset] |
           ((uint32_t)chunk->code[offset + 1] << 8) |
           ((uint32_t)chunk->code[offset + 2] << 16) |
           ((uint32_t)chunk->code[offset + 3] << 24);
}

size_t disassembleInstruction(const Chunk* chunk, size_t offset) {
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
            std::cout << opcodeToString(op) << "\n";
            return offset + 1;

        // u32 operands
        case OpCode::OP_CONSTANT: {
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

        case OpCode::OP_GET_GLOBAL:
        case OpCode::OP_SET_GLOBAL:
        case OpCode::OP_DEFINE_GLOBAL:
        case OpCode::OP_SET_PROPERTY:
        case OpCode::OP_GET_PROPERTY: {
            uint32_t nameIndex = readUint32(chunk, offset + 1);
            std::cout << opcodeToString(op) << " name[" << nameIndex << "]";
            if (nameIndex < chunk->constants.size()) {
                std::cout << " \"" << chunk->constants[nameIndex].toString() << "\"";
            }
            std::cout << "\n";
            return offset + 1 + 4;
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

        // calls
        case OpCode::OP_CALL: {
            uint8_t argCount = chunk->code[offset + 1];
            std::cout << opcodeToString(op) << " " << (int)argCount << " args\n";
            return offset + 2;
        }
    }

    std::cout << "UNKNOWN " << (int)instruction << "\n";
    return offset + 1;
}

void disassembleChunk(const Chunk* chunk, const std::string& name) {
    std::cout << "== " << name << " ==\n";
    for (size_t offset = 0; offset < chunk->code.size();) {
        offset = disassembleInstruction(chunk, offset);
    }
}
