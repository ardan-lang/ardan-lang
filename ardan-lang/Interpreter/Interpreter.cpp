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
#include "../Scanner/Scanner.hpp"
#include "../Parser/Parser.hpp"
#include "../GUI/gui.h"
#include "Promise/Promise.hpp"
#include "../builtin/Server/Server.hpp"

Interpreter::Interpreter() {
    env = new Env();
    event_loop = new EventLoop();
    
    // init all builtins
    init_builtins();

}

Interpreter::Interpreter(Env* local_env) {
    
    env = local_env;
    event_loop = new EventLoop();

    // init all builtins
    init_builtins();

}

Interpreter::~Interpreter() {

    if (env != nullptr) {
        delete env;
    }
    
    if (event_loop != nullptr) {
        event_loop->stop();
        delete event_loop;
    }
    
}

void Interpreter::init_builtins() {
    
    env->set_var("Math", make_shared<Math>());
    env->set_var("console", make_shared<Print>());
    env->set_var("fs", make_shared<File>());
    env->set_var("Server", make_shared<Server>(event_loop));
    
    env->set_var("print", Value::function([this](vector<Value> args) mutable -> Value {
        Print::print(args);
        return Value::nullVal();
    }));
    
//    env->set_var("window", Value::function([this](vector<Value> args) mutable -> Value {
//        
//        gui_init();
//        
//        std::string titleStr = args[0].toString();
//        const char* title = titleStr.c_str();
//        gui_create_window(title,
//                          200,
//                          200,
//                          args[1].numberValue,
//                          args[2].numberValue);
//        
//        return Value::nullVal();
//        
//    }));
//    
//    env->set_var("run", Value::function([this](vector<Value> args) mutable -> Value {
//        
//        gui_run();
//        
//        return Value::nullVal();
//        
//    }));
    
}

void Interpreter::execute(vector<unique_ptr<Statement>> ast) {
    AstPrinter printer;

    for (unique_ptr<Statement>& stmt : ast) {
        stmt->accept(*this);
        // stmt->accept(printer);
    }
    
    // run the event loop
    // add function to the ev
    event_loop->run();
    
}

void Interpreter::executeBlock(unique_ptr<Statement> block) {
    block->accept(*this);
}

R Interpreter::visitExpression(ExpressionStatement* stmt) {
    return stmt->expression->accept(*this);
}

R Interpreter::visitBlock(BlockStatement* stmt) {
    Env* previous = env;
    env = new Env(previous);
    env->this_binding = previous->this_binding;
    for (auto& item : previous->getStack()) {
        env->set_var(item.first, item.second);
    }

    try {
        for (auto& s : stmt->body) {
            s->accept(*this);
        }
    } catch (...) {
        //delete env;
        env = previous;
        throw;
    }

    previous->clearStack();
    previous->this_binding = nullptr;
    //delete env;
    env = previous;
    return true;
}

R Interpreter::visitVariable(VariableStatement* stmt) {
    
    const string kind = stmt->kind;
    
    if ((stmt->declarations).size() > 0) {
        
        for (auto& declarator : stmt->declarations) {
            
            if (kind == CONST && declarator.init == nullptr) {
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
                
                if (kind == VAR) {
                    env->set_var(declarator.id, value);
                } else if (kind == LET) {
                    env->set_let(declarator.id, value);
                } else if (kind == CONST) {
                    
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
        
    // TODO: check if callee is a MemberExpression e.g user.getAge();
    if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->callee.get())) {
        
        // it is a dot access
        // get the object name and the JSObject.
        R object = member->object->accept(*this);
        string property_name = member->name.lexeme;
        Value prop_val;
        
        shared_ptr<JSObject> js_object = nullptr;
        shared_ptr<JSClass> js_class = nullptr;

        // TODO: adding support for static calls. e.g User.getAge();
        if (std::get_if<std::shared_ptr<JSClass>>(&object)) {
            
            js_class = get<std::shared_ptr<JSClass>>(object);
            prop_val = js_class->get(property_name, env->this_binding ? false : true);
            
        }
        else if (std::get_if<std::shared_ptr<JSArray>>(&object)) {
            
            js_object = get<std::shared_ptr<JSArray>>(object);
            check_obj_prop_access(member, js_object.get(), property_name);
            prop_val = js_object->get(property_name);
            
        }
        else if (std::get_if<std::shared_ptr<JSObject>>(&object)) {
            
            js_object = get<std::shared_ptr<JSObject>>(object);
            check_obj_prop_access(member, js_object.get(), property_name);
            prop_val = js_object->get(property_name);
            
        }
        else if (std::get_if<std::shared_ptr<Value>>(&object)) {
            
            shared_ptr<Value> value = get<std::shared_ptr<Value>>(object);
            
            if (value->type == ValueType::ARRAY) {
                js_object = value->arrayValue;
            }
            
            if (value->type == ValueType::OBJECT) {
                js_object = value->objectValue;
            }
            
            check_obj_prop_access(member, js_object.get(), property_name);
            prop_val = js_object->get(property_name);
            
        }
        else if (std::get_if<Value>(&object)) {
            
            Value value = get<Value>(object);
            
            if (value.type == ValueType::ARRAY) {
                js_object = value.arrayValue;
            }
            
            if (value.type == ValueType::OBJECT) {
                js_object = value.objectValue;
            }
            
            check_obj_prop_access(member, js_object.get(), property_name);
            prop_val = js_object->get(property_name);
            
        }

        if (prop_val.type == ValueType::NATIVE_FUNCTION) {
                        
            vector<Value> argValues;
            for (auto& arg : expr->arguments) {
                
                R visited_value = arg->accept(*this);
                
                Value converted_visited_value = toValue(visited_value);
                
                argValues.push_back(converted_visited_value);
                
            }
            
            R val = prop_val.nativeFunction(argValues);
            
            return val;
            
        }
        
        // here, we know it is not a built-in function.
        
        // this property name is the name of method in the object to be called.
        // we need to be able to walkup the superclass chain to find methods
        if (prop_val.type == ValueType::METHOD) {

            MethodDefinition* method = nullptr;

            if (js_class != nullptr) {
                method = prop_val.classValue->methods[property_name].get();
            }
            
            if (js_object != nullptr) {
                method = prop_val.objectValue->getKlass()->methods[property_name].get();
                env->this_binding = js_object;
            }
            
            int index = 0;
            for (auto& arg : expr->arguments) {
                
                string key;
                
                if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(method->params[index].get())) {
                    key = ident->name;
                }
                
                env->setStackValue(key, arg->accept(*this));
                index++;
            }

            try {
                
                method->accept(*this);
                
            } catch (ReturnException& r) {
                
                return r.value;
                
            }
            
        }
        
        return monostate();

    }
    
    // ------- end of member access --------
    
    // ------- super ------- super();
    if (SuperExpression* super = dynamic_cast<SuperExpression*>(expr->callee.get())) {
        // copy all props and methods
        return expr->callee->accept(*this);
    }
    
    auto callee = expr->callee->accept(*this);

    vector<Value> vector_args;

    for (auto& arg : expr->arguments) {
        auto accept_arg = arg->accept(*this);
        vector_args.push_back(toValue(accept_arg));
    }

    // check for built-in function.
    // If the callee is "print"
    if (holds_alternative<string>(callee) && get<string>(callee) == "print") {

        Print::print(vector_args);
        return std::monostate{};
    }
    
    // we check for arrow function or user-defined function closure
    if (holds_alternative<Value>(callee) && (get<Value>(callee).type == ValueType::FUNCTION)) {
        vector<Value> vector_value_args;

        for (auto& vector_arg : vector_args) {
            vector_value_args.push_back(toValue(vector_arg));
        }

        return get<Value>(callee).functionValue(vector_value_args);
        
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
        
        if (RestParameter* rest_parameter = dynamic_cast<RestParameter*>(arg)) {
            
            auto array = make_shared<JSArray>();
            int arr_index = 0;
            
            for (size_t i = arg_index; i < expr->arguments.size(); i++) {
                array->setIndex(arr_index, toValue(expr->arguments[i]->accept(*this)));
                arr_index++;
            }
            
            env->setStackValue(get<string>(rest_parameter->accept(*this)), array);

            break;
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
    // check if name is function or class
    // construct and return function
    return env->getValue(expr->name);
}

R Interpreter::visitFunction(FunctionDeclaration* stmt) {
    
    auto id = stmt->id;
        
    env->set_var(id, create_func_expr(stmt));
    // env->setFunctionDeclarations(id, std::move(stmt->params), std::move(stmt->body));

    return true;
}

R Interpreter::create_func_expr(FunctionDeclaration* stmt) {
    
    auto id = stmt->id;
    Env* closureEnv = env; // capture the current environment
    auto lexicalThis = env->this_binding;
    auto intr = this;

    // Build a closure Value that can be called
    Value closureValue = Value::function([stmt, closureEnv, lexicalThis, intr, this](vector<Value> args) mutable -> Value {

        auto promise = std::make_shared<Promise>();

        auto callback_func = [stmt, closureEnv, lexicalThis, intr, promise](vector<Value> args) -> Value {
            
            Env* localEnv = new Env(closureEnv);
            localEnv->this_binding = lexicalThis;
            auto prevEnv = intr->env;
            intr->env = localEnv;
            
            // Bind parameters to arguments
            for (size_t i = 0; i < stmt->params.size(); ++i) {
                Value paramValue = (i < args.size()) ? args[i] : Value::undefined();
                Expression* param_expr = stmt->params[i].get();
                std::string paramName;
                if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(param_expr)) {
                    paramName = ident->token.lexeme;
                } else if (VariableStatement* variable = dynamic_cast<VariableStatement*>(param_expr)) {
                    paramName = variable->declarations[0].id;
                    if (i >= args.size()) {
                        paramValue = toValue(variable->declarations[0].init->accept(*intr));
                    }
                } else if (RestParameter* rest_expr = dynamic_cast<RestParameter*>(param_expr)) {
                    paramName = toValue(param_expr->accept(*intr)).stringValue;
                    auto array = make_shared<JSArray>();
                    int arr_index = 0;
                    for (size_t rest_i = i; rest_i < args.size(); rest_i++) {
                        array->setIndex(arr_index, args[rest_i]);
                        arr_index++;
                    }
                    localEnv->set_var(paramName, array);
                    break;
                } else {
                    if (BinaryExpression* bin_expr = dynamic_cast<BinaryExpression*>(param_expr)) {
                        auto left = dynamic_cast<IdentifierExpression*>(bin_expr->left.get());
                        if (left) {
                            paramName = left->token.lexeme;
                            if (paramValue.type == ValueType::UNDEFINED || paramValue.type == ValueType::NULLTYPE) {
                                paramValue = toValue(bin_expr->right->accept(*intr));
                            }
                        }
                    }
                }
                localEnv->set_var(paramName, paramValue);
            }
            
            try {
                
                if (stmt->body) {
                    stmt->body->accept(*intr);
                }
                promise->resolve(Value::undefined());

                intr->env = prevEnv;
                return Value::undefined();
                
            } catch (const ReturnException& r) {
                
                promise->resolve(toValue(r.value));
                intr->env = prevEnv;
                return toValue(r.value);
                
            } catch (std::exception& e) {
                
                intr->env = prevEnv;

                // If it throws, reject the promise
                promise->reject(Value::str(e.what()));
                return Value::str(e.what());
                
            }
            
        };
        
        if (stmt->is_async) {
            // what happens here?
            // it pushes callback to Event Loop queue.
            // what happens inside the callback?
            
            event_loop->post(callback_func, args);
            
            return Value::promise(promise);

        }
        
        return callback_func(args);

    });
    
    return closureValue;

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
    for (auto field : js_object->get_all_properties()) {
        
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
    
    for (auto& element : js_array->get_indexed_properties()) {
                
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

R Interpreter::visitEmpty(EmptyStatement* stmt) {
    return true;
}

// TODO: evaluate static fields
R Interpreter::visitClass(ClassDeclaration* stmt) {
    
    shared_ptr<JSClass> js_class = get<shared_ptr<JSClass>>(create_js_class(std::move(stmt->fields),
                                    std::move(stmt->body),
                                    stmt->superClass.get()));

    js_class->name = stmt->id;

    env->set_var(stmt->id, js_class);

    return js_class;
    
}

R Interpreter::create_js_class(vector<unique_ptr<PropertyDeclaration>> fields, vector<unique_ptr<MethodDefinition>> body, Expression* superClass) {
        
    auto js_class = make_shared<JSClass>();
    
    // loop through fields
    for (auto& field : fields) {
        
        vector<string> field_modifiers;
        
        // check tha field is a variable statement
        if (VariableStatement* variable = dynamic_cast<VariableStatement*>(field->property.get())) {
            
            if (variable->declarations.size() > 1) {
                throw runtime_error("You cannot have multiple variable declarations here.");
            }
            
            bool is_static = false;
            
            for (auto& modifier : field->modifiers) {

                string current_modifier = get<string>(modifier->accept(*this));
                field_modifiers.push_back(current_modifier);

                if (current_modifier == "static") {
                    is_static = true;
                }
                
            }
            
            if (is_static) {

                if (VariableStatement* variable_stmt = dynamic_cast<VariableStatement*>(field->property.get())) {
                    
                    if (variable_stmt->declarations.size() == 0) {
                        throw runtime_error("static field must be initialized.");
                    }
                    
                    if (variable_stmt->declarations.size() > 1) {
                        throw runtime_error("Multiple declarations is not allowed.");
                    }
                    
                    // var, let or const
                    string var_kind = variable_stmt->kind;
                    string id = variable->declarations[0].id;
                    Value value = toValue(variable_stmt->declarations[0].init->accept(*this));

                    if (var_kind == LET) {
                        js_class->set_let(id, value, field_modifiers);
                    } else if (var_kind == CONST) {
                        js_class->set_const(id, value, field_modifiers);
                    } else {
                        js_class->set_var(id, value, field_modifiers);
                    }
                    
                } else {
                    
                    throw runtime_error("static field should be a variable statement.");
                }
                
            }
            else {
                js_class->fields[variable->declarations[0].id] = std::move(field);
            }
     
        }
        
    }

    // loop through methods
    for (auto& method : body) {
        
        vector<string> field_modifiers;
        bool is_static = false;
        
        for (auto& modifier : method->modifiers) {

            string current_modifier = get<string>(modifier->accept(*this));
            field_modifiers.push_back(current_modifier);

            if (current_modifier == "static") {
                is_static = true;
            }
            
        }

        if (is_static) {
            // we are storing static methods in var
            js_class->set_var(method->name,
                              Value::method(js_class),
                              field_modifiers);
            
            js_class->methods[method->name] = std::move(method);

        } else {
            js_class->methods[method->name] = std::move(method);
        }
        
    }
        
    if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(superClass)) {
        js_class->superClass = get<shared_ptr<JSClass>>(env->get(ident->name));
    }
    
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
        
        try {
            
            current_stmt->accept(*this);
            
        } catch(BreakException&) {
            
            break;
            
        } catch(ContinueException&) {
            
            continue;
            
        }
        
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
    } catch (const Value& v) {
        if (stmt->handler) {
                Env* previous = env;
                env = new Env(previous);
                env->set_var(stmt->handler->param, v);
                stmt->handler->accept(*this);
                env = previous;
            } else {
                throw; // propagate
            }
    } catch (const std::exception& err) {
        if (stmt->handler) {
            // Enter a new environment scope for catch
            Env* previous = env;
            env = new Env(previous);
            // Bind the exception to the catch variable
            env->set_var(stmt->handler->param, toValue(err.what()));
            try {
                stmt->handler->accept(*this); // invokes visitCatch
            } catch(...) {
                //delete env;
                env = previous;
                throw;
            }
            //delete env;
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
                //delete env;
                env = previous;
                throw;
            }
            //delete env;
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

R Interpreter::visitThrow(ThrowStatement* stmt) {
    
    // throw runtime_error(toValue(stmt->argument->accept(*this)).toString());
    
    throw (toValue(stmt->argument->accept(*this)));
                  
    return false;
}

// -------- Expressions --------
R Interpreter::visitLiteral(LiteralExpression* expr) {
    return expr->token.lexeme;
}

R Interpreter::visitBinary(BinaryExpression* expr) {
    R lvalue = expr->left->accept(*this);
    R rvalue = expr->right->accept(*this);
    
    auto add = [](R lvalue, R rvalue) -> R {
        
        if (holds_alternative<string>(lvalue) || holds_alternative<string>(rvalue)) {
            return toValue(lvalue).toString() + toValue(rvalue).toString();
        }
                
        Value l_value = toValue(lvalue);
        Value r_value = toValue(rvalue);
        
        if (l_value.type == ValueType::STRING || r_value.type == ValueType::STRING) {
            return l_value.toString() + r_value.toString();
        }
        
        return l_value.numberValue + r_value.numberValue;
    };
    
    auto minus = [] (R lvalue, R rvalue) -> R {
        return toValue(lvalue).numberValue - toValue(rvalue).numberValue;
    };
    
    auto mul = [] (R lvalue, R rvalue) -> R {
        return toValue(lvalue).numberValue * toValue(rvalue).numberValue;
    };
    
    auto div = [] (R lvalue, R rvalue) -> R {
        return toValue(lvalue).numberValue / toValue(rvalue).numberValue;
    };
    
    switch (expr->op.type) {
        // --- Arithmetic ---
        case TokenType::ADD: return add(lvalue, rvalue);
        case TokenType::MINUS:
            return minus(lvalue, rvalue);
        case TokenType::MUL:
            return mul(lvalue, rvalue);
        case TokenType::DIV:
            return div(lvalue, rvalue);
        case TokenType::MODULI:
            return fmod(toValue(lvalue).numberValue,
                        toValue(rvalue).numberValue);
        case TokenType::POWER:
            return pow(toValue(lvalue).numberValue,
                       toValue(rvalue).numberValue);

        // --- Comparisons ---
        case TokenType::VALUE_EQUAL:       return toValue(lvalue).numberValue == toValue(rvalue).numberValue;
        case TokenType::REFERENCE_EQUAL:   return equals(lvalue, rvalue);
        case TokenType::INEQUALITY:        return toValue(lvalue).numberValue != toValue(rvalue).numberValue;
        case TokenType::STRICT_INEQUALITY: return !equals(lvalue, rvalue);

        case TokenType::LESS_THAN:         return toValue(lvalue).numberValue < toValue(rvalue).numberValue;
        case TokenType::LESS_THAN_EQUAL:   return toValue(lvalue).numberValue <= toValue(rvalue).numberValue;
        case TokenType::GREATER_THAN:      return toValue(lvalue).numberValue > toValue(rvalue).numberValue;
        case TokenType::GREATER_THAN_EQUAL:return toValue(lvalue).numberValue >= toValue(rvalue).numberValue;

        // --- Logical ---
        case TokenType::LOGICAL_AND:       return truthy(lvalue) && truthy(rvalue);
        case TokenType::LOGICAL_OR:        return truthy(lvalue) || truthy(rvalue);
        case TokenType::NULLISH_COALESCING:return (isNullish(lvalue) ? rvalue : lvalue);

        // --- Bitwise ---
        case TokenType::BITWISE_AND:       return (int)toValue(lvalue)
                .numberValue & (int)toValue(rvalue).numberValue;
        case TokenType::BITWISE_OR:        return (int)toValue(lvalue)
                .numberValue | (int)toValue(rvalue).numberValue;
        case TokenType::BITWISE_XOR:       return (int)toValue(lvalue)
                .numberValue ^ (int)toValue(rvalue).numberValue;
        case TokenType::BITWISE_LEFT_SHIFT:return (int)toValue(lvalue)
                .numberValue << (int)toValue(rvalue).numberValue;
        case TokenType::BITWISE_RIGHT_SHIFT:return (int)toValue(lvalue)
                .numberValue >> (int)toValue(rvalue).numberValue;
        case TokenType::UNSIGNED_RIGHT_SHIFT:return ((unsigned int)toValue(lvalue).numberValue) >> (int)toValue(rvalue).numberValue;

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
            
            auto* member_expr = dynamic_cast<MemberExpression*>(expr->left.get());

            if (!ident && !member_expr) throw runtime_error("Invalid left-hand side in assignment");
            
            // TODO: check if we need to support, e.g user.age()

            string name;
            R current;
            
            // add support for member expression
            if (member_expr) {
                // get the member object name.
                auto* member_ident  = dynamic_cast<IdentifierExpression*>(member_expr->object.get());
                if (member_ident) {
                    name = member_ident->token.lexeme;
                    current = env->get(name);
                }
                                
                // TODO: add support for super.age
                
            } else {
                if (ident) {
                    name = ident->name;
                    current = env->get(name);
                }
            }
            
            R newVal;
            switch (expr->op.type) {
                case TokenType::ASSIGN: newVal = rvalue; break;
                case TokenType::ASSIGN_ADD: newVal = add(lvalue, rvalue); /*visitBinary(new BinaryExpression(Token(TokenType::ADD), std::move(expr->left), std::move(expr->right)));*/ break;
                case TokenType::ASSIGN_MINUS: newVal = minus(lvalue, rvalue); break; // visitBinary(new BinaryExpression(Token(TokenType::MINUS), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::ASSIGN_MUL: newVal = mul(lvalue, rvalue); break; // visitBinary(new BinaryExpression(Token(TokenType::MUL), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::ASSIGN_DIV: newVal = div(lvalue, rvalue); break; // visitBinary(new BinaryExpression(Token(TokenType::DIV), std::move(expr->left), std::move(expr->right))); break;
                case TokenType::MODULI_ASSIGN: newVal = fmod(toValue(current).numberValue,
                                                             toValue(rvalue).numberValue); break;
                case TokenType::POWER_ASSIGN: newVal = pow(toValue(current).numberValue,
                                                           toValue(rvalue).numberValue); break;
                case TokenType::BITWISE_AND_ASSIGN: newVal = (int)toValue(current).numberValue & (int)toValue(rvalue).numberValue; break;
                case TokenType::BITWISE_OR_ASSIGN: newVal = (int)toValue(current).numberValue | (int)toValue(rvalue).numberValue; break;
                case TokenType::BITWISE_XOR_ASSIGN: newVal = (int)toValue(current).numberValue ^ (int)toValue(rvalue).numberValue; break;
                case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: newVal = (int)toValue(current).numberValue << (int)toValue(rvalue).numberValue; break;
                case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: newVal = (int)toValue(current).numberValue >> (int)toValue(rvalue).numberValue; break;
                case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: newVal = ((unsigned int)toValue(current).numberValue) >> (int)toValue(rvalue).numberValue; break;
                case TokenType::LOGICAL_AND_ASSIGN: newVal = truthy(current) ? rvalue : current; break;
                case TokenType::LOGICAL_OR_ASSIGN: newVal = truthy(current) ? current : rvalue; break;
                case TokenType::NULLISH_COALESCING_ASSIGN: newVal = isNullish(current) ? rvalue : current; break;
                default: throw runtime_error("Unsupported assignment op");
            }

            if (member_expr) {
            
                string property_name;

                auto* this_epxr = dynamic_cast<ThisExpression*>(member_expr->object.get());
                auto* super_epxr = dynamic_cast<ThisExpression*>(member_expr->object.get());

                if (super_epxr) {
                    current = env->this_binding->parent_object;
                }
                
                // this.name
                if (this_epxr) {
                    current = env->this_binding;
                }
                
                if (auto obj = dynamic_cast<MemberExpression*>(member_expr->object.get())) {
                    current = member_expr->object->accept(*this);
                }

                if (member_expr->computed) {
                    // TODO: evaluate this.
                    auto* property_name_ident = dynamic_cast<IdentifierExpression*>(member_expr->property.get());
                    
                    if (property_name_ident) {
                        property_name = property_name_ident->token.lexeme;
                    } else {
                        R value = member_expr->property->accept(*this);
                        property_name = toValue(value).toString();
                    }
                    
                } else {
                    property_name = member_expr->name.lexeme;
                }
                
                if (holds_alternative<shared_ptr<JSObject>>(current)) {
                    
                    shared_ptr<JSObject> current_object = get<shared_ptr<JSObject>>(current);

                    check_obj_prop_access(member_expr,
                                          current_object.get(),
                                          property_name);
                    
                    current_object
                        .get()->set(property_name, toValue(newVal));

                }
                
                if (holds_alternative<shared_ptr<JSArray>>(current)) {
                    
                    shared_ptr<JSArray> current_object = get<shared_ptr<JSArray>>(current);

                    check_obj_prop_access(member_expr,
                                          current_object.get(),
                                          property_name);
                    
                    current_object
                        .get()->set(property_name, toValue(newVal));

                }

                if (holds_alternative<shared_ptr<JSClass>>(current)) {
                    
                    shared_ptr<JSClass> current_klass = get<shared_ptr<JSClass>>(current);
                    current_klass
                        .get()->set(property_name,
                                    toValue(newVal),
                                    env->this_binding ? false : true);

                }
                
                if (holds_alternative<shared_ptr<Value>>(current)) {
                    
                    shared_ptr<Value> current_value = get<shared_ptr<Value>>(current);
                    
                    if (current_value->type == ValueType::OBJECT) {
                        // TODO: we need to check the "VAR" we sent
                        current_value->objectValue->set(property_name, toValue(newVal), VAR, {});
                    }

                    if (current_value->type == ValueType::ARRAY) {
                        current_value->arrayValue->set(property_name, toValue(newVal));
                    }

                }
                
                if (holds_alternative<Value>(current)) {
                    
                    Value current_value = get<Value>(current);
                    
                    if (current_value.type == ValueType::OBJECT) {
                        // TODO: we need to check the "VAR" we sent
                        current_value.objectValue->set(property_name, toValue(newVal), VAR, {});
                    }

                    if (current_value.type == ValueType::ARRAY) {
                        current_value.arrayValue->set(property_name, toValue(newVal));
                    }

                }
                

                return monostate();
                
            }
            
            env->assign(name, newVal); // update variable
            return newVal;
            
        }

        default:
            throw runtime_error("Unknown binary operator: " + expr->op.lexeme);
    }
}

// !true, ~9, ++user, --user.bar.baz
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
            return !toValue(rvalue).boolean();
            
            // throw runtime_error("Logical NOT lvalue must be a bool.");

        }
          
            // ~9
        case TokenType::BITWISE_NOT: {
            
            if (holds_alternative<int>(rvalue)) return ~(get<int>(rvalue));
            if (holds_alternative<size_t>(rvalue)) return ~(get<size_t>(rvalue));
            if (holds_alternative<char>(rvalue)) return ~(get<char>(rvalue));
            if (holds_alternative<bool>(rvalue)) return ~(get<bool>(rvalue) ? 1 : 0);
            return ~toValue(rvalue).integer();

            // throw runtime_error("Invalid operand to perform ~.");
            
        }
            // ++age
        case TokenType::INCREMENT: {
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->right.get())) {
                
                R value = env->get(ident->name);
                
                env->assign(ident->name, (toValue(rvalue).numberValue + 1));
                
                return value;
                
            } else {
                // this is member expression.
                if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->right.get())) {
                                        
                    // Compute property key
                    std::string key;
                    if (member->computed) {
                        R comp = member->property->accept(*this);
                        key = toValue(comp).toString();
                    } else {
                        key = member->name.lexeme;
                    }
                    
                    shared_ptr<JSClass> js_class = getMemberExprJSClass(member);
                    
                    if (js_class) {
                        
                        Value oldVal = js_class->get(key, env->this_binding ? false : true);
                        Value newVal = toValue(oldVal).numberValue + 1;

                        js_class->set(key, newVal, env->this_binding ? false : true);
                        
                        return oldVal;

                    }
                    
                    shared_ptr<JSObject> targetObj = getMemberExprJSObject(member);

                    // Get and update property
                    check_obj_prop_access(member, targetObj.get(), key);
                    Value oldVal = targetObj->get(key);
                    Value newVal = toValue(oldVal).numberValue + 1;

                    check_obj_prop_access(member,
                                          targetObj.get(),
                                          key);

                    targetObj->set(key, newVal);
                    
                    return oldVal;
                    
                }

            }
            
            throw runtime_error("Invalid operand, the operand must be a variable.");
            
        }
            
            // --age
        case TokenType::DECREMENT: {

            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->right.get())) {
                
                R value = env->get(ident->name);
                
                env->assign(ident->name, (toValue(rvalue).numberValue - 1));
                
                return value;
                
            } else {
                // this is member expression.
                if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->right.get())) {
                    
                    // check of its static access
                    
                    R objectValue = member->object->accept(*this);

                    shared_ptr<JSObject> target_obj = nullptr;
                    shared_ptr<JSClass> js_class = nullptr;
                    Value oldVal;

                    if (std::get_if<std::shared_ptr<JSClass>>(&objectValue)) {
                        js_class = getMemberExprJSClass(member);
                    }

                    if (std::get_if<std::shared_ptr<JSObject>>(&objectValue)) {
                        target_obj = getMemberExprJSObject(member);
                    }
                    
                    if (holds_alternative<shared_ptr<Value>>(objectValue)) {
                        
                        shared_ptr<Value> current_value = get<shared_ptr<Value>>(objectValue);
                        
                        if (current_value->type == ValueType::OBJECT) {
                            
                            target_obj = current_value->objectValue;

                        }

                        if (current_value->type == ValueType::ARRAY) {
                            target_obj = current_value->arrayValue;
                        }

                    }
                    
                    if (holds_alternative<Value>(objectValue)) {
                        
                        Value current_value = get<Value>(objectValue);
                        
                        if (current_value.type == ValueType::OBJECT) {
                            
                            target_obj = current_value.objectValue;

                        }

                        if (current_value.type == ValueType::ARRAY) {
                            target_obj = current_value.arrayValue;
                        }

                    }

                    // Compute property key
                    std::string key;
                    if (member->computed) {
                        R comp = member->property->accept(*this);
                        key = toValue(comp).toString();
                    } else {
                        key = member->name.lexeme;
                    }
                    
                    
                    if (js_class) {
                        
                        oldVal = js_class->get(key, env->this_binding ? false : true);
                        Value newVal = toValue(oldVal).numberValue + 1;

                        js_class->set(key, newVal, env->this_binding ? false : true);
                        
                    }

                    // Get and update property
                    // check if key is public
                    if (target_obj) {
                        
                        oldVal = target_obj->get(key);
                        Value newVal = toValue(oldVal).numberValue - 1;
                        
                        check_obj_prop_access(member, target_obj.get(), key);
                        
                        target_obj->set(key, newVal);
                        
                    }
                    
                    return oldVal;
                    
                }
                
            }
            
            throw runtime_error("Invalid operand, the operand must be a variable.");

        }
            
            // +90
        case TokenType::ADD: {
            return (0 + toValue(rvalue).numberValue);

            // throw runtime_error("Invalid operand to perform +.");

        }
            // -89
        case TokenType::MINUS: {
            
            return (0 - toValue(rvalue).numberValue);

            // throw runtime_error("Invalid operand to perform -.");

        }
                        
        default:
            throw runtime_error("Unknown op found.");
            break;
    }
    
}

// user--, user.bar.baz++
R Interpreter::visitUpdate(UpdateExpression* expr) {
        
    R value = expr->argument->accept(*this);

    switch (expr->op.type) {
            
        case TokenType::INCREMENT: {
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
                R sum = toValue(value).numberValue + 1;
                env->assign(ident->name, sum);
                return sum;
            }
            
            // this is member expression.
            if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->argument.get())) {

                R objectValue = member->object->accept(*this);
                
                shared_ptr<JSObject> targetObj;
                shared_ptr<JSClass> targetKlass;
                
                Value newVal;

                if (std::get_if<std::shared_ptr<JSObject>>(&objectValue)) {
                    targetObj = getMemberExprJSObject(member);
                }

                if (std::get_if<std::shared_ptr<JSClass>>(&objectValue)) {
                    targetKlass = getMemberExprJSClass(member);
                }

                // Compute property key
                std::string key;
                if (member->computed) {
                    R comp = member->property->accept(*this);
                    key = toValue(comp).toString();
                } else {
                    key = member->name.lexeme;
                }
                
                if (holds_alternative<shared_ptr<Value>>(objectValue)) {
                    
                    shared_ptr<Value> current_value = get<shared_ptr<Value>>(objectValue);
                    
                    if (current_value->type == ValueType::OBJECT) {
                        
                        targetObj = current_value->objectValue;

                    }

                    if (current_value->type == ValueType::ARRAY) {
                        targetObj = current_value->arrayValue;
                    }

                }
                
                if (holds_alternative<Value>(objectValue)) {
                    
                    Value current_value = get<Value>(objectValue);
                    
                    if (current_value.type == ValueType::OBJECT) {
                        
                        targetObj = current_value.objectValue;

                    }

                    if (current_value.type == ValueType::ARRAY) {
                        targetObj = current_value.arrayValue;
                    }

                }

                // Get and update property
                if (targetObj) {
                    Value oldVal = targetObj->get(key);
                    newVal = toValue(oldVal).numberValue + 1;
                    
                    check_obj_prop_access(member,
                                          targetObj.get(),
                                          key);
                    
                    targetObj->set(key, newVal);
                }
                
                if (targetKlass) {
                    
                    Value oldVal = targetKlass->get(key, env->this_binding ? false : true);
                    newVal = toValue(oldVal).numberValue + 1;
                                        
                    targetKlass->set(key, newVal, env->this_binding ? false : true);

                }
                
                return newVal;

            }
            
            return monostate();
            
        }
            
        case TokenType::DECREMENT: {
            
            if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->argument.get())) {
                R sum = toValue(value).numberValue - 1;
                env->assign(ident->name, sum);
                return sum;
            }

            // this is member expression.
            if (MemberExpression* member = dynamic_cast<MemberExpression*>(expr->argument.get())) {

                R objectValue = member->object->accept(*this);
                
                shared_ptr<JSObject> targetObj;
                shared_ptr<JSClass> targetKlass;
                
                Value newVal;

                if (std::get_if<std::shared_ptr<JSObject>>(&objectValue)) {
                    targetObj = getMemberExprJSObject(member);
                }

                if (std::get_if<std::shared_ptr<JSClass>>(&objectValue)) {
                    targetKlass = getMemberExprJSClass(member);
                }

                // Compute property key
                std::string key;
                if (member->computed) {
                    R comp = member->property->accept(*this);
                    key = toValue(comp).toString();
                } else {
                    key = member->name.lexeme;
                }
                
                if (holds_alternative<shared_ptr<Value>>(objectValue)) {
                    
                    shared_ptr<Value> current_value = get<shared_ptr<Value>>(objectValue);
                    
                    if (current_value->type == ValueType::OBJECT) {
                        
                        targetObj = current_value->objectValue;

                    }

                    if (current_value->type == ValueType::ARRAY) {
                        targetObj = current_value->arrayValue;
                    }

                }

                if (holds_alternative<Value>(objectValue)) {
                    
                    Value current_value = get<Value>(objectValue);
                    
                    if (current_value.type == ValueType::OBJECT) {
                        
                        targetObj = current_value.objectValue;

                    }

                    if (current_value.type == ValueType::ARRAY) {
                        targetObj = current_value.arrayValue;
                    }

                }
                
                // Get and update property
                if (targetObj) {
                    Value oldVal = targetObj->get(key);
                    newVal = toValue(oldVal).numberValue - 1;
                    
                    check_obj_prop_access(member, targetObj.get(), key);
                    
                    targetObj->set(key, newVal);
                    
                }
                
                if (targetKlass) {
                    
                    Value oldVal = targetKlass->get(key, env->this_binding ? false : true);
                    newVal = toValue(oldVal).numberValue - 1;
                                        
                    targetKlass->set(key, newVal, env->this_binding ? false : true);

                }
                
                return newVal;

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
                property_name = toValue(property_value).toString();
                // throw runtime_error("The computed property is not supported. It must be a string.");
            } else {
                
                property_name = get<string>(property_value);
                
            }
            
        } else {
            
            // .
            property_name = expr->name.lexeme;
            
        }

        js_object_instance = get<shared_ptr<JSObject>>(object_value);

        check_obj_prop_access(expr, js_object_instance.get(), property_name);
        return_value = js_object_instance->get(property_name);

    }
    else if (holds_alternative<shared_ptr<JSClass>>(object_value)) {
        // we have a static access
        // User.age or Math.PI
        if (expr->computed) {
            
            // []
            R property_value = expr->property->accept(*this);
            
            if (!holds_alternative<string>(property_value)) {
                property_name = toValue(property_value).toString();
                // throw runtime_error("The computed property is not supported. It must be a string.");
            } else {
                property_name = get<string>(property_value);
            }
            
        } else {
            
            // .
            property_name = expr->name.lexeme;
            
        }
        
        // get the property from the JSClass
        shared_ptr<JSClass> js_class = get<shared_ptr<JSClass>>(object_value);
        return_value = js_class->get(property_name, env->this_binding ? false : true);
        
        return return_value;

    }
    else if (holds_alternative<shared_ptr<JSArray>>(object_value)) {
        
        if (expr->computed) {
            
            // []
            R property_value = expr->property->accept(*this);
                        
            property_name = toValue(property_value).toString();
            
        } else {
            
            // .
            property_name = expr->name.lexeme;
            
        }
        
        js_object_instance = get<shared_ptr<JSArray>>(object_value);

        check_obj_prop_access(expr, js_object_instance.get(), property_name);
        return_value = js_object_instance->get(property_name);

    }
    else if (holds_alternative<shared_ptr<Value>>(object_value)) {
        
        shared_ptr<Value> value = get<shared_ptr<Value>>(object_value);
        
        if (value->type == ValueType::ARRAY) {
            
            if (expr->computed) {
                
                // []
                R property_value = expr->property->accept(*this);
                            
                property_name = toValue(property_value).toString();
                
            } else {
                
                // .
                property_name = expr->name.lexeme;
                
            }
            
            js_object_instance = value->arrayValue;

            check_obj_prop_access(expr, js_object_instance.get(), property_name);
            return_value = js_object_instance->get(property_name);

        }
        
        if (value->type == ValueType::OBJECT) {
            
            if (expr->computed) {
                
                // []
                R property_value = expr->property->accept(*this);
                            
                property_name = toValue(property_value).toString();
                
            } else {
                
                // .
                property_name = expr->name.lexeme;
                
            }
            
            js_object_instance = value->objectValue;

            check_obj_prop_access(expr, js_object_instance.get(), property_name);
            return_value = js_object_instance->get(property_name);

        }

    }
    else if (holds_alternative<Value>(object_value)) {
        
        Value value = get<Value>(object_value);
        
        if (value.type == ValueType::ARRAY) {
            
            if (expr->computed) {
                
                // []
                R property_value = expr->property->accept(*this);
                            
                property_name = toValue(property_value).toString();
                
            } else {
                
                // .
                property_name = expr->name.lexeme;
                
            }
            
            js_object_instance = value.arrayValue;

            check_obj_prop_access(expr, js_object_instance.get(), property_name);
            return_value = js_object_instance->get(property_name);

        }
        
        if (value.type == ValueType::OBJECT) {
            
            if (expr->computed) {
                
                // []
                R property_value = expr->property->accept(*this);
                            
                property_name = toValue(property_value).toString();
                
            } else {
                
                // .
                property_name = expr->name.lexeme;
                
            }
            
            js_object_instance = value.objectValue;

            check_obj_prop_access(expr, js_object_instance.get(), property_name);
            return_value = js_object_instance->get(property_name);

        }

    }
    else {
        
        if (!holds_alternative<string>(object_value)) {
            throw runtime_error("The object name must be an identifier.");
        }
        
        // TODO: add support for "this" and "super" access.
        
        string object_name = get<string>(object_value);
        
        if (expr->computed) {
            
            // []
            R property_value = expr->property->accept(*this);
            
            if (!holds_alternative<string>(property_value)) {
                property_name = toValue(property_value).toString();
                // throw runtime_error("The computed property is not supported. It must be a string.");
            } else {
                
                property_name = get<string>(property_value);
            }
            
        } else {
            
            // .
            property_name = expr->name.lexeme;
            
        }
        
        R object_instance = env->get(object_name);
        js_object_instance = get<shared_ptr<JSObject>>(object_instance);
        
        // check if property_name is public
        check_obj_prop_access(expr, js_object_instance.get(), property_name);
        
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

    // TODO: do we need to add support for native classes?
    
    // here, the object is created from user-defined class
    auto object = make_shared<JSObject>();
    
    if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->callee.get())) {

        const string class_name = ident->name;
        
        // get from env
        R value = env->get(class_name);
        
        if (!holds_alternative<std::shared_ptr<JSClass>>(value)) {
            throw runtime_error("New keyword should always instantiate a class. " + to_string(expr->token.line));
            return monostate();
        }
                
        std::shared_ptr<JSClass> new_class = std::get<std::shared_ptr<JSClass>>(value);

        if (new_class->is_native == true) {
            return new_class->construct();
        }

        object->setClass(new_class);
        
        // set properties from class value to object.
        // remember modifiers
        
        for (auto& field : new_class->fields) {

            vector<string> field_modifiers;
            
            for (auto& modifier : field.second->modifiers) {

                string current_modifier = get<string>(modifier->accept(*this));
                field_modifiers.push_back(current_modifier);
                
            }

            // property is a Statement: VariableStatement
            if (VariableStatement* variable = dynamic_cast<VariableStatement*>(field.second->property.get())) {

                string kind = variable->kind;

                for (auto& declarator : variable->declarations) {
                    
                    // I think we need to set NULL if the init is nullptr.
                    if (declarator.init == nullptr) {
                        // throw runtime_error("Missing initializer in const declaration: " + declarator.id);
                        object->set(declarator.id,
                                    Value::nullVal(),
                                    kind,
                                    field_modifiers);
                    }
                                        
                    if (declarator.init) {
                        
                        R value = declarator.init->accept(*this);

                        // TODO: fix
                        object->set(declarator.id, toValue(value), kind, field_modifiers);

                    }
                }

            }

        }
                
        // hold constructor
        MethodDefinition* constructor = nullptr;
        
        // copy methods
        for (auto& method : new_class->methods) {
            
            vector<string> field_modifiers;
            
            for (auto& modifier : method.second->modifiers) {

                string current_modifier = get<string>(modifier->accept(*this));
                field_modifiers.push_back(current_modifier);
                
            }

            if (method.first == "constructor") {
                constructor = method.second.get();
            }
            
            string kind = VAR;
            
            for (auto& modifier : method.second->modifiers) {
                
                Value val = toValue(modifier->accept(*this));
                
                if (val.stringValue == LET) {
                    kind = LET;
                }
                
                if (val.stringValue == CONST) {
                    kind = CONST;
                }
                
            }
            
            object->set(method.first,
                        Value::method(object),
                        kind,
                        field_modifiers);
            
        }
        
        // create a jsobject from supercalss and assigne to parent_object
        if (new_class->superClass != nullptr) {
            object->parent_object = createJSObject(new_class->superClass);
            object->parent_class = new_class->superClass;
        }
                    
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
    
    // here, it is literal object
    auto object = make_shared<JSObject>();
    object->set_as_object_literal();
    
    for (auto& prop : expr->props) {
        object->set(prop.first.lexeme, toValue(prop.second->accept(*this)), "VAR", {});
    }

    return object;
    
}

R Interpreter::visitSuper(SuperExpression* expr) {
        
    return env->this_binding->parent_object;
    
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

R Interpreter::visitArrowFunction(ArrowFunction* expr) {
    
    // Capture the current environment and lexical `this`
    Env* closure = env;
    auto lexicalThis = env->this_binding;
    auto intr = this;

    // Return a callable Value (assuming Value supports callable lambdas)
    return Value::function([closure, lexicalThis, expr, intr, this](vector<Value> args) mutable -> Value {

        auto promise = std::make_shared<Promise>();

        auto callback_func = [closure, lexicalThis, expr, intr, promise](vector<Value> args) -> Value {
            
            // Create a new local environment inheriting from the closure
            Env* localEnv = new Env(closure);
            localEnv->this_binding = lexicalThis; // Lexical 'this' (as in arrow functions)
            
            auto prevEnv = intr->env;
            intr->env = localEnv;
            
            // Bind parameters to arguments
            if (expr->parameters != nullptr) {
                
                if (SequenceExpression* seq = dynamic_cast<SequenceExpression*>(expr->parameters.get())) {
                    
                    for (size_t i = 0; i < seq->expressions.size(); ++i) {
                        
                        Value paramValue = (i < args.size()) ? args[i] : Value::undefined();
                        
                        Expression* param_expr = seq->expressions[i].get();
                        string paramName;
                        
                        if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(param_expr)) {
                            paramName = ident->token.lexeme;
                        } else if (VariableStatement* variable = dynamic_cast<VariableStatement*>(param_expr)) {
                            // TODO: fix var decl
                            paramName = variable->declarations[0].id;
                            
                            // use the init value if no value is passed for the argument.
                            if (i >= args.size()) {
                                paramValue = toValue(variable->declarations[0].init->accept(*intr));
                            }
                            
                        } else if (RestParameter* rest_expr = dynamic_cast<RestParameter*>(param_expr)) {
                            paramName = toValue(param_expr->accept(*intr)).stringValue;
                            
                            auto array = make_shared<JSArray>();
                            int arr_index = 0;
                            
                            for (size_t rest_i = i; rest_i < args.size(); rest_i++) {
                                array->setIndex(arr_index, args[rest_i]);
                                arr_index++;
                            }
                            
                            localEnv->set_var(paramName, array);
                            break; // we break because rest should be the last param.
                            
                        } else {
                            
                            if (BinaryExpression* bin_expr = dynamic_cast<BinaryExpression*>(param_expr)) {
                                auto left = dynamic_cast<IdentifierExpression*>(bin_expr->left.get());
                                
                                if (left) {
                                    paramName = left->token.lexeme;
                                    if (paramValue.type == ValueType::UNDEFINED || paramValue.type == ValueType::NULLTYPE) {
                                        paramValue = toValue(bin_expr->right->accept(*intr));
                                    }
                                }
                            }
                            // param_expr->accept(*intr);
                        }
                        
                        localEnv->set_var(paramName, paramValue);
                        
                    }
                    
                }
                
                if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(expr->parameters.get())) {
                    localEnv->set_var(ident->token.lexeme, args.size() > 0 ? args[0] : Value::undefined());
                }
                
            }
            
            // Evaluate the body
            try {
                
                if (expr->exprBody) {
                    
                    R value = expr->exprBody->accept(*intr);
                    intr->env = prevEnv;  // restore
                    return toValue(value);
                    
                } else if (expr->stmtBody) {
                    
                    expr->stmtBody->accept(*intr);
                    
                }

                promise->resolve(Value::undefined());

                intr->env = prevEnv;  // restore before returning
                return Value::undefined();
                
            } catch (const ReturnException& r) {
                
                promise->resolve(toValue(r.value));

                intr->env = prevEnv;  // restore before throwing
                return toValue(r.value);
                
            } catch (std::exception& e) {
                
                intr->env = prevEnv;  // restore before throwing

                // If it throws, reject the promise
                promise->reject(Value::str(e.what()));
                return Value::str(e.what());
                
            }
            
        };
        
        if (expr->is_async) {
            
            event_loop->post(callback_func, args);
            
            return Value::promise(promise);

        }
        
        return callback_func(args);

    });
    
}

shared_ptr<JSObject> Interpreter::createJSObject(shared_ptr<JSClass> klass) {
        
    shared_ptr<JSObject> object = make_shared<JSObject>();
    object->setClass(klass);

    // add all props from this_binding to parent class.
    for (auto& field : klass->fields) {

        // property is a Statement: VariableStatement
        if (VariableStatement* variable = dynamic_cast<VariableStatement*>(field.second->property.get())) {

            string kind = variable->kind;

            for (auto& declarator : variable->declarations) {
                
                vector<string> field_modifiers;
                
                for (auto& modifier : field.second->modifiers) {

                    string current_modifier = get<string>(modifier->accept(*this));
                    field_modifiers.push_back(current_modifier);
                    
                }

                if (declarator.init == nullptr) {
                    
                    // throw runtime_error("Missing initializer in const declaration: " + declarator.id);
                    object->set(declarator.id, Value::nullVal(), kind, field_modifiers);

                }

                if (declarator.init) {
                    
                    R value = declarator.init->accept(*this);

                    // TODO: fix
                    object->set(declarator.id, toValue(value), kind, field_modifiers);

                }
            }

        }

    }
    
    // copy methods
    for (auto& method : klass->methods) {
        
        vector<string> field_modifiers;
        
        string kind = VAR;
        
        for (auto& modifier : method.second->modifiers) {
            
            Value val = toValue(modifier->accept(*this));
            
            if (val.stringValue == LET) {
                kind = LET;
            }
            if (val.stringValue == CONST) {
                kind = CONST;
            }
            
            string current_modifier = get<string>(modifier->accept(*this));
            field_modifiers.push_back(current_modifier);
            
        }

        object->set(method.first, Value::method(object), kind, field_modifiers);
        
    }
    
    // create a jsobject from superclass and assign to parent_object
    if (klass->superClass != nullptr) {
        
        object->parent_object = createJSObject(klass->superClass);
        object->parent_class = klass->superClass;
        
    }

    return object;
    
}

shared_ptr<JSClass> Interpreter::getMemberExprJSClass(MemberExpression* member) {

    R object_value = member->object->accept(*this);
    shared_ptr<JSClass> klass = get<shared_ptr<JSClass>>(object_value);
    
    if (klass) {
        return klass;
    }
    
    return nullptr;

}

shared_ptr<JSObject> Interpreter::getMemberExprJSObject(MemberExpression* member) {
    
    // Evaluate object (user, this, super)
    R objectValue = member->object->accept(*this);
    shared_ptr<JSObject> targetObj;
    
    // Handle "this"
    if (auto* thisExpr = dynamic_cast<ThisExpression*>(member->object.get())) {
        targetObj = env->this_binding;
    }
    // Handle "super"
    else if (auto* superExpr = dynamic_cast<SuperExpression*>(member->object.get())) {
        targetObj = env->this_binding->parent_object;
    }
    // Generic object
    else {
        targetObj = get<shared_ptr<JSObject>>(objectValue);
    }
    
    if (holds_alternative<shared_ptr<Value>>(objectValue)) {
        
        shared_ptr<Value> current_value = get<shared_ptr<Value>>(objectValue);
        
        if (current_value->type == ValueType::OBJECT) {
            
            targetObj = current_value->objectValue;

        }

        if (current_value->type == ValueType::ARRAY) {
            targetObj = current_value->arrayValue;
        }

    }
    
    if (holds_alternative<Value>(objectValue)) {
        
        Value current_value = get<Value>(objectValue);
        
        if (current_value.type == ValueType::OBJECT) {
            
            targetObj = current_value.objectValue;

        }

        if (current_value.type == ValueType::ARRAY) {
            targetObj = current_value.arrayValue;
        }

    }

        
    return targetObj;
    
}

R Interpreter::visitTemplateLiteral(TemplateLiteral* expr) {
    
    string result;

    size_t qsize = expr->quasis.size();
    size_t esize = expr->expressions.size();

    // Append quasis and interleave expressions
    for (size_t i = 0; i < qsize; i++) {
        // add the quasi
        result += toValue(expr->quasis[i]->accept(*this)).toString();

        // if there’s a matching expression, evaluate it
        if (i < esize) {
            Value val = toValue(expr->expressions[i]->accept(*this));
            result += val.toString();
        }
    }

    return result;
    
}

bool Interpreter::check_obj_prop_access(MemberExpression* member,
                                        JSObject* js_object,
                                        const string& key) {
    
    // if member expression is a this, then we don't check for access.
    if (member != nullptr) {
        
        ThisExpression* is_object_this = dynamic_cast<ThisExpression*>(member->object.get());
        SuperExpression* is_object_super = dynamic_cast<SuperExpression*>(member->object.get());
        
        if (is_object_this || is_object_super) {
            return true;
        }
        
    }
    
    // if env->this_binding is null then
    
    shared_ptr<JSClass> klass = js_object->getKlass();
    
    if (klass) {
        
        for (auto& field : klass->fields) {
            if (field.first == key) {
                // get the modifiers
                for (auto& modifier : field.second->modifiers) {
                    string modifier_string = get<string>(modifier->accept(*this));
                    
                    if (modifier_string == "private") {
                        throw runtime_error("Attempting to access a private property from outside of its class. " + field.first);
                        return false;
                    }
                }
            }
        }
        
        for (auto& method : klass->methods) {
            if (method.first == key) {
                // get the modifiers
                for (auto& modifier : method.second->modifiers) {
                    string modifier_string = get<string>(modifier->accept(*this));
                    
                    if (modifier_string == "private") {
                        throw runtime_error("Attempting to access a private property from outside of its class. " + method.first);
                        return false;
                    }
                }
            }
        }
        
    }
    
    // check in methods
    
    if (js_object->parent_object) {
        return check_obj_prop_access(nullptr, js_object->parent_object.get(), key);
    }
    
    return true;
    
}

R Interpreter::visitRestParameter(RestParameter *expr) {
    return expr->token.lexeme;
}

R Interpreter::visitImportDeclaration(ImportDeclaration* stmt) {
    namespace fs = std::filesystem;

    // dir of the file that contained the import
    fs::path baseDir = fs::path(stmt->sourceFile).parent_path();

    std::string raw = stmt->path.lexeme;
    if (!raw.empty() && raw.front() == '"' && raw.back() == '"') {
        raw = raw.substr(1, raw.size() - 2);
    }

    fs::path resolved = fs::weakly_canonical(baseDir / raw);

    std::string source = read_file(resolved.string());

    Scanner scanner(source);
    auto tokens = scanner.getTokens();

    // ✅ pass the resolved file path into the parser
    Parser parser(tokens);
    parser.sourceFile = resolved.string();
    auto ast = parser.parse();

    this->execute(std::move(ast));
    return true;
}

R Interpreter::visitFunctionExpression(FunctionExpression* expr) {
    
    // Capture the current environment and lexical `this`
    Env* closure = env;
    auto lexicalThis = env->this_binding;
    auto intr = this;
    
    // Return a callable Value (assuming Value supports callable lambdas)
    return Value::function([closure, lexicalThis, expr, intr, this](vector<Value> args) mutable -> Value {
        
        auto promise = std::make_shared<Promise>();
                
        auto callback_func = [closure, lexicalThis, expr, intr, promise](vector<Value> args) -> Value {
            
            // Create a new local environment inheriting from the closure
            Env* localEnv = new Env(closure);
            localEnv->this_binding = lexicalThis; // Lexical 'this' (as in arrow functions)
            auto prevEnv = intr->env;
            intr->env = localEnv;
            
            // Bind parameters to arguments
            
            for (size_t i = 0; i < expr->params.size(); ++i) {
                
                Value paramValue = (i < args.size()) ? args[i] : Value::undefined();
                
                Expression* param_expr = expr->params[i].get();
                string paramName;
                
                if (IdentifierExpression* ident = dynamic_cast<IdentifierExpression*>(param_expr)) {
                    paramName = ident->token.lexeme;
                } else if (VariableStatement* variable = dynamic_cast<VariableStatement*>(param_expr)) {
                    // TODO: fix var decl
                    paramName = variable->declarations[0].id;
                    
                    // use the init value if no value is passed for the argument.
                    if (i >= args.size()) {
                        paramValue = toValue(variable->declarations[0].init->accept(*intr));
                    }
                    
                } else if (RestParameter* rest_expr = dynamic_cast<RestParameter*>(param_expr)) {
                    paramName = toValue(param_expr->accept(*intr)).stringValue;
                    
                    auto array = make_shared<JSArray>();
                    int arr_index = 0;
                    
                    for (size_t rest_i = i; rest_i < args.size(); rest_i++) {
                        array->setIndex(arr_index, args[rest_i]);
                        arr_index++;
                    }
                    
                    localEnv->set_var(paramName, array);
                    break; // we break because rest should be the last param.
                    
                } else {
                    if (BinaryExpression* bin_expr = dynamic_cast<BinaryExpression*>(param_expr)) {
                        auto left = dynamic_cast<IdentifierExpression*>(bin_expr->left.get());
                        
                        if (left) {
                            paramName = left->token.lexeme;
                            if (paramValue.type == ValueType::UNDEFINED || paramValue.type == ValueType::NULLTYPE) {
                                paramValue = toValue(bin_expr->right->accept(*intr));
                            }
                        }
                    }
                    // param_expr->accept(*intr);
                }
                
                localEnv->set_var(paramName, paramValue);
                
            }
            
            // Evaluate the body
            try {
                
                if (expr->body) {
                    
                    expr->body->accept(*intr);
                    
                }
                
                intr->env = prevEnv;  // restore before returning
                promise->resolve(Value::undefined());
                return Value::undefined();
                
            } catch (const ReturnException& r) {
                
                intr->env = prevEnv;  // restore before throwing
                promise->resolve(toValue(r.value));
                return toValue(r.value);
                
            } catch (std::exception& e) {

                intr->env = prevEnv;  // restore before throwing

                // If it throws, reject the promise
                promise->reject(Value::str(e.what()));
                return Value::str(e.what());
                
            }
            
        };
        
        if (expr->is_async) {
            // what happens here?
            // it pushes callback to Event Loop queue.
            // what happens inside the callback?
            
            event_loop->post(callback_func, args);
            
            return Value::promise(promise);

        }
        
        return callback_func(args);
        
    });
    
}

R Interpreter::visitClassExpression(ClassExpression* expr) {
    
    auto js_class = create_js_class(std::move(expr->fields),
                                    std::move(expr->body),
                                    expr->superClass.get());

    return js_class;

}

R Interpreter::visitNullKeyword(NullKeyword* visitor) {
    return Value::nullVal();
}

R Interpreter::visitUndefinedKeyword(UndefinedKeyword* visitor) {
    Value v;
    return v;
}

R Interpreter::visitAwaitExpression(AwaitExpression* expr) {
    // here, we are inside the event loop
    // it waits for an async to return
    return expr->inner->accept(*this);
}

//R Interpreter::visitTaggedTemplate(TaggedTemplateExpression* expr) {
//    // 1. Evaluate the tag function
//    Value callee = evaluate(expr->tag.get());
//    if (!callee.isCallable()) {
//        throw RuntimeError("Tag is not a function");
//    }
//
//    // 2. Build cooked & raw strings
//    vector<Value> cooked;
//    vector<Value> raw;
//
//    for (auto& quasi : expr->quasi->quasis) {
//        cooked.push_back(Value(quasi->value)); // cooked string
//        raw.push_back(Value(quasi->rawValue)); // if you stored raw text separately
//    }
//
//    // 3. Create the template object
//    //    In JS it's an array with a `raw` property
//    Object* templateObj = new Object();
//    for (size_t i = 0; i < cooked.size(); i++) {
//        templateObj->set(i, cooked[i]);
//    }
//    templateObj->set("raw", Value(raw));
//
//    // 4. Build call arguments
//    vector<Value> args;
//    args.push_back(Value(templateObj));
//
//    for (auto& e : expr->quasi->expressions) {
//        args.push_back(evaluate(e.get()));
//    }
//
//    // 5. Call the function
//    return callee.call(args);
//}
