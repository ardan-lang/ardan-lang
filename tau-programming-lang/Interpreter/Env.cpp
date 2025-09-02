//
//  Environment.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 25/08/2025.
//

#include "Env.h"

Env::Env(Env* parent) : parent(parent) {}

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
    
    // key does not exist
    return false;
}

bool Env::is_var_key_set(const string& key) {
    
    if (variables.find(key) != variables.end()) {
        // key exists
        return true;
    }
    
    // key does not exist
    return false;
}

bool Env::is_let_key_set(const string& key) {
    
    if (let_variables.find(key) != let_variables.end()) {
        // key exists
        return true;
    }
    
    // key does not exist
    return false;
}

void Env::assign(const string& key, R value) {
    
    // check where the key is located
    
    // is var?
    
    if (is_var_key_set(key)) {
        set_var(key, value);
    } else if (is_let_key_set(key)) {
        set_let(key, value);
    } else {
        if (!is_const_key_set(key)) {
            set_const(key, value);
        } else {
            throw runtime_error("Cannot assign a value to a const variable after it has been initially assigned a value.");
        }
    }
    
    
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
    return nullptr;
}

vector<Expression*> Env::getFunctionParams(const string& name) {
    vector<Expression*> result;
    auto it = params.find(name);
    if (it != params.end()) {
        for (auto& expr : it->second) {
            result.push_back(expr.get());
        }
    }
    return result;
}

unordered_map<string, R> Env::getStack() {
    return stack;
}

void Env::clearStack() {
    stack = {};
}
