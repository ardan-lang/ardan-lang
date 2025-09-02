//
//  Environment.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 25/08/2025.
//

#ifndef Environment_hpp
#define Environment_hpp

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include "../Statements/Statements.hpp"
#include "../Expression/Expression.hpp"

using namespace std;

class JSObject;

class Env {
public:
    Env(Env* parent = nullptr) : parent(parent) {}

    R getValue(const string& key) {
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
    
    R get(const string& key) {
        return getValue(key);
    }

    void set_var(const string& key, R value) {
        variables[key] = std::move(value);
    }
    
    void set_let(const string& key, R value) {
        let_variables[key] = std::move(value);
    }

    void set_const(const string& key, R value) {
        const_variables[key] = std::move(value);
    }
    
    bool is_const_key_set(string& key) {
        
        if (const_variables.find(key) != const_variables.end()) {
            // key exists
            return true;
        }
        
        // key does not exist
        return false;
    }

    void assign_var(const string& key, R value) {
        set_var(key, value);
    }

    void setStackValue(const string& key, R value) {
        stack[key] = std::move(value);
    }

    void setFunctionDeclarations(
        const string& name,
        vector<unique_ptr<Expression>> argList,
        unique_ptr<Statement> bodyList
    ) {
        body[name] = std::move(bodyList);
        params[name] = std::move(argList);
    }

    Statement* getFunctionBody(const string& name) {
        auto it = body.find(name);
        if (it != body.end() && it->second) {
            return it->second.get();
        }
        return nullptr;
    }

    vector<Expression*> getFunctionParams(const string& name) {
        vector<Expression*> result;
        auto it = params.find(name);
        if (it != params.end()) {
            for (auto& expr : it->second) {
                result.push_back(expr.get());
            }
        }
        return result;
    }
    
    unordered_map<string, R> getStack() {
        return stack;
    }
    
    void clearStack() {
        stack = {};
    }
        
    shared_ptr<JSObject> this_binding;
    shared_ptr<JSObject> global_object = make_shared<JSObject>();

private:
    unordered_map<string, R> variables = {
        {"print", "print"}
    };
    unordered_map<string, R> let_variables = {};
    unordered_map<string, R> const_variables = {};

    unordered_map<string, R> stack = {};

    unordered_map<string, vector<unique_ptr<Expression>>> params;
    unordered_map<string, unique_ptr<Statement>> body;

    Env* parent;
};

#endif /* Environment_hpp */
