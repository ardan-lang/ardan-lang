//
//  AstPrinter.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 20/08/2025.
//

#include <stdio.h>
#include "AstPrinter.h"

// helper
void AstPrinter::printIndent() {
    for (int i = 0; i < indent; i++) std::cout << "  ";
}

// -------- Statements --------
R AstPrinter::visitExpression(ExpressionStatement* stmt) {
    printIndent(); std::cout << "ExpressionStatement:\n";
    indent++;
    stmt->expression->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitBlock(BlockStatement* stmt) {
    printIndent(); std::cout << "Block:\n";
    indent++;
    for (auto& s : stmt->body) {
        s->accept(*this);
    }
    indent--;
    
    return true;
}


R AstPrinter::visitVariable(VariableStatement* stmt) {
    printIndent(); std::cout << "VariableDeclaration " << stmt->kind;
    std::cout << " =\n";
    indent++;
    for (auto& declarator : stmt->declarations) {
        std::cout << declarator.id << "\n";
        if (declarator.init) {
            declarator.init->accept(*this);
        }
    }
    indent--;
    return true;
    
}

R AstPrinter::visitFunction(FunctionDeclaration* stmt) {
    printIndent(); std::cout << (stmt->is_async ? "Async" : "") << "Function " << stmt->id << "(";
    for (size_t i = 0; i < stmt->params.size(); i++) {
        stmt->params[i]->accept(*this); std::cout << ", ";
    }
    std::cout << ")\n";
    indent++;
    stmt->body->accept(*this);
    indent--;
    return true;
    
}

R AstPrinter::visitIf(IfStatement* stmt) {
    printIndent(); std::cout << "If\n";
    indent++;
    stmt->test->accept(*this);
    printIndent(); std::cout << "Then:\n";
    stmt->consequent->accept(*this);
    if (stmt->alternate) {
        printIndent(); std::cout << "Else:\n";
        stmt->alternate->accept(*this);
    }
    indent--;
    return true;
    
}

R AstPrinter::visitWhile(WhileStatement* stmt) {
    printIndent(); std::cout << "While\n";
    indent++;
    stmt->test->accept(*this);
    stmt->body->accept(*this);
    indent--;
    return true;
}

R AstPrinter::visitFor(ForStatement* stmt) {
    printIndent(); std::cout << "For\n";
    indent++;
    if (stmt->init) stmt->init->accept(*this);
    if (stmt->test) stmt->test->accept(*this);
    if (stmt->update) stmt->update->accept(*this);
    stmt->body->accept(*this);
    indent--;
    return true;
}

R AstPrinter::visitReturn(ReturnStatement* stmt) {
    printIndent(); std::cout << "Return\n";
    if (stmt->argument) {
        indent++;
        stmt->argument->accept(*this);
        indent--;
    }
    return true;
}

R AstPrinter::visitBreak(BreakStatement* stmt) {
    printIndent(); std::cout << "Break\n";
    return true;
}

R AstPrinter::visitContinue(ContinueStatement* stmt) {
    printIndent(); std::cout << "Continue\n";
    return true;
}

R AstPrinter::visitThrow(ThrowStatement* stmt) {
    printIndent(); std::cout << "Throw\n";
    indent++;
    stmt->argument->accept(*this);
    indent--;
    return true;
}

R AstPrinter::visitEmpty(EmptyStatement* stmt) {
    printIndent(); std::cout << "Empty\n";
    return true;
}

R AstPrinter::visitClass(ClassDeclaration* stmt) {
    printIndent(); std::cout << "Class " << stmt->id << " {\n";
    indent++;
    for (auto& field : stmt->fields) {
        for (auto& modifier : field->modifiers) {
            modifier->accept(*this);
        }
        field->property->accept(*this);
    }
    for (auto& method : stmt->body) {
        method->accept(*this);
    }
    indent--;
    printIndent(); std::cout << "}\n";
    return true;
}

R AstPrinter::visitMethodDefinition(MethodDefinition* stmt) {
    for (auto& modifier : stmt->modifiers) {
        modifier->accept(*this);
    }
    printIndent(); std::cout << "Method " << stmt->name << "(";
    for (size_t i = 0; i < stmt->params.size(); i++) {
        stmt->params[i]->accept(*this); std::cout << ", ";
    }
    std::cout << ") ";
    stmt->methodBody->accept(*this);
    return true;
}

R AstPrinter::visitDoWhile(DoWhileStatement* stmt) {
    cout << "DoWhileStatement {\n";
    cout << "  body: ";
    if (stmt->body) stmt->body->accept(*this);
    cout << "\n  condition: ";
    if (stmt->condition) stmt->condition->accept(*this);
    cout << "\n}\n";
    return true;
}

R AstPrinter::visitSwitchCase(SwitchCase* stmt) {
    cout << "SwitchCase {\n";
    cout << "  test: ";
    if (stmt->test) stmt->test->accept(*this);
    else cout << "default";
    cout << "\n  consequent: [\n";
    for (auto& cons : stmt->consequent) {
        if (cons) {
            cout << "    ";
            cons->accept(*this);
            cout << "\n";
        }
    }
    cout << "  ]\n";
    cout << "}\n";
    return true;
}

R AstPrinter::visitSwitch(SwitchStatement* stmt) {
    cout << "SwitchStatement {\n";
    cout << "  discriminant: ";
    if (stmt->discriminant) stmt->discriminant->accept(*this);
    cout << "\n  cases: [\n";
    for (auto& scase : stmt->cases) {
        if (scase) {
            cout << "    ";
            scase->accept(*this);
        }
    }
    cout << "  ]\n";
    cout << "}\n";
    return true;
}

R AstPrinter::visitCatch(CatchClause* stmt) {
    printIndent();
    cout << "Catch (" << stmt->param << ") ";
    if (stmt->body) {
        cout << "{\n";
        indent++;
        stmt->body->accept(*this);
        indent--;
        printIndent();
        cout << "}\n";
    } else {
        cout << "{}\n";
    }
    
    return true;
    
}

R AstPrinter::visitTry(TryStatement* stmt) {
    printIndent(); std::cout << "Try\n";
    if (stmt->block) {
        cout << "{\n";
        indent++;
        stmt->block->accept(*this);
        indent--;
        printIndent();
        cout << "}";
    }
    cout << "\n";
    if (stmt->handler) stmt->handler->accept(*this);
    if (stmt->finalizer) {
        printIndent();
        cout << "Finally ";
        cout << "{\n";
        indent++;
        stmt->finalizer->accept(*this);
        indent--;
        printIndent();
        cout << "}\n";
    }
    
    return true;
    
}

R AstPrinter::visitForIn(ForInStatement* stmt) {
    printIndent(); std::cout << "ForIn\n";
    indent++;
    if (stmt->init) stmt->init->accept(*this);
    if (stmt->object) stmt->object->accept(*this);
    stmt->body->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitForOf(ForOfStatement* stmt) {
    printIndent(); std::cout << "ForOf\n";
    indent++;
    if (stmt->left) stmt->left->accept(*this);
    if (stmt->right) stmt->right->accept(*this);
    stmt->body->accept(*this);
    indent--;
    
    return true;
    
}

// -------- Expressions --------
R AstPrinter::visitLiteral(LiteralExpression* expr) {
    printIndent(); std::cout << "Literal(" << expr->token.lexeme << ")\n";
    
    return true;
    
}

R AstPrinter::visitIdentifier(IdentifierExpression* expr) {
    printIndent(); std::cout << "Identifier(" << /*expr->name <<*/ expr->token.lexeme << ")\n";
    
    return true;
    
}

R AstPrinter::visitBinary(BinaryExpression* expr) {
    printIndent(); std::cout << "Binary(" << expr->op.lexeme << ")\n";
    indent++;
    expr->left->accept(*this);
    expr->right->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitUnary(UnaryExpression* expr) {
    printIndent(); std::cout << "Unary(" << expr->op.lexeme << ")\n";
    indent++;
    expr->right->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitUpdate(UpdateExpression* expr) {
    printIndent();
    std::cout << "Update(" << expr->op.lexeme
              << (expr->prefix ? " prefix" : " postfix") << ")\n";
    indent++;
    expr->argument->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitAssignment(AssignmentExpression* expr) {
    printIndent(); std::cout << "Assignment(" << expr->op.lexeme << ")\n";
    indent++;
    expr->left->accept(*this);
    expr->right->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitLogical(LogicalExpression* expr) {
    printIndent(); std::cout << "Logical(" << expr->op.lexeme << ")\n";
    indent++;
    expr->left->accept(*this);
    expr->right->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitConditional(ConditionalExpression* expr) {
    printIndent(); std::cout << "Conditional\n";
    indent++;
    expr->test->accept(*this);
    expr->consequent->accept(*this);
    expr->alternate->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitCall(CallExpression* expr) {
    printIndent(); std::cout << "Call\n";
    indent++;
    expr->callee->accept(*this);
    for (auto& arg : expr->arguments) {
        arg->accept(*this);
    }
    indent--;
    
    return true;
    
}

R AstPrinter::visitMember(MemberExpression* expr) {
    printIndent(); std::cout << "Member(" << (expr->computed ? "computed" : "dot") << ")\n";
    indent++;
    expr->object->accept(*this);
    if (expr->computed) {
        expr->property->accept(*this);
    } else {
        printIndent(); cout << expr->name.lexeme << "\n";
    }
    indent--;
    
    return true;
    
}

R AstPrinter::visitThis(ThisExpression* expr) {
    printIndent(); std::cout << "This\n";
    
    return true;
    
}

R AstPrinter::visitNew(NewExpression* expr) {
    printIndent(); std::cout << "New\n";
    indent++;
    expr->callee->accept(*this);
    for (auto& arg : expr->arguments) arg->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitArray(ArrayLiteralExpression* expr) {
    printIndent(); std::cout << "Array\n";
    indent++;
    for (auto& e : expr->elements) e->accept(*this);
    indent--;
    
    return true;
    
}

R AstPrinter::visitObject(ObjectLiteralExpression* expr) {
    printIndent(); std::cout << "Object\n";
    indent++;
    for (auto& prop : expr->props) {
        printIndent(); std::cout << "Property " << prop.first.lexeme << ":\n";
        indent++;
        prop.second->accept(*this);
        indent--;
    }
    indent--;
    
    return true;
    
}

R AstPrinter::visitSuper(SuperExpression* expr) {
    printIndent(); std::cout << "Super:\n";
    
    return true;
    
}

R AstPrinter::visitProperty(PropertyExpression* expr) {
    printIndent(); std::cout << "Property:\n";
    
    return true;
    
}

R AstPrinter::visitSequence(SequenceExpression* expr) {
    printIndent(); std::cout << "Sequence:\n";
    indent++;
    for (auto& e : expr->expressions) {
        e->accept(*this);
    }
    indent--;
    
    return true;
    
}

R AstPrinter::visitFalseKeyword(FalseKeyword* expr) {
    printIndent(); std::cout << "False\n";
    
    return true;
    
}

R AstPrinter::visitTrueKeyword(TrueKeyword* expr) {
    printIndent(); std::cout << "True\n";
    
    return true;
    
}

R AstPrinter::visitNumericLiteral(NumericLiteral* expr) {
    printIndent(); std::cout << "Numeric ";
    indent++;
    
    auto value = expr->value;
    
    if (std::holds_alternative<std::monostate>(value)) {
        std::cout << "nil";
    } else if (std::holds_alternative<double>(value)) {
        std::cout << std::get<double>(value);
    } else if (std::holds_alternative<unsigned long>(value)) {
        std::cout << std::get<unsigned long>(value);
    } else if (std::holds_alternative<int>(value)) {
        std::cout << std::get<int>(value);
    } else if (std::holds_alternative<size_t>(value)) {
        std::cout << std::get<size_t>(value);
    } else if (std::holds_alternative<std::string>(value)) {
        std::cout << std::get<std::string>(value);
    } else if (std::holds_alternative<bool>(value)) {
        std::cout << (std::get<bool>(value) ? "true" : "false");
    } else if (std::holds_alternative<shared_ptr<Value>>(value)) {
        std::cout << (std::get<shared_ptr<Value>>(value))->toString();
    } else if (std::holds_alternative<Value>(value)) {
        std::cout << (std::get<Value>(value)).toString();
    }
    
    cout << "\n";
    
    indent--;
    
    return true;
    
}

R AstPrinter::visitStringLiteral(StringLiteral* expr) {
    printIndent(); std::cout << "String ";
    indent++;
    cout << expr->text << "\n";
    indent--;
    
    return true;
    
}

R AstPrinter::visitPublicKeyword(PublicKeyword* expr) {
    printIndent(); std::cout << "public";
    
    return true;
    
}

R AstPrinter::visitPrivateKeyword(PrivateKeyword* expr) {
    printIndent(); std::cout << "private";
    
    return true;
    
}

R AstPrinter::visitProtectedKeyword(ProtectedKeyword* expr) {
    printIndent(); std::cout << "protected";
    
    return true;
    
}

R AstPrinter::visitStaticKeyword(StaticKeyword* expr) {
    printIndent(); std::cout << "static";
    
    return true;
    
}

R AstPrinter::visitArrowFunction(ArrowFunction *expr) {
    printIndent(); std::cout << (expr->is_async ? "Async" : "") << "(";
    
    indent++;
    cout << expr->token.lexeme;
    indent--;
    
    if (expr->parameters) {
        expr->parameters->accept(*this);
    }
    
    printIndent(); std::cout << ") => ";
    
    if (expr->exprBody != nullptr) {
        expr->exprBody->accept(*this);
    }
    
    if (expr->stmtBody != nullptr) {
        expr->stmtBody->accept(*this);
    }
    
    return true;

}

R AstPrinter::visitTemplateLiteral(TemplateLiteral* expr) {
    printIndent(); std::cout << "TemplateString: ";
//    vector<unique_ptr<StringLiteral>> quasis;
//    vector<unique_ptr<Expression>> expressions;

//    for (auto& part : expr->parts) {
//        part->accept(*this);
//    }
    
    std::string result;

    size_t qsize = expr->quasis.size();
    size_t esize = expr->expressions.size();

    // Append quasis and interleave expressions
    for (size_t i = 0; i < qsize; i++) {
        // add the quasi
        expr->quasis[i]->accept(*this);

        // if there’s a matching expression, evaluate it
        if (i < esize) {
            expr->expressions[i]->accept(*this);
        }
    }
    
    return true;
    
}

R AstPrinter::visitRestParameter(RestParameter *expr) {
    printIndent();
    std::cout << "Rest \"" << expr->token.lexeme << "\"\n";
    return monostate();
}

R AstPrinter::visitImportDeclaration(ImportDeclaration* stmt) {
    printIndent();
    std::cout << "Import \"" << stmt->path.lexeme << "\"\n";
    return true;
}

R AstPrinter::visitFunctionExpression(FunctionExpression* expr) {
    printIndent(); std::cout << (expr->is_async ? "Async" : "") << "(";
    return true;
}

R AstPrinter::visitClassExpression(ClassExpression* visitor) {
    return true;
}

R AstPrinter::visitNullKeyword(NullKeyword* visitor) {
    printIndent();
    std::cout << "Null \"" << visitor->token.lexeme << "\"\n";
    return true;
}

R AstPrinter::visitUndefinedKeyword(UndefinedKeyword* visitor) {
    printIndent();
    std::cout << "Undefined \"" << visitor->token.lexeme << "\"\n";
    return true;
}

R AstPrinter::visitAwaitExpression(AwaitExpression* expr) {
    printIndent(); cout << "Await";
    expr->inner->accept(*this);
    return true;
}
