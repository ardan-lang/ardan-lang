//
//  IRBuilderVisitor.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/05/2026.
//

#include "IRBuilderVisitor.hpp"

void IRBuilderVisitor::build(const vector<unique_ptr<Statement>> &program) {
    
    currentBlock = new BasicBlock("entry");
    currentFunction = new IRFunction();
    
    for (const auto &s : program) {
        s->accept(*this);
    }

}

R IRBuilderVisitor::visitExpression(ExpressionStatement* stmt) {
    
    stmt->accept(*this);
    
    return true;
}

R IRBuilderVisitor::visitBlock(BlockStatement* stmt) { return true; }

void IRBuilderVisitor::create() {}
void IRBuilderVisitor::store() {}
void IRBuilderVisitor::load() {}

R IRBuilderVisitor::visitVariable(VariableStatement* stmt) {
    
    // var a = ...
    // let b = ...
    // const c = ...
    const string kind = stmt->kind;
    
    for (auto& decl : stmt->declarations) {
        const string id = decl.id;
        
        shared_ptr<IRValue> value;
        
        if (decl.init) {
            value = get<shared_ptr<IRValue>>(decl.init->accept(*this));
        } else value = make_shared<IRValue>();
        
        // symTable[id] = std::move(value);
        bind(id, *(value));
        emit(IRInstruction(IROp::Store, createTemp(IRType::Any), { *value }));
        
    }
    
    return true;
}

R IRBuilderVisitor::visitLiteral(LiteralExpression* expr) {
    return true;
}

R IRBuilderVisitor::visitNumericLiteral(NumericLiteral* expr) {
    
    auto ir = createTemp(IRType::Number);
    
    auto inst = IRInstruction(IROp::Constant, ir, {});
    inst.immediate = get<double>(expr->value);
        
    currentBlock->instructions.push_back(inst);
    
    return ir;
}

R IRBuilderVisitor::visitStringLiteral(StringLiteral* expr) {
    
    auto ir = createTemp(IRType::String);
    
    auto inst = IRInstruction(IROp::Constant, ir, {});
    inst.immediate = expr->text;
        
    currentBlock->instructions.push_back(inst);
    
    return ir;

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

    auto result = createTemp(IRType::Any);
    IROp op = getBinaryOp(expr->op);

    
    shared_ptr<IRValue> lhs = get<shared_ptr<IRValue>>(expr->left->accept(*this));
    shared_ptr<IRValue> rhs = get<shared_ptr<IRValue>>(expr->right->accept(*this));

    std::vector<IRValue> inputs = {*lhs, *rhs};
    
    auto inst = IRInstruction(op, result, inputs);
    
    emit(inst);

    return true;
}

R IRBuilderVisitor::visitIf(IfStatement* stmt) { return true; }
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
R IRBuilderVisitor::visitFalseKeyword(FalseKeyword* expr) { return true; }
R IRBuilderVisitor::visitTrueKeyword(TrueKeyword* expr) { return true; }
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

IRValue IRBuilderVisitor::lookup(string name) {

    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {

        auto found = it->symbols.find(name);

        if (found != it->symbols.end())
            return found->second;
    }

    throw runtime_error("Undefined variable");
}

void IRBuilderVisitor::bind(string name, IRValue value) {
    scopes.back().symbols[name] = value;
}
