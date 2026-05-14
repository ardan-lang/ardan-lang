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

R IRBuilderVisitor::visitVariable(VariableStatement* stmt) {
    
    // var a = ...
    // let b = ...
    // const c = ...
    const string kind = stmt->kind;
    
    for (auto& decl : stmt->declarations) {
        const string id = decl.id;
        
        IRValue value;
        
        if (decl.init) {
            value = get<IRValue>(decl.init->accept(*this));
        }
        
        symTable[id] = value;
        
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
    return symTable[expr->token.lexeme];
}

R IRBuilderVisitor::visitBinary(BinaryExpression* expr) { return true; }

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
