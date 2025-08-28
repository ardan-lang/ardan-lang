//
//  Interpreter.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 24/08/2025.
//

#include <memory>

#include "Interpreter.h"
#include "../Statements/Statements.hpp"
#include "../Expression/Expression.hpp"
#include "../Visitor/AstPrinter/AstPrinter.h"
#include "Utils/Utils.h"

Interpreter::Interpreter() {
    env = new Env();
}

Interpreter::~Interpreter() {
    delete env;
}

void Interpreter::execute(vector<unique_ptr<Statement>> ast) {
    for (unique_ptr<Statement>& stmt : ast) {
        stmt->accept(*this);
    }
}

void Interpreter::executeBlock(unique_ptr<Statement> block) {
    block->accept(*this);
}

R Interpreter::visitExpression(ExpressionStatement* stmt) {
    return stmt->expression->accept(*this);
}

R Interpreter::visitBlock(BlockStatement* stmt) {
    
    Env* previous = env;           // save current scope
    env = new Env(previous);       // allocate child env on heap
    
    for (auto& item : previous->getStack()) {
        env->setValue(item.first, item.second);
    }

    for (auto& s : stmt->body) {
        s->accept(*this);
    }
    
    previous->clearStack();
    delete env;                    // cleanup after block
    env = previous;                // restore old scope
    
    return true;
}

R Interpreter::visitVariable(VariableStatement* stmt) {
    
    const string kind = stmt->kind;
    
    if ((stmt->declarations).size() > 0) {
        for (auto& declarator : stmt->declarations) {
            
            if (kind == "const" && declarator.init == nullptr) {
                throw runtime_error("Missing initializer in const declaration: " + declarator.id);
            }
            
            if (declarator.init) {
                
                R value = declarator.init->accept(*this);
                
                env->setValue(declarator.id, value);
            }
        }
    }
    
    return true;
    
}


R Interpreter::visitCall(CallExpression* expr) {
    
    vector<R> vectorArg;
    
    auto callee = expr->callee->accept(*this);
    
    for (auto& arg : expr->arguments) {
        auto _arg = arg->accept(*this);
        vectorArg.push_back(_arg);
    }

    // If the callee is "print"
    if (holds_alternative<std::string>(callee) &&
        get<std::string>(callee) == "print")
    {
        for (auto& v : vectorArg) {
            printValue(v);
            std::cout << " ";
        }
        cout << std::endl;
        return std::monostate{};
    }
    
    // search call name in function declarations.

    vector<Expression*> args = env->getFunctionParams(get<std::string>(callee));
    Statement* body = env->getFunctionBody(get<std::string>(callee));
    
    for (Expression* arg : args) {
        
        string key;
        
        if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(arg)) {
            key = ident->name;
        }
        
        env->setStackValue(key, arg->accept(*this));
        
    }

    // AstPrinter printer;

    if (body != nullptr) {
        
        try {
            
            body->accept(*this);
            
        } catch (ReturnException& r) {
            
            return r.value;
            
        }
        
        return monostate();
    }
    
    throw runtime_error("Unknown function call");
    
}

R Interpreter::visitFalseKeyword(FalseKeyword* expr) {
    return false;
}

R Interpreter::visitTrueKeyword(TrueKeyword* expr) {
    return true;
}

R Interpreter::visitNumericLiteral(NumericLiteral* expr) {
    return expr->text;
}

R Interpreter::visitStringLiteral(StringLiteral* expr) {
    return expr->text;
}

R Interpreter::visitIdentifier(IdentifierExpression* expr) {
    return env->getValue(expr->name);
}

R Interpreter::visitFunction(FunctionDeclaration* stmt) {
    
    auto id = stmt->id;
        
    env->setValue(id, id);
    env->setFunctionDeclarations(id, std::move(stmt->params), std::move(stmt->body));

    return true;
}

R Interpreter::visitIf(IfStatement* stmt) {
    
    if (truthy(stmt->test->accept(*this))) {
        
        stmt->consequent->accept(*this);
        
    } else {
        
        stmt->alternate->accept(*this);
        
    }
    
    return true;
    
}

R Interpreter::visitWhile(WhileStatement* stmt) {
    
    while (truthy(stmt->test->accept(*this))) {
        
        try {
            
            stmt->body->accept(*this);
            
        } catch(BreakException&) {
            
            break;
            
        } catch(ContinueException&) {
            
            continue;
            
        }
        
    }
    
    return monostate{};
    
}

R Interpreter::visitFor(ForStatement* stmt) {
    
    if (stmt->init == nullptr) {
        throw runtime_error("Initializer must be declared and initialized.");
        return false;
    }

    if (stmt->test == nullptr) {
        throw runtime_error("Condition must be declared and initialized.");
        return false;
    }

    if (stmt->update == nullptr) {
        throw runtime_error("Update statement must be declared and initialized.");
        return false;
    }
    
    stmt->init->accept(*this);
    
    while (truthy(stmt->test->accept(*this))) {
        
        try {
            
            stmt->body->accept(*this);
            
        } catch(BreakException&) {
            
            break;
            
        } catch(ContinueException&) {
            
            continue;
            
        }
        
        stmt->update->accept(*this);
        
    }

    return true;
    
}

// loop over objects keys
R Interpreter::visitForIn(ForInStatement* stmt) {
    return true;
}

// loop over iterables: array, string
R Interpreter::visitForOf(ForOfStatement* stmt) {
    return true;
}

R Interpreter::visitReturn(ReturnStatement* stmt) {
    R value = stmt->argument ? stmt->argument->accept(*this) : monostate{};
    throw ReturnException{ value };
}

R Interpreter::visitBreak(BreakStatement* stmt) {
    throw BreakException();
}

R Interpreter::visitContinue(ContinueStatement* stmt) {
    throw ContinueException();
}

R Interpreter::visitThrow(ThrowStatement* stmt) {
    
    throw runtime_error(toString(stmt->argument->accept(*this)));
    
    return false;
}

R Interpreter::visitEmpty(EmptyStatement* stmt) {
    return true;
}

R Interpreter::visitClass(ClassDeclaration* stmt) {
    return true;
}

R Interpreter::visitMethodDefinition(MethodDefinition* stmt) {
    return true;
}

R Interpreter::visitDoWhile(DoWhileStatement* stmt) {
    
    if (stmt->condition == nullptr) {
        throw runtime_error("The test condition must evaluate to a bool.");
        return false;
    }
    
    do {
        
        stmt->body->accept(*this);
        
    } while(truthy(stmt->condition->accept(*this)));
        
    return true;
    
}

R Interpreter::visitSwitchCase(SwitchCase* stmt) {
    
    for (auto& current_stmt : stmt->consequent) {
        current_stmt->accept(*this);
    }
    
    return true;
}

R Interpreter::visitSwitch(SwitchStatement* stmt) {
    
    R value = stmt->discriminant->accept(*this);
    
    for (auto& current_case : stmt->cases) {
        
        if (current_case->test == nullptr) {
            // we have hit default case
            current_case->accept(*this);
            continue;
        }
        
        R testVal = current_case->test->accept(*this);
                
        bool eq = equals(value, testVal);
                
        if (eq) {
            current_case->accept(*this);
            break;
        }
        
    }
    
    return true;
}

R Interpreter::visitCatch(CatchClause* stmt) {
    return true;
}

R Interpreter::visitTry(TryStatement* stmt) {
    try {
        if (stmt->block)
            stmt->block->accept(*this);
    } catch (const std::exception& err) {
        if (stmt->handler) {
            // Enter a new environment scope for catch
            Env* previous = env;
            env = new Env(previous);
            // Bind the exception to the catch variable
            env->setValue(stmt->handler->param, std::string(err.what()));
            try {
                stmt->handler->accept(*this); // invokes visitCatch
            } catch(...) {
                delete env;
                env = previous;
                throw;
            }
            delete env;
            env = previous;
        } else {
            // No catch handler: propagate
            throw;
        }
    } catch (...) {
        // Non-std::exception: only handle if catch handler present
        if (stmt->handler) {
            Env* previous = env;
            env = new Env(previous);
            env->setValue(stmt->handler->param, "unknown error");
            try {
                stmt->handler->accept(*this);
            } catch(...) {
                delete env;
                env = previous;
                throw;
            }
            delete env;
            env = previous;
        } else {
            throw;
        }
    }
    if (stmt->finalizer) {
        stmt->finalizer->accept(*this);
    }
    return true;
}

// -------- Expressions --------
R Interpreter::visitLiteral(LiteralExpression* expr) {
    return expr->token.lexeme;
}

R Interpreter::visitBinary(BinaryExpression* expr) {
    R lvalue = expr->left->accept(*this);
    R rvalue = expr->right->accept(*this);

    switch (expr->op.type) {
        // --- Arithmetic ---
        case TokenType::ADD: {
            if (holds_alternative<string>(lvalue) || holds_alternative<string>(rvalue)) {
                return toString(lvalue) + toString(rvalue);
            }
            return toNumber(lvalue) + toNumber(rvalue);
        }
        case TokenType::MINUS:
            return toNumber(lvalue) - toNumber(rvalue);
        case TokenType::MUL:
            return toNumber(lvalue) * toNumber(rvalue);
        case TokenType::DIV:
            return toNumber(lvalue) / toNumber(rvalue);
        case TokenType::MODULI:
            return fmod(toNumber(lvalue), toNumber(rvalue));
        case TokenType::POWER:
            return pow(toNumber(lvalue), toNumber(rvalue));

        // --- Comparisons ---
        case TokenType::VALUE_EQUAL:       return toNumber(lvalue) == toNumber(rvalue);
        case TokenType::REFERENCE_EQUAL:   return lvalue == rvalue; // shallow equality
        case TokenType::INEQUALITY:        return toNumber(lvalue) != toNumber(rvalue);
        case TokenType::STRICT_INEQUALITY: return lvalue != rvalue;

        case TokenType::LESS_THAN:         return toNumber(lvalue) < toNumber(rvalue);
        case TokenType::LESS_THAN_EQUAL:   return toNumber(lvalue) <= toNumber(rvalue);
        case TokenType::GREATER_THAN:      return toNumber(lvalue) > toNumber(rvalue);
        case TokenType::GREATER_THAN_EQUAL:return toNumber(lvalue) >= toNumber(rvalue);

        // --- Logical ---
        case TokenType::LOGICAL_AND:       return truthy(lvalue) && truthy(rvalue);
        case TokenType::LOGICAL_OR:        return truthy(lvalue) || truthy(rvalue);
        case TokenType::NULLISH_COALESCING:return (isNullish(lvalue) ? rvalue : lvalue);

        // --- Bitwise ---
        case TokenType::BITWISE_AND:       return (int)toNumber(lvalue) & (int)toNumber(rvalue);
        case TokenType::BITWISE_OR:        return (int)toNumber(lvalue) | (int)toNumber(rvalue);
        case TokenType::BITWISE_XOR:       return (int)toNumber(lvalue) ^ (int)toNumber(rvalue);
        case TokenType::BITWISE_LEFT_SHIFT:return (int)toNumber(lvalue) << (int)toNumber(rvalue);
        case TokenType::BITWISE_RIGHT_SHIFT:return (int)toNumber(lvalue) >> (int)toNumber(rvalue);
        case TokenType::UNSIGNED_RIGHT_SHIFT:return ((unsigned int)toNumber(lvalue)) >> (int)toNumber(rvalue);

        // --- Assignment & Compound Assignment ---
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
        case TokenType::NULLISH_COALESCING_ASSIGN: {
            // Left must be identifier
            auto* ident = dynamic_cast<IdentifierExpression*>(expr->left.get());
            if (!ident) throw runtime_error("Invalid left-hand side in assignment");

            string name = ident->name;
            R current = env->get(name);

            R newVal;
            switch (expr->op.type) {
                case TokenType::ASSIGN: newVal = rvalue; break;
                case TokenType::ASSIGN_ADD: newVal = visitBinary(new BinaryExpression(Token(TokenType::ADD), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::ASSIGN_MINUS: newVal = visitBinary(new BinaryExpression(Token(TokenType::MINUS), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::ASSIGN_MUL: newVal = visitBinary(new BinaryExpression(Token(TokenType::MUL), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::ASSIGN_DIV: newVal = visitBinary(new BinaryExpression(Token(TokenType::DIV), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::MODULI_ASSIGN: newVal = fmod(toNumber(current), toNumber(rvalue)); break;
                case TokenType::POWER_ASSIGN: newVal = pow(toNumber(current), toNumber(rvalue)); break;
                case TokenType::BITWISE_AND_ASSIGN: newVal = (int)toNumber(current) & (int)toNumber(rvalue); break;
                case TokenType::BITWISE_OR_ASSIGN: newVal = (int)toNumber(current) | (int)toNumber(rvalue); break;
                case TokenType::BITWISE_XOR_ASSIGN: newVal = (int)toNumber(current) ^ (int)toNumber(rvalue); break;
                case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: newVal = (int)toNumber(current) << (int)toNumber(rvalue); break;
                case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: newVal = (int)toNumber(current) >> (int)toNumber(rvalue); break;
                case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: newVal = ((unsigned int)toNumber(current)) >> (int)toNumber(rvalue); break;
                case TokenType::LOGICAL_AND_ASSIGN: newVal = truthy(current) ? rvalue : current; break;
                case TokenType::LOGICAL_OR_ASSIGN: newVal = truthy(current) ? current : rvalue; break;
                case TokenType::NULLISH_COALESCING_ASSIGN: newVal = isNullish(current) ? rvalue : current; break;
                default: throw runtime_error("Unsupported assignment op");
            }

            env->assign(name, newVal); // update variable
            return newVal;
        }

        default:
            throw runtime_error("Unknown binary operator: " + expr->op.lexeme);
    }
}

R Interpreter::visitUnary(UnaryExpression* expr) {
    
    Token token = expr->op;
    
    R rvalue = expr->right->accept(*this);
        
    switch (token.type) {

            // !true
        case TokenType::LOGICAL_NOT: {
            
            // R post_r_value = env->get(toString(rvalue));
            if (holds_alternative<bool>(rvalue)) {
                return !get<bool>(rvalue);
            }
            
            throw runtime_error("Logical NOT lvalue must be a bool.");

        }
          
            // ~9
        case TokenType::BITWISE_NOT: {
            
            if (holds_alternative<int>(rvalue)) return ~(get<int>(rvalue));
            if (holds_alternative<size_t>(rvalue)) return ~(get<size_t>(rvalue));
            if (holds_alternative<char>(rvalue)) return ~(get<char>(rvalue));
            if (holds_alternative<bool>(rvalue)) return ~(get<bool>(rvalue) ? 1 : 0);

            throw runtime_error("Invalid operand to perform ~.");
            
        }
            // ++age
        case TokenType::INCREMENT: {
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->right.get())) {
                
                R value = env->get(ident->name);
                
                env->assign(ident->name, (toNumber(rvalue) + 1));
                
                return value;
                
            }
            
            throw runtime_error("Invalid operand, the opearnd must be a variable.");
            
        }
            
            // --age
        case TokenType::DECREMENT: {

            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->right.get())) {
                
                R value = env->get(ident->name);
                
                env->assign(ident->name, (toNumber(rvalue) - 1));
                
                return value;
                
            }
            
            throw runtime_error("Invalid operand, the opearnd must be a variable.");

        }
            
        // case TokenType::ADD: {}
            
        // case TokenType::MINUS: {}
            
        default:
            throw runtime_error("Unknown op found.");
            break;
    }
    
}

R Interpreter::visitUpdate(UpdateExpression* expr) {
        
    R value = expr->argument->accept(*this);

    switch (expr->op.type) {
            
        case TokenType::INCREMENT: {
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
                R sum = toNumber(value) + 1;
                env->assign(ident->name, sum);
            }
            
            return monostate();
        }
            
        case TokenType::DECREMENT: {
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
                R sum = toNumber(value) - 1;
                env->assign(ident->name, sum);
            }

            return monostate();
        }
            
        default:
            throw runtime_error("Unknown op found.");
            break;
    }
    
    return true;
    
}

R Interpreter::visitAssignment(AssignmentExpression* expr) {
    return true;
}

R Interpreter::visitLogical(LogicalExpression* expr) {
    return true;
}

R Interpreter::visitConditional(ConditionalExpression* expr) {

    R test_value = expr->test->accept(*this);
    
    if (truthy(test_value)) {
        return expr->consequent->accept(*this);
    } else {
        return expr->alternate->accept(*this);
    }
        
    return true;
        
}

R Interpreter::visitMember(MemberExpression* expr) { return true; }
R Interpreter::visitThis(ThisExpression* expr) { return true; }
R Interpreter::visitNew(NewExpression* expr) { return true; }
R Interpreter::visitArray(ArrayLiteralExpression* expr) { return true; }
R Interpreter::visitObject(ObjectLiteralExpression* expr) { return true; }
R Interpreter::visitSuper(SuperExpression* expr) { return true; }
R Interpreter::visitProperty(PropertyExpression* expr) { return true; }

R Interpreter::visitSequence(SequenceExpression* expr) {
    
    R last_value;
    
    for (auto& expression : expr->expressions) {
        
        Expression* raw = expression.get();
        R value = raw->accept(*this);
        last_value = value;
        
    }
    
    return last_value;
}

R Interpreter::visitPublicKeyword(PublicKeyword* expr) { return true; }
R Interpreter::visitPrivateKeyword(PrivateKeyword* expr) { return true; }
R Interpreter::visitProtectedKeyword(ProtectedKeyword* expr) { return true; }
R Interpreter::visitStaticKeyword(StaticKeyword* expr) { return true; }

