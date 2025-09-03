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
#include "ExecutionContext/JSArray/JSArray.h"
#include "ExecutionContext/Value/Value.h"
#include "../builtin/Print/Print.hpp"
#include "../builtin/builtin-includes.h"

Interpreter::Interpreter() {
    env = new Env();
    
    // init all builtins
    env->set_var("Math", make_shared<Math>());
    env->set_var("console", make_shared<Print>());
    // env->set_var("readFile", );
    
}

Interpreter::~Interpreter() {
    delete env;
}

void Interpreter::execute(vector<unique_ptr<Statement>> ast) {
    AstPrinter printer;

    for (unique_ptr<Statement>& stmt : ast) {
        // cout << "--------------" << endl;
        stmt->accept(*this);
        // stmt->accept(printer);
        // cout << "**************" << endl;
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
    
    // inherit or set `this_binding`
    env->this_binding = previous->this_binding;
    
    for (auto& item : previous->getStack()) {
        env->set_var(item.first, item.second);
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
            
            if (kind == "CONST" && declarator.init == nullptr) {
                throw runtime_error("Missing initializer in const declaration: " + declarator.id);
            }
            
            if (declarator.init) {
                
                R value = declarator.init->accept(*this);

                if (NewExpression* new_expr = dynamic_cast<NewExpression*>(declarator.init.get())) {

                    std::shared_ptr<JSObject> object = std::get<std::shared_ptr<JSObject>>(value);

                    // value is a JSObject.
                    // run the constructor
                    R klass = env->get((dynamic_cast<IdentifierExpression*>(new_expr->callee.get())->name));
                    std::shared_ptr<JSClass> new_klass = std::get<std::shared_ptr<JSClass>>(klass);
                    
                    // get the constructor
                    MethodDefinition* constructor = new_klass->methods["constructor"].get();
                    
                    if (constructor != nullptr) {
                        
                        int index = 0;
                        for (auto& constructor_arg : constructor->params) {
                            
                            string key;
                            R arg_value;
                            
                            if (VariableStatement* variable = dynamic_cast<VariableStatement*>(constructor_arg.get())) {
                                key = variable->kind;
                            } else if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(constructor_arg.get())) {
                                key = ident->token.lexeme;
                            }
                            
                            arg_value = new_expr->arguments[index]->accept(*this);
                            
                            object->set(key, toValue(arg_value));
                            env->setStackValue(key, arg_value);
                            
                            index++;
                            
                        }
                        
                        env->this_binding = object;
                        
                        constructor->methodBody->accept(*this);
                        
                    }

                }
                
                if (kind == "VAR") {
                    env->set_var(declarator.id, value);
                } else if (kind == "LET") {
                    env->set_let(declarator.id, value);
                } else if (kind == "CONST") {
                    
                    // check if const already exists
                    
                    if (env->is_const_key_set(declarator.id)) {
                        throw runtime_error("Cannot assign value to a const variable.");
                    }
                    
                    env->set_const(declarator.id, value);
                    
                }
                
            }
        }
        
    }
    
    return true;
    
}

R Interpreter::visitCall(CallExpression* expr) {
        
    // TODO: check if callee is a MemberExpression
    if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->callee.get())) {
        
        // it is a dot access
        // get the object name and the JSObject.
        R object = member->object->accept(*this);
        shared_ptr<JSObject> js_object = get<shared_ptr<JSObject>>(object);
        
        string property_name = member->name.lexeme;
                
        // check if its a native function
        Value propVal = js_object->get(property_name);

        if (propVal.type == ValueType::NATIVE_FUNCTION) {
                        
            vector<Value> argValues;
            for (auto& arg : expr->arguments) {
                
                R visited_value = arg->accept(*this);
                
                Value converted_visited_value = toValue(visited_value);
                
                argValues.push_back(converted_visited_value);
                
            }
            
            R val = propVal.nativeFunction(argValues);
            
            return val;
            
        }
        
        // here, we know it is not a built-in function.

        for (auto& arg : expr->arguments) {
            
            string key;
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(arg.get())) {
                key = ident->name;
            }
            
            env->setStackValue(key, arg->accept(*this));
            
        }
        
        // this property name is the name of method in the object to be called.
        MethodDefinition* method = js_object->getKlass()->methods[property_name].get();

        env->this_binding = js_object;
        
        method->accept(*this);
        
        return monostate();

    }
    
    // ------- end of member access --------
    
    // ------- super -------
    if (SuperExpression* super = dynamic_cast<SuperExpression*>(expr->callee.get())) {
        // copy all props and methods
        return expr->callee->accept(*this);
    }
    
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
        Print::print(vectorArg);
        return std::monostate{};
    }
    
    // search call name in function declarations.

    vector<Expression*> args = env->getFunctionParams(get<std::string>(callee));
    Statement* body = env->getFunctionBody(get<std::string>(callee));
    
    int arg_index = 0;
    
    for (Expression* arg : args) {
        
        string key;
        
        if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(arg)) {
            key = ident->name;
        }
                
        env->setStackValue(key, expr->arguments[arg_index]->accept(*this));

        arg_index++;
        
    }

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
    return expr->value;
}

R Interpreter::visitStringLiteral(StringLiteral* expr) {
    return expr->text;
}

R Interpreter::visitIdentifier(IdentifierExpression* expr) {
    return env->getValue(expr->name);
}

R Interpreter::visitFunction(FunctionDeclaration* stmt) {
    
    auto id = stmt->id;
        
    env->set_var(id, id);
    env->setFunctionDeclarations(id, std::move(stmt->params), std::move(stmt->body));

    return true;
}

R Interpreter::visitIf(IfStatement* stmt) {
    
    if (truthy(stmt->test->accept(*this))) {
        
        stmt->consequent->accept(*this);
        
    } else {
        
        if (stmt->alternate != nullptr) {
            stmt->alternate->accept(*this);
        }
        
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
    
    //    unique_ptr<Statement> init;
    //    unique_ptr<Expression> object;
    //    unique_ptr<Statement> body;
    // for (let key in object) for (key of object)
    
    // init can be VariableStatement or Identifier
    string variable;
    
    if (VariableStatement* variable_stmt = dynamic_cast<VariableStatement*>(stmt->init.get())) {
        
        if (variable_stmt->declarations.size() == 1) {
            variable = variable_stmt->declarations[0].id;
        }
        
    } else if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(stmt->init.get())) {
        variable = ident->name;
    }
    
    // get the object from env
    R value = stmt->object->accept(*this);
    shared_ptr<JSObject> js_object = get<shared_ptr<JSObject>>(value);
    
    // loop through the object fields
    for (auto& field : *js_object->get_all_properties()) {
        
        try {
            
            env->setStackValue(variable, field.first);
            stmt->body->accept(*this);
            
        } catch(BreakException&) {
            
            break;
            
        } catch(ContinueException&) {
            
            continue;
            
        }
        
    }

    return true;
    
}

// loop over iterables: array, string
R Interpreter::visitForOf(ForOfStatement* stmt) {

    //    std::unique_ptr<Statement> left; // variable declaration or expression
    //    std::unique_ptr<Expression> right; // iterable expression
    //    std::unique_ptr<Statement> body;

    // for (let key of array) for (key of array)
    // init can be VariableStatement or Identifier
    
    string variable;
    
    if (VariableStatement* variable_stmt = dynamic_cast<VariableStatement*>(stmt->left.get())) {
        
        if (variable_stmt->declarations.size() == 1) {
            variable = variable_stmt->declarations[0].id;
        }
        
    } else if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(stmt->left.get())) {
        
        variable = ident->name;
        
    }
    
    // TODO: visit array
    
    shared_ptr<JSArray> js_array = get<shared_ptr<JSArray>>(stmt->right->accept(*this));
    
    for (auto& element : *js_array->get_all_properties()) {
        
        if (element.first == "length") {
            continue;
        }
        
        try {
            
            auto shared_element = std::make_shared<Value>();
            *shared_element = element.second;

            env->setStackValue(variable, shared_element);
            stmt->body->accept(*this);
            
        } catch(BreakException&) {
            
            break;
            
        } catch(ContinueException&) {
            
            continue;
            
        }
        
    }

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
    
    auto js_class = make_shared<JSClass>();
    
    // loop through fields
    for (auto& field : stmt->fields) {
        
        // check tha field is a variable statement
        if (VariableStatement* variable = dynamic_cast<VariableStatement*>(field->property.get())) {
            
            if (variable->declarations.size() > 1) {
                throw runtime_error("You cannot have multiple variable declarations here.");
            }
     
            js_class->fields[variable->declarations[0].id] = std::move(field);
     
        }
        
    }
        
    // loop through methods
    for (auto& method : stmt->body) {
        js_class->methods[method->name] = std::move(method);
    }
    
    js_class->name = stmt->id;
    
    if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(stmt->superClass.get())) {
        js_class->superClass = get<shared_ptr<JSClass>>(env->get(ident->name));
    }
    
    env->set_var(stmt->id, js_class);

    return js_class;
    
}

R Interpreter::visitMethodDefinition(MethodDefinition* stmt) {
    
    stmt->methodBody->accept(*this);
    
    return monostate();
    
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
    if (stmt->body) {
        stmt->body->accept(*this);
    }
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
            env->set_var(stmt->handler->param, std::string(err.what()));
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
            env->set_var(stmt->handler->param, "unknown error");
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
        case TokenType::REFERENCE_EQUAL:   return equals(lvalue, rvalue);
        case TokenType::INEQUALITY:        return toNumber(lvalue) != toNumber(rvalue);
        case TokenType::STRICT_INEQUALITY: return !equals(lvalue, rvalue);

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

// user["name"]; user.age; this.age; this["age"]; super.age;
// TODO: we need to check if the property access is a method call.
R Interpreter::visitMember(MemberExpression* expr) {
    
    //    unique_ptr<Expression> object;
    //    unique_ptr<Expression> property;
    //    bool computed; // true for [], false for .
    //    Token name;
    
    R object_value = expr->object->accept(*this);
    string property_name;

    shared_ptr<JSObject> js_object_instance;
    Value return_value;
        
    // this supports the "this"
    if (holds_alternative<shared_ptr<JSObject>>(object_value)) {
        
        if (expr->computed) {
            
            // []
            R property_value = expr->property->accept(*this);
            
            if (!holds_alternative<string>(property_value)) {
                throw runtime_error("The computed property is not supported. It must be a string.");
            }
            
            property_name = get<string>(property_value);
            
        } else {
            
            // .
            property_name = expr->name.lexeme;
            
        }

        js_object_instance = get<shared_ptr<JSObject>>(object_value);
        
        return_value = js_object_instance->get(property_name);

    } else {
        
        if (!holds_alternative<string>(object_value)) {
            throw runtime_error("The object name must be an identifier.");
        }
        
        // TODO: add support for "this" and "super" access.
        
        string object_name = get<string>(object_value);
        
        if (expr->computed) {
            
            // []
            R property_value = expr->property->accept(*this);
            
            if (!holds_alternative<string>(property_value)) {
                throw runtime_error("The computed property is not supported. It must be a string.");
            }
            
            property_name = get<string>(property_value);
            
        } else {
            
            // .
            property_name = expr->name.lexeme;
            
        }
        
        R object_instance = env->get(object_name);
        js_object_instance = get<shared_ptr<JSObject>>(object_instance);
        
        return_value = js_object_instance->get(property_name);

    }
    
    return std::make_shared<Value>(return_value);
    
}

R Interpreter::visitThis(ThisExpression* expr) {
    return env->this_binding;
}

R Interpreter::visitNew(NewExpression* expr) {

//    unique_ptr<Expression> callee;
//    vector<unique_ptr<Expression>> arguments;

    auto object = make_shared<JSObject>();
    
    if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->callee.get())) {

        const string class_name = ident->name;
        
        // get from env
        R value = env->get(class_name);
        
        if (!holds_alternative<std::shared_ptr<JSClass>>(value)) {
            throw runtime_error("New keyword should always instantiate a class.");
            return monostate();
        }

        std::shared_ptr<JSClass> new_class = std::get<std::shared_ptr<JSClass>>(value);
        object->setClass(new_class);
        
        // set properties from class value to object.
        // remember modifiers
        
        for (auto& field : new_class->fields) {

            // property is a Statement: VariableStatement
            if (VariableStatement* variable = dynamic_cast<VariableStatement*>(field.second->property.get())) {

                for (auto& declarator : variable->declarations) {
                    
                    if (declarator.init == nullptr) {
                        throw runtime_error("Missing initializer in const declaration: " + declarator.id);
                    }
                    
                    if (declarator.init) {
                        
                        R value = declarator.init->accept(*this);

                        // TODO: fix
                        object->set(declarator.id, toValue(value));

                    }
                }

            }

        }
                
        // hold constructor
        MethodDefinition* constructor = nullptr;
        
        // copy methods
        for (auto& method : new_class->methods) {
            
            if (method.first == "constructor") {
                constructor = method.second.get();
            }
            
            object->set(method.first, Value("@@method@@"));
        }
        
        object->parent_class = new_class->superClass;
                    
    }
        
    return object;
    
}

R Interpreter::visitArray(ArrayLiteralExpression* expr) {
    
    auto arr = make_shared<JSArray>();
    
    int index = 0;
    
    for(auto& element : expr->elements) {
        const R value = element->accept(*this);
        arr->setIndex(index, toValue(value));
        index++;
    }
        
    return arr;
    
}

R Interpreter::visitObject(ObjectLiteralExpression* expr) {
    
    auto object = make_shared<JSObject>();
    
    for (auto& prop : expr->props) {
        object->set(prop.first.lexeme, toValue(prop.second->accept(*this)));
    }

    return object;
    
}

R Interpreter::visitSuper(SuperExpression* expr) {
    
    shared_ptr<JSClass> new_class = env->this_binding->parent_class;
    
    auto object = make_shared<JSObject>();

    // add all props from this_binding to parent class.
    for (auto& field : new_class->fields) {

        // property is a Statement: VariableStatement
        if (VariableStatement* variable = dynamic_cast<VariableStatement*>(field.second->property.get())) {

            for (auto& declarator : variable->declarations) {
                
                if (declarator.init == nullptr) {
                    throw runtime_error("Missing initializer in const declaration: " + declarator.id);
                }
                
                if (declarator.init) {
                    
                    R value = declarator.init->accept(*this);

                    // TODO: fix
                    object->set(declarator.id, toValue(value));

                }
            }

        }

    }
                
    // copy methods
    for (auto& method : new_class->methods) {
                
        object->set(method.first, Value("@@method@@"));
        
    }
    
    env->this_binding->parent_object = object;
    
    return object;
    
}

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

R Interpreter::visitPublicKeyword(PublicKeyword* expr) { return "public"; }
R Interpreter::visitPrivateKeyword(PrivateKeyword* expr) { return "private"; }
R Interpreter::visitProtectedKeyword(ProtectedKeyword* expr) { return "protected"; }
R Interpreter::visitStaticKeyword(StaticKeyword* expr) { return "static"; }

