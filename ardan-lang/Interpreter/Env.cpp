//
//  Environment.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 25/08/2025.
//

#include "Env.h"
#include <sstream>
#include <typeinfo>

Env::Env(Env* parent) : parent(parent) {}

Env::~Env() {}

R Env::getValue(const string& key) {
    auto it = variables.find(key);
    if (it != variables.end()) {
        return it->second;
    }

    // search in let.
    auto it_let = let_variables.find(key);
    if (it_let != let_variables.end()) {
        return it_let->second;
    }
    
    // search in const.
    auto it_const = const_variables.find(key);
    if (it_const != const_variables.end()) {
        return it_const->second;
    }

    if (parent) return parent->getValue(key);
    throw runtime_error("Undefined variable: " + key);
}

R Env::get_var_value(const string& key) {

    auto it = variables.find(key);
    if (it != variables.end()) {
        return it->second;
    }
    
    if (parent) return parent->get_var_value(key);
    throw runtime_error("Undefined variable: " + key);
}

R Env::get_let_value(const string& key) {
    
    auto it_let = let_variables.find(key);
    if (it_let != let_variables.end()) {
        return it_let->second;
    }
    
    if (parent) return parent->get_let_value(key);
    throw runtime_error("Undefined variable: " + key);
}

R Env::get_const_value(const string& key) {
    
    auto it_const = const_variables.find(key);
    if (it_const != const_variables.end()) {
        return it_const->second;
    }
    
    if (parent) return parent->get_const_value(key);
    throw runtime_error("Undefined variable: " + key);
}

R Env::get(const string& key) {
    return getValue(key);
}

void Env::set_var(const string& key, R value) {
    variables[key] = std::move(value);
}

void Env::set_let(const string& key, R value) {
    let_variables[key] = std::move(value);
}

void Env::set_const(const string& key, R value) {
    const_variables[key] = std::move(value);
}

bool Env::is_const_key_set(const string& key) {
    
    if (const_variables.find(key) != const_variables.end()) {
        // key exists
        return true;
    }
    
    if (parent) {
        return parent->is_const_key_set(key);
    }
    
    // key does not exist
    return false;
}

bool Env::is_var_key_set(const string& key) {
    
    if (variables.find(key) != variables.end()) {
        // key exists
        return true;
    }
    
    if (parent) {
        return parent->is_var_key_set(key);
    }
    
    // key does not exist
    return false;
}

bool Env::is_let_key_set(const string& key) {
    
    if (let_variables.find(key) != let_variables.end()) {
        // key exists
        return true;
    }
    
    if (parent) {
        return parent->is_let_key_set(key);
    }

    // key does not exist
    return false;
}

void Env::assign(const string& key, R value) {
    // 1. Look in this scope's var
    auto it_var = variables.find(key);
    if (it_var != variables.end()) {
        variables[key] = std::move(value);
        return;
    }

    // 2. Look in this scope's let
    auto it_let = let_variables.find(key);
    if (it_let != let_variables.end()) {
        let_variables[key] = std::move(value);
        return;
    }

    // 3. Look in this scope's const
    auto it_const = const_variables.find(key);
    if (it_const != const_variables.end()) {
        throw runtime_error("Cannot reassign to const variable: " + key);
    }

    // 4. If not found in current scope, walk up the parent chain
    if (parent) {
        parent->assign(key, std::move(value));
        return;
    }

    // 5. Not found anywhere → define as var in current scope (default behavior)
    variables[key] = std::move(value);
}

void Env::setStackValue(const string& key, R value) {
    stack[key] = std::move(value);
}

void Env::setFunctionDeclarations(
    const string& name,
    vector<unique_ptr<Expression>> argList,
    unique_ptr<Statement> bodyList
) {
    body[name] = std::move(bodyList);
    params[name] = std::move(argList);
}

Statement* Env::getFunctionBody(const string& name) {
    
    auto it = body.find(name);
    if (it != body.end() && it->second) {
        return it->second.get();
    }
    
    if (parent) return parent->getFunctionBody(name);

    return nullptr;
}

//vector<Expression*> Env::getFunctionParams(const string& name) {
//    vector<Expression*> result;
//    auto it = params.find(name);
//    if (it != params.end()) {
//        for (auto& expr : it->second) {
//            result.push_back(expr.get());
//        }
//    }
//    
//    if (parent) return parent->getFunctionParams(name);
//
//    return result;
//}

vector<Expression*> Env::getFunctionParams(const string& name) {
    // Look in current env first
    auto it = params.find(name);
    if (it != params.end()) {
        vector<Expression*> result;
        for (auto& expr : it->second) {
            result.push_back(expr.get());
        }
        return result; // <-- stop here if found
    }

    // Otherwise, check parent
    if (parent) {
        return parent->getFunctionParams(name);
    }

    // Not found at all
    return {};
}

unordered_map<string, R> Env::getStack() {
    return stack;
}

void Env::clearStack() {
    stack.clear();
    unordered_map<string, R>().swap(stack);
}


void Env::debugPrint() const {
    cout << "\n=== Environment Debug ===\n";

    cout << "Vars:\n";
    for (const auto& [k, v] : variables) {
        cout << "  " << k << " = ";
        printValue(v);
        cout << "\n";
    }

    cout << "Lets:\n";
    for (const auto& [k, v] : let_variables) {
        cout << "  " << k << " = ";
        printValue(v);
        cout << "\n";
    }

    cout << "Consts:\n";
    for (const auto& [k, v] : const_variables) {
        cout << "  " << k << " = ";
        printValue(v);
        cout << "\n";
    }

    cout << "Stack:\n";
    for (const auto& [k, v] : stack) {
        cout << "  " << k << " = ";
        printValue(v);
        cout << "\n";
    }

    cout << "Functions:\n";
    for (const auto& [name, paramList] : params) {
        cout << "  " << name << "(";
        for (size_t i = 0; i < paramList.size(); i++) {
            Expression* expr = paramList[i].get();
            // if expr is IdentifierExpression, print its name; otherwise print "param"
            if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr)) {
                cout << idExpr->name;
            } else {
                cout << "param";
            }
            if (i + 1 < paramList.size()) cout << ", ";
        }
        cout << ") { ... }\n";
    }

    if (parent) {
        cout << "\n-- Parent Env --\n";
        parent->debugPrint();
    }

    cout << "========================\n";
}
