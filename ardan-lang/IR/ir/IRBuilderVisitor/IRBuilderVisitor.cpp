//
//  IRBuilderVisitor.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/05/2026.
//

#include "IRBuilderVisitor.hpp"

void IRBuilderVisitor::build(const vector<unique_ptr<Statement>> &program) {

    unique_ptr<IRFunction> uniqueCurrentFunction = make_unique<IRFunction>();
    currentFunction = uniqueCurrentFunction.get();

    currentBlock = createBlock("entry");

    scopes.push_back({ Scope::Type::Global, nullptr, {} });
    for (const auto &s : program) {
        s->accept(*this);
    }
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

    throw runtime_error("Undefined variable");
}

/**
 * This maps variable names to their registers
 *  variable | register
 *  -------------
 *    age | %0
 *    name | %1
 */
void IRBuilderVisitor::bind(string name,
                            shared_ptr<IRValue> value,
                            BindingKind kind) {
    
    if (kind == BindingKind::Let) {
        
        scopes.back().symbols[name] = value;
        
    } else if (kind == BindingKind::Var) {
        
        for (int i = 0; i < scopes.size(); i++) {
            if (scopes[i].type != Scope::Type::Block) {
                scopes[i].symbols[name] = value;
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
    if (kind == "var") {
        return BindingKind::Var;
    } else if (kind == "let")
        return BindingKind::Let;
    
    return BindingKind::Const;
}

R IRBuilderVisitor::visitVariable(VariableStatement* stmt) {
    
    const string kind = stmt->kind;
    const BindingKind bindingKind = getBindingKind(kind);
    
    for (auto& decl : stmt->declarations) {
        const string id = decl.id;
        
        shared_ptr<IRValue> reg;
        
        if (decl.init) {
            reg = get<shared_ptr<IRValue>>(decl.init->accept(*this));
        } else reg = make_shared<IRValue>();
        
        bind(id, reg, bindingKind);

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

    IRInstruction inst = IRInstruction(IROp::Constant, dst, {  });
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

    auto inst = IRInstruction(IROp::Constant, dst, { value });
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
            // emitAssignment(expr);
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

    vector<Scope> afterThen = scopes;

    vector<Scope> afterElse;

    if (stmt->alternate) {
        
        scopes = before;
        currentBlock = elseBlock;
        stmt->alternate->accept(*this);
        afterElse = scopes;
        
    }

    scopes = before;
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

R IRBuilderVisitor::visitWhile(WhileStatement* stmt) { return true; }
R IRBuilderVisitor::visitFor(ForStatement* stmt) { return true; }
R IRBuilderVisitor::visitReturn(ReturnStatement* stmt) { return true; }
R IRBuilderVisitor::visitFunction(FunctionDeclaration* stmt) { return true; }
R IRBuilderVisitor::visitCall(CallExpression* expr) { return true; }
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

    auto instruction = IRInstruction(IROp::Constant, destination, { value });
    
    emit(instruction);
    
    return destination;
    
}

R IRBuilderVisitor::visitTrueKeyword(TrueKeyword* expr) {
    
    auto destination = createTemp(IRType::Bool);
    
    auto value = make_shared<IRValue>("1", IRType::Number);
    
    auto instruction = IRInstruction(IROp::Constant, destination, { value });
    
    emit(instruction);
    
    return destination;
    
}

R IRBuilderVisitor::visitPublicKeyword(PublicKeyword* expr) { return true; }
R IRBuilderVisitor::visitPrivateKeyword(PrivateKeyword* expr) { return true; }
R IRBuilderVisitor::visitProtectedKeyword(ProtectedKeyword* expr) { return true; }
R IRBuilderVisitor::visitStaticKeyword(StaticKeyword* expr) { return true; }
R IRBuilderVisitor::visitRestParameter(RestParameter* expr) { return true; }
R IRBuilderVisitor::visitClassExpression(ClassExpression* expr) { return true; }
R IRBuilderVisitor::visitNullKeyword(NullKeyword* expr) { return true; }
R IRBuilderVisitor::visitUndefinedKeyword(UndefinedKeyword* expr) { return true; }
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
R IRBuilderVisitor::visitForIn(ForInStatement* stmt) { return true; }
R IRBuilderVisitor::visitForOf(ForOfStatement* stmt) { return true; }
R IRBuilderVisitor::visitYieldExpression(YieldExpression* visitor) { return true; }
R IRBuilderVisitor::visitSpreadExpression(SpreadExpression* visitor) { return true; }
R IRBuilderVisitor::visitComma(CommaExpression *expr) { return true; }

void IRBuilderVisitor::emit(const IRInstruction inst) {
    currentBlock->instructions.push_back(inst);
}
