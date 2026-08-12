//
//  IRBuilderVisitor.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/05/2026.
//

#include "IRBuilderVisitor.hpp"

#include <cstdint>

constexpr int32_t kSmiMin = -(1 << 30);
constexpr int32_t kSmiMax =  (1 << 30) - 1;

bool IsSmi(int64_t value) {
    return value >= kSmiMin && value <= kSmiMax;
}

void IRBuilderVisitor::build(const vector<unique_ptr<Statement>> &program) {

    unique_ptr<IRFunction> uniqueCurrentFunction = make_unique<IRFunction>();
    uniqueCurrentFunction->entry_point = true;
    currentFunction = uniqueCurrentFunction.get();

    currentBlock = createBlock("entry");
    
    scopes.push_back({ Scope::Type::Global, nullptr, {} });
    for (const auto &s : program) {
        s->accept(*this);
    }
    
    declare("undefined", {}, BindingKind::Var);
    auto dst_reg = lookup("undefined");
    auto ret = IRInstruction(IROp::Return, nullptr, {dst_reg});
    emit(ret);
    
    scopes.pop_back();
    
    irModule.functions.emplace_back(std::move(uniqueCurrentFunction));
}

R IRBuilderVisitor::visitExpression(ExpressionStatement* stmt) {
    
    return stmt->expression->accept(*this);
    
}


/**
 * This looks up a variable name and returns the register it is stored in.
 */
shared_ptr<IRValue> IRBuilderVisitor::lookup(string name) {
    
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {

        auto found = it->symbols.find(name);

        if (found != it->symbols.end())
            return found->second;
    }
    
    // look at the contexts next
    for (int depth = 0; depth < contexts.size(); depth++) {
                
        auto context = contexts[contexts.size() - 1 - depth];
        auto slot = context.slots.find(name);
        
        if (slot != context.slots.end()) {
            
            // emit LdaContextSlot
            // this loads value from a give slot from the context into acc
            // the context to retrieve from is specified in the depth
            // LdaContextSlot [slot], depth
            // reaches for the context depth in the context chain
            // from the context, go to index slot, get the value there
            // push the value to the dst reg
            
            bool isOwnerCtx = depth == 0 && currentFunctionOwnsTopContextFrame;
            
            auto dst = createTemp(IRType::Any);
            IRInstruction ldaContextSlotOp = isOwnerCtx ? IRInstruction(IROp::LoadCurrentContextSlot,
                                                                                                                dst,
                                                                                                                {
            }) : IRInstruction(IROp::LoadContextSlot,
                               dst,
                               {});
            
            ldaContextSlotOp.contextSlot = slot->second;
            if (isOwnerCtx == false) {
                ldaContextSlotOp.contextDepth = static_cast<int>(depth);
            }
            
            emit(ldaContextSlotOp);
            
            return dst;
            
        }
        
    }

    throw runtime_error("Undefined variable");
    
}

void IRBuilderVisitor::store(string id, shared_ptr<IRValue> reg) {

    // this is called by assign
    // get id from context
    // the reg is the register of the value to store
    
    for (size_t depth = 0; depth < contexts.size(); depth++) {
        
        auto ctx = contexts[contexts.size() - 1 - depth];
        auto found = ctx.slots.find(id);
        
        if (found != ctx.slots.end()) {
            
            IRInstruction irOpInst(IROp::StoreContextSlot, nullptr, { reg });
            irOpInst.contextDepth = static_cast<int>(depth);
            irOpInst.contextSlot = found->second;
            
            emit(irOpInst);
            
            return;
            
        }
                        
    }
    
    scopes.back().symbols[id] = reg;
    
}

/**
 * This maps variable names to their registers
 *  variable | register
 *  -------------
 *    age | %0
 *    name | %1
 */
void IRBuilderVisitor::declare(string name,
                            shared_ptr<IRValue> value,
                            BindingKind kind) {
    
    if (currentFunctionOwnsTopContextFrame) {
        
        // check if the function is the current context.
        // it's only the owner of a variable can declare it.
        // its at the top of the contexts stack, so the depth is 0

        auto context = contexts.back();
                
        auto slot = context.slots.find(name);
        
        if (slot != context.slots.end()) {
            
            // stacontextslot
            // this get the value from the accumulator.
            // stores the value in the context slot
            // StoreContextSlot depth, slot
            IRInstruction staContextSlotOp(IROp::StoreCurrentContextSlot,
                                           nullptr,
                                           { value });
            staContextSlotOp.contextSlot = slot->second;
            // staContextSlotOp.contextDepth = 0;
            
            emit(staContextSlotOp);
            return;

        }
                
    }
    
    if (kind == BindingKind::Let) {
        
        scopes.back().symbols[name] = value;
        
    } else if (kind == BindingKind::Var) {
        
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->type != Scope::Type::Block) {
                it->symbols[name] = value;
                return;
            }
        }
        
    }
    
    if (kind == BindingKind::Const) {
        
        scopes.back().symbols[name] = value;
        
    }
}

R IRBuilderVisitor::visitBlock(BlockStatement* stmt) {
    
    scopes.push_back({ Scope::Type::Block, nullptr, {} });
    
    for (auto& s : stmt->body) {
        s->accept(*this);
    }

    scopes.pop_back();
    
    return true;
}

BindingKind IRBuilderVisitor::getBindingKind(string kind) {
    if (kind == "var" || kind == "VAR") {
        return BindingKind::Var;
    } else if (kind == "let" || kind == "LET")
        return BindingKind::Let;
    
    return BindingKind::Const;
}

R IRBuilderVisitor::visitVariable(VariableStatement* stmt) {
    
    const string kind = stmt->kind;
    const BindingKind bindingKind = getBindingKind(kind);
    
    for (auto& decl : stmt->declarations) {
        const string id = decl.id;
        
        shared_ptr<IRValue> value_reg;
        
        if (decl.init) {
            value_reg = get<shared_ptr<IRValue>>(decl.init->accept(*this));
        } else value_reg = make_shared<IRValue>();
        
        declare(id, value_reg, bindingKind);

    }
    
    return true;
}

R IRBuilderVisitor::visitLiteral(LiteralExpression* expr) {
    return true;
}

/**
 * the number is stored on a register
 * the register is returned
 */
R IRBuilderVisitor::visitNumericLiteral(NumericLiteral* expr) {
    
    shared_ptr<IRValue> dst = createTemp(IRType::Number);

    // auto value = make_shared<IRValue>(std::to_string(expr->value), IRType::Number);
    
    // need to emit Zero if the value is zero
    Value val = toValue(expr->value);
    IROp op;
    
    switch (expr->type) {
        case NumberType::NONE:
            op = IROp::Zero;
            break;
        case NumberType::USHORT:
        case NumberType::UINT:
        case NumberType::ULONG:
        case NumberType::LONG_LONG:
        case NumberType::ULONG_LONG:
        case NumberType::INT:
        case NumberType::SHORT:
        case NumberType::LONG:
        {
            auto value = val.numberValue;
            
            if (value == 0)
                op = IROp::Zero;
            else if (IsSmi(value))
                op = IROp::Constant;
            else
                op = IROp::HeapNumber;
            break;
        }
            
        case NumberType::LONG_DOUBLE:
        case NumberType::FLOAT:
        case NumberType::DOUBLE:
            op = IROp::HeapNumber;
            break;
    }
    
    IRInstruction inst = IRInstruction( op, dst, {  });
    inst.immediate = expr->value;
    
    // destination register holds expr->value
        
    currentBlock->instructions.push_back(inst);
    
    return dst;
}

/**
 * the string is stored on a register
 * the register is returned
 */
R IRBuilderVisitor::visitStringLiteral(StringLiteral* expr) {
    
    auto dst = createTemp(IRType::String);
    
    auto value = make_shared<IRValue>(expr->text, IRType::String);

    auto inst = IRInstruction(IROp::StringConstant, dst, { value });
    inst.immediate = expr->text;
        
    currentBlock->instructions.push_back(inst);
    
    return dst;

}

R IRBuilderVisitor::visitIdentifier(IdentifierExpression* expr) {
    return lookup(expr->token.lexeme);
}

IROp IRBuilderVisitor::getBinaryOp(const Token& op) {
    
    switch (op.type) {

        case TokenType::ADD:                 return IROp::Add;
        case TokenType::MINUS:               return IROp::Subtract;
        case TokenType::MUL:                 return IROp::Multiply;
        case TokenType::DIV:                 return IROp::Divide;
        case TokenType::MODULI:              return IROp::Modulo;
        case TokenType::POWER:               return IROp::Power;
            
        case TokenType::VALUE_EQUAL:         return IROp::Equal;
        case TokenType::REFERENCE_EQUAL:     return IROp::StrictEqual;
        case TokenType::INEQUALITY:          return IROp::NotEqual;
        case TokenType::STRICT_INEQUALITY:   return IROp::StrictNotEqual;
        case TokenType::LESS_THAN:           return IROp::LessThan;
        case TokenType::LESS_THAN_EQUAL:     return IROp::LessThanOrEqual;
        case TokenType::GREATER_THAN:        return IROp::GreaterThan;
        case TokenType::GREATER_THAN_EQUAL:  return IROp::GreaterThanOrEqual;
            
        case TokenType::LOGICAL_AND:         return IROp::LogicalAnd;
        case TokenType::LOGICAL_OR:          return IROp::LogicalOr;
        case TokenType::NULLISH_COALESCING:  return IROp::NullishCoalescing;
            
        case TokenType::BITWISE_AND:         return IROp::BitAnd;
        case TokenType::BITWISE_OR:          return IROp::BitOr;
        case TokenType::BITWISE_XOR:         return IROp::BitXor;
        case TokenType::BITWISE_LEFT_SHIFT:  return IROp::ShiftLeft;
        case TokenType::BITWISE_RIGHT_SHIFT: return IROp::ShiftRight;
        case TokenType::UNSIGNED_RIGHT_SHIFT:return IROp::UnsignedShiftRight;
            
        // case TokenType::INSTANCEOF: return IROp::InstanceOf;
        // case TokenType::IN: return IROp::In;
            
        default:
            throw std::runtime_error("Unknown binary operator in compiler: " + op.lexeme);
    }
    
}

void IRBuilderVisitor::emitAssignment(BinaryExpression* expr) {
    
    // for the lhs, we need to check if its in the context chain
    auto left = expr->left.get();

    shared_ptr<IRValue> value_dst_reg = get<shared_ptr<IRValue>>(expr->right->accept(*this));
    
    if (expr->op.type == TokenType::ASSIGN) {
        
        if (auto* ident = dynamic_cast<IdentifierExpression*>(left)) {
            store(ident->name, value_dst_reg);
        }

    }

}

R IRBuilderVisitor::visitBinary(BinaryExpression* expr) {

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

    shared_ptr<IRValue> result = createTemp(IRType::Any);
    IROp op = getBinaryOp(expr->op);
    
    shared_ptr<IRValue> lhs = get<shared_ptr<IRValue>>(expr->left->accept(*this));
    shared_ptr<IRValue> rhs = get<shared_ptr<IRValue>>(expr->right->accept(*this));

    std::vector<shared_ptr<IRValue>> inputs = {lhs, rhs};
    
    auto inst = IRInstruction(op, result, inputs);
    
    emit(inst);

    return result;
}

R IRBuilderVisitor::visitIf(IfStatement* stmt) {
    
    shared_ptr<IRValue> cond = get<shared_ptr<IRValue>>(stmt->test->accept(*this));

    // if has 'else' and 'then' blocks
    // create 'then' block
    // create 'else' block
    
    // compile into then block
    // compile into else block
    
    BasicBlock* entryBlock = currentBlock;
    
    BasicBlock* thenBlock = createBlock("if.then");
    BasicBlock* elseBlock = stmt->alternate ? createBlock("if.else") : nullptr;
    BasicBlock* mergeBlock = createBlock("merge.block");
    
    // create "if" op
    auto ifInstruction = IRInstruction(IROp::If, nullptr, { cond });
    ifInstruction.targets = { thenBlock, elseBlock ? elseBlock : mergeBlock };
    
    entryBlock->instructions.push_back(ifInstruction);
    entryBlock->terminator = &entryBlock->instructions.back();
    // set the successors: blocks that will succeed the entry block
    entryBlock->successors = { thenBlock, elseBlock ? elseBlock : mergeBlock };
    
    vector<Scope> before = scopes;
    
    currentBlock = thenBlock;
    scopes = before;
    stmt->consequent->accept(*this);

    BasicBlock* thenExit = currentBlock;
    vector<Scope> afterThen = scopes;

    vector<Scope> afterElse = before;
    BasicBlock* elseExit = nullptr;

    if (stmt->alternate) {
        
        scopes = before;
        currentBlock = elseBlock;
        stmt->alternate->accept(*this);
        afterElse = scopes;
        elseExit = currentBlock;
    }

    thenExit->successors.push_back(mergeBlock);
    if (stmt->alternate) elseExit->successors.push_back(mergeBlock);
    
    scopes = before;
    mergeBlock->predecessors = { thenExit, elseExit ? elseExit : entryBlock };
    currentBlock = mergeBlock;
    
    // we need to check to see if 'then' and 'else' blocks
    // refernces any vars in scopes in their scopes.
    // If any, we construct a Phi op.
    for (int i = 0; i < before.size(); i++) {
        
        for (auto symbol : before[i].symbols) {
            
            string name = symbol.first;
            auto value = symbol.second;
            
            // get val name for both then and else
            // *Val is the register number
            // if the register value changes for then and else
            // create phi
            auto thenVal = afterThen[i].symbols[name];
            auto afterVal = afterElse[i].symbols[name];
            
            if (value != thenVal || value != afterVal) {
                
                auto dst = createTemp(value->type);
                auto operands = { thenVal, afterVal };
                auto instruction = IRInstruction(IROp::Phi, dst, operands);
                
                mergeBlock->instructions.push_back(instruction);
                
                scopes[i].symbols[name] = dst;
                
            }

        }
        
    }
    
    return true;
    
}

R IRBuilderVisitor::visitWhile(WhileStatement* stmt) {
        
    BasicBlock* entryBlock = currentBlock;
    auto headerBlock = createBlock("while.header");
    auto bodyBlock = createBlock("while.body");
    auto mergeBlock = createBlock("merge.block");
    
    entryBlock->successors.push_back(headerBlock);
        
    // compile while body
    vector<Scope> before = scopes;
        
    currentBlock = headerBlock;
    
    unordered_map<string, size_t> phiIndexForName;
    
    for (size_t i = 0; i < scopes.size(); i++) {
        for (auto& [name, val] : scopes[i].symbols) {
            auto phiDst = createTemp(val->type);
            IRInstruction phi(IROp::Phi, phiDst, { val });
            phi.label = name;
            headerBlock->instructions.push_back(phi);
            phiIndexForName[name] = headerBlock->instructions.size() - 1;
            scopes[i].symbols[name] = phiDst;
        }
    }
    
    shared_ptr<IRValue> cond = get<shared_ptr<IRValue>>(stmt->test->accept(*this));

    auto instruction = IRInstruction(IROp::If, nullptr, { cond });
    instruction.targets = { bodyBlock, mergeBlock };
    
    headerBlock->successors = { bodyBlock, mergeBlock };
    headerBlock->instructions.push_back(instruction);
    bodyBlock->predecessors.push_back(headerBlock);
    mergeBlock->predecessors.push_back(headerBlock);

    currentBlock = bodyBlock;
    if (stmt->body) stmt->body->accept(*this);
    // add jump inst in body to jmp to header
    
    auto instructionBody = IRInstruction(IROp::If, nullptr, { cond });
    instructionBody.targets = { mergeBlock, bodyBlock };
    
    bodyBlock->instructions.push_back(instructionBody);
    bodyBlock->terminator = &bodyBlock->instructions.back();
    
    mergeBlock->predecessors = { headerBlock };
    
    for (size_t i = 0; i < scopes.size(); i++) {
        for (auto& [name, val] : scopes[i].symbols) {
            auto it = phiIndexForName.find(name);
            if (it != phiIndexForName.end())
                headerBlock->instructions[it->second].operands.push_back(val);
        }
    }
    
    currentBlock = mergeBlock;
    
    return true;
    
}

R IRBuilderVisitor::visitFor(ForStatement* stmt) {
    
    //    for (int i = 0; )
    //    unique_ptr<Statement> init;       // may be null
    //    unique_ptr<Expression> test;      // may be null
    //    unique_ptr<Expression> update;    // may be null
    //    unique_ptr<Statement> body;

    scopes.push_back({ Scope::Type::Block, nullptr, {} });
    if (stmt->init) stmt->init->accept(*this);
    
    shared_ptr<IRValue> cond = get<shared_ptr<IRValue>>(stmt->test->accept(*this));
    
    BasicBlock* entryBlock = currentBlock;
    auto headerBlock = createBlock("for.header");
    auto bodyBlock = createBlock("for.body");
    auto mergeBlock = createBlock("merge.block");
    
    entryBlock->successors.push_back(headerBlock);
    
    auto instruction = IRInstruction(IROp::If, nullptr, { cond });
    instruction.targets = { bodyBlock, mergeBlock };
    
    // compile while body
    vector<Scope> before = scopes;
    
    headerBlock->successors = { bodyBlock, mergeBlock };
    headerBlock->instructions.push_back(instruction);
    bodyBlock->predecessors.push_back(headerBlock);
    mergeBlock->predecessors.push_back(headerBlock);

    currentBlock = headerBlock;
    
    unordered_map<string, size_t> phiIndexForName;

    for (size_t i = 0; i < scopes.size(); i++) {
        for (auto& [name, val] : scopes[i].symbols) {
            auto phiDst = createTemp(val->type);
            IRInstruction phi(IROp::Phi, phiDst, { val });
            phi.label = name;
            headerBlock->instructions.push_back(phi);
            phiIndexForName[name] = headerBlock->instructions.size() - 1;
            scopes[i].symbols[name] = phiDst;
        }
    }

    currentBlock = bodyBlock;
    if (stmt->body) stmt->body->accept(*this);
    if (stmt->update) stmt->update->accept(*this);
    
    // add jump inst in body to jmp to header
    
    auto instructionBody = IRInstruction(IROp::If, nullptr, { cond });
    instructionBody.targets = { mergeBlock, bodyBlock };
    
    bodyBlock->instructions.push_back(instructionBody);
    bodyBlock->terminator = &bodyBlock->instructions.back();
    
    mergeBlock->predecessors = { headerBlock };
    
    for (size_t i = 0; i < scopes.size(); i++) {
        for (auto& [name, val] : scopes[i].symbols) {
            auto it = phiIndexForName.find(name);
            if (it != phiIndexForName.end())
                headerBlock->instructions[it->second].operands.push_back(val);
        }
    }

    currentBlock = mergeBlock;
    scopes.pop_back();

    return true;
    
}

R IRBuilderVisitor::visitReturn(ReturnStatement* stmt) {
    
    std::vector<shared_ptr<IRValue>> operands;
    
    if (stmt->argument) {
        auto dst = get<shared_ptr<IRValue>>(stmt->argument->accept(*this));
        operands.push_back(dst);
    } else {
        
    }
    
    IRInstruction returnInstruction(IROp::Return, nullptr, operands);
    emit(returnInstruction);
    
    return true;
    
}

unordered_set<string> IRBuilderVisitor::collectDeclaredNames(Statement* body) {
    
    unordered_set<string> names;
    
    if (BlockStatement* block = dynamic_cast<BlockStatement*>(body)) {
        
        for (int i = 0; i < block->body.size(); i++) {
            
            auto stmt = block->body[i].get();
            
            if (VariableStatement* var = dynamic_cast<VariableStatement*>(stmt)) {
                //
                for (int j = 0; j < var->declarations.size(); j++) {
                    auto var_decl = var->declarations[j].id;
                    names.insert(var_decl);
                }
                
            }
        }
    }
    
    return names;
    
}

void IRBuilderVisitor::collectFreeVars(Expression* expr,
                                       unordered_set<string>& names) {
    
    if (IdentifierExpression* var = dynamic_cast<IdentifierExpression*>(expr)) {
        
        names.insert(var->name);
        
    }
    
    if (BinaryExpression* block = dynamic_cast<BinaryExpression*>(expr)) {
        collectFreeVars(block->left.get(), names);
        collectFreeVars(block->right.get(), names);
    }

}

void IRBuilderVisitor::walkForFreeVars(Expression* expr,
                                       vector<unordered_set<string>>& boundStack,
                                       unordered_set<string>& freeVars) {
    if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr)) {
        for (auto& s : boundStack) {
            auto it = s.count(ident->name);
            if (!it) {
                freeVars.insert(ident->name);
            }
        }
    }
    
    if (auto* bin = dynamic_cast<BinaryExpression*>(expr)) {
        walkForFreeVars(bin->left.get(), boundStack, freeVars);
        walkForFreeVars(bin->right.get(), boundStack, freeVars);
        return;
    }
    
    
}

void IRBuilderVisitor::walkForFreeVars(Statement* stmt, vector<unordered_set<string>>& boundStack, unordered_set<string>& freeVars) {
    
    if (VariableStatement* var = dynamic_cast<VariableStatement*>(stmt)) {
        
        for (int j = 0; j < var->declarations.size(); j++) {
            
            string var_decl = var->declarations[j].id;
            
            if (var->declarations[j].init) {
                walkForFreeVars(var->declarations[j].init.get(), boundStack, freeVars);
            }
            
            boundStack.back().insert(var_decl);
            
        }
        
    }
    
    if (ReturnStatement* returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
        
        walkForFreeVars(returnStmt->argument.get(), boundStack, freeVars);
        
        return;
        
    }
    
    
}

unordered_set<string> IRBuilderVisitor::freeVariablesOf(const vector<unique_ptr<Statement>>& body, vector<string>& names) {
    
    vector<unordered_set<string>> boundStack;
    boundStack.push_back(unordered_set<string>(names.begin(), names.end()));

    unordered_set<string> freeVars;
    
    for (auto& s : body) {
        walkForFreeVars(s.get(), boundStack, freeVars);
    }
    
    return freeVars;
    
}

void IRBuilderVisitor::collectFreeVars(Statement* stmt, unordered_set<string>& result) {
    
    if (FunctionDeclaration* fnStmt = dynamic_cast<FunctionDeclaration*>(stmt)) {
        
        if (BlockStatement* block = dynamic_cast<BlockStatement*>(fnStmt->body.get())) {
            
            vector<string> names;
            auto v = freeVariablesOf(block->body, names);
            for (auto v : v) {
                result.insert(v);
            }
            
        }
                
        return;
    }
    
}

unordered_set<string> IRBuilderVisitor::collectNestedVariables(Statement* body) {

    unordered_set<string> names;

    if (BlockStatement* block = dynamic_cast<BlockStatement*>(body)) {
        
        for (int i = 0; i < block->body.size(); i++) {
            
            auto stmt = block->body[i].get();
            
            collectFreeVars(stmt, names);
            
        }
        
    }
    
    return names;
    
}

R IRBuilderVisitor::visitFunction(FunctionDeclaration* stmt) {
    
    unique_ptr<IRFunction> function = make_unique<IRFunction>();
    IRFunction* childFnPtr = function.get();

    function->name = stmt->id;
    
    IRFunction* savedFunction = currentFunction;
    BasicBlock* savedBlock = currentBlock;
    vector<Scope> savedScopes = std::move(scopes);
    bool savedOwnsTopFrame = currentFunctionOwnsTopContextFrame;

    currentFunction = function.get();
    BasicBlock* functionBlock = createBlock(stmt->id);
    currentBlock = functionBlock;

    scopes.clear();
    scopes.push_back({ Scope::Type::Function, nullptr, {} });

    // compute vars captured by nesting functions
    // first, get all vars here
    unordered_set<string> vars = collectDeclaredNames(stmt->body.get());
    
    unordered_set<string> nested_vars = collectNestedVariables(stmt->body.get());
    
    // check if vars is in nested_vars
    unordered_set<string> captured_vars;
    for (string nested_var : nested_vars) {
        if (vars.count(nested_var)) {
            captured_vars.insert(nested_var);
        }
    }
    
    // if captured is not empty, emit CreateContext
    bool pushedContextFrame = false;

    if (captured_vars.size() > 0) {
        
        auto dst = createTemp(IRType::Any);
        // sets up new context
        // this should be called at the very top of a function body
        // sets ups the slots in the context
        // sets the frame->owncontext to the context
        IRInstruction createContextOp(IROp::CreateContext, dst, {});
        createContextOp.contextSlot = static_cast<int>(captured_vars.size());
        emit(createContextOp);
        
        Context funcCtx;
        funcCtx.contextValue = dst;

        for (int i = 0; i < captured_vars.size(); i++) {
            auto it = next(captured_vars.begin(), i);
            string v = *it;
            funcCtx.slots[v] = i;
        }
        
        contexts.push_back(funcCtx);
        pushedContextFrame = true;
        currentFunctionOwnsTopContextFrame = true;
        
    } else {
        currentFunctionOwnsTopContextFrame = false;
    }
    
    if (BlockStatement* body = dynamic_cast<BlockStatement*>(stmt->body.get())) {
        
        for (auto& s : body->body) {
            
            s->accept(*this);

        }
    }
    
    if (pushedContextFrame) contexts.pop_back();
    
    scopes.pop_back();
    irModule.functions.emplace_back(std::move(function));

    currentBlock = savedBlock;
    currentFunction = savedFunction;
    scopes = std::move(savedScopes);
    currentFunctionOwnsTopContextFrame = savedOwnsTopFrame;

    auto fnValue = createTemp(IRType::Function);
    IRInstruction closureIr (IROp::Closure, fnValue, {} /*contexts.empty() ? vector<shared_ptr<IRValue>>{} : vector<shared_ptr<IRValue>>{ contexts.back().contextValue }*/);
    
    // we will need to attach the current context, it is the current context executing.
    // a function picks the current context it is in.
    // this is the function's parent context or closure
    if (!contexts.empty()) {
        closureIr.operands.push_back(contexts[contexts.size() - 1].contextValue);
    }
    
    closureIr.childFunction = childFnPtr;
    emit(closureIr);
    
    declare(stmt->id, fnValue, BindingKind::Var);
    
    return true;
    
}

R IRBuilderVisitor::visitCall(CallExpression* expr) {
    
    if (auto ident = dynamic_cast<IdentifierExpression*>(expr->callee.get())) {
        if (ident->name == "print") {
            
            vector<shared_ptr<IRValue>> operands;
            for (auto& arg : expr->arguments) {
                operands.push_back(get<shared_ptr<IRValue>>(arg->accept(*this)));
            }

            IRInstruction call(IROp::Print, {}, operands);
            
            emit(call);
            
            return true;

        }
    }
    
    shared_ptr<IRValue> callee = get<shared_ptr<IRValue>>(expr->callee->accept(*this));

    auto result = createTemp(IRType::Any);

    vector<shared_ptr<IRValue>> operands;
    operands.push_back(callee);
    for (auto& arg : expr->arguments) {
        operands.push_back(get<shared_ptr<IRValue>>(arg->accept(*this)));
    }

    IRInstruction call(IROp::Call, result, operands);
    
    emit(call);
    
    return result;
    
}

R IRBuilderVisitor::visitMember(MemberExpression* expr) { return true; }
R IRBuilderVisitor::visitNew(NewExpression* expr) { return true; }
R IRBuilderVisitor::visitArray(ArrayLiteralExpression* expr) { return true; }
R IRBuilderVisitor::visitObject(ObjectLiteralExpression* expr) { return true; }
R IRBuilderVisitor::visitConditional(ConditionalExpression* expr) { return true; }
R IRBuilderVisitor::visitUnary(UnaryExpression* expr) { return true; }
R IRBuilderVisitor::visitArrowFunction(ArrowFunction* expr) { return true; }
R IRBuilderVisitor::visitFunctionExpression(FunctionExpression* expr) { return true; }
R IRBuilderVisitor::visitTemplateLiteral(TemplateLiteral* expr) { return true; }
R IRBuilderVisitor::visitImportDeclaration(ImportDeclaration* stmt) { return true; }

R IRBuilderVisitor::visitAssignment(AssignmentExpression* expr) { return true; }
R IRBuilderVisitor::visitLogical(LogicalExpression* expr) { return true; }
R IRBuilderVisitor::visitThis(ThisExpression* expr) { return true; }
R IRBuilderVisitor::visitSuper(SuperExpression* expr) { return true; }
R IRBuilderVisitor::visitProperty(PropertyExpression* expr) { return true; }
R IRBuilderVisitor::visitSequence(SequenceExpression* expr) { return true; }
R IRBuilderVisitor::visitUpdate(UpdateExpression* expr) { return true; }

R IRBuilderVisitor::visitFalseKeyword(FalseKeyword* expr) {
    
    auto destination = createTemp(IRType::Bool);
    
    auto value = make_shared<IRValue>("0", IRType::Number);

    auto instruction = IRInstruction(IROp::False, destination, { value });
    
    emit(instruction);
    
    return destination;
    
}

R IRBuilderVisitor::visitTrueKeyword(TrueKeyword* expr) {
    
    // dest register = true;
    
    auto destination = createTemp(IRType::Bool);
    
    auto value = make_shared<IRValue>("1", IRType::Number);
    
    auto instruction = IRInstruction(IROp::True, destination, { value });
    
    emit(instruction);
    
    return destination;
    
}

R IRBuilderVisitor::visitPublicKeyword(PublicKeyword* expr) { return true; }
R IRBuilderVisitor::visitPrivateKeyword(PrivateKeyword* expr) { return true; }
R IRBuilderVisitor::visitProtectedKeyword(ProtectedKeyword* expr) { return true; }
R IRBuilderVisitor::visitStaticKeyword(StaticKeyword* expr) { return true; }
R IRBuilderVisitor::visitRestParameter(RestParameter* expr) { return true; }
R IRBuilderVisitor::visitClassExpression(ClassExpression* expr) { return true; }

R IRBuilderVisitor::visitNullKeyword(NullKeyword* expr) {
    // dest reg = null
    auto dst = createTemp(IRType::Null);
    
    IRInstruction ir(IROp::Null, dst, {});
    
    emit(ir);
    
    return dst;
    
}

R IRBuilderVisitor::visitUndefinedKeyword(UndefinedKeyword* expr) {
    
    // dest reg = undefined
    auto dst = createTemp(IRType::Undefined);

    IRInstruction ir(IROp::Undefined, dst, {});
    
    emit(ir);

    return dst;
    
}

R IRBuilderVisitor::visitAwaitExpression(AwaitExpression* expr) { return true; }
R IRBuilderVisitor::visitUIExpression(UIViewExpression* visitor) { return true; }
R IRBuilderVisitor::visitEnumDeclaration(EnumDeclaration* stmt) { return true; }
R IRBuilderVisitor::visitInterfaceDeclaration(InterfaceDeclaration* stmt) { return true; }

R IRBuilderVisitor::visitBreak(BreakStatement* stmt) { return true; }
R IRBuilderVisitor::visitContinue(ContinueStatement* stmt) { return true; }
R IRBuilderVisitor::visitThrow(ThrowStatement* stmt) { return true; }
R IRBuilderVisitor::visitEmpty(EmptyStatement* stmt) { return true; }
R IRBuilderVisitor::visitClass(ClassDeclaration* stmt) { return true; }
R IRBuilderVisitor::visitMethodDefinition(MethodDefinition* stmt) { return true; }

R IRBuilderVisitor::visitDoWhile(DoWhileStatement* stmt) { return true; }

R IRBuilderVisitor::visitSwitchCase(SwitchCase* stmt) { return true; }
R IRBuilderVisitor::visitSwitch(SwitchStatement* stmt) { return true; }
R IRBuilderVisitor::visitCatch(CatchClause* stmt) { return true; }
R IRBuilderVisitor::visitTry(TryStatement* stmt) { return true; }

R IRBuilderVisitor::visitForIn(ForInStatement* stmt) {
    return true;
}

R IRBuilderVisitor::visitForOf(ForOfStatement* stmt) {
    return true;
}

R IRBuilderVisitor::visitYieldExpression(YieldExpression* visitor) { return true; }
R IRBuilderVisitor::visitSpreadExpression(SpreadExpression* visitor) { return true; }
R IRBuilderVisitor::visitComma(CommaExpression *expr) { return true; }

void IRBuilderVisitor::emit(const IRInstruction inst) {
    currentBlock->instructions.push_back(inst);
}
