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
inline void printValue(const R& value);

class Env {

public:
    
    Env(Env* parent = nullptr);
    ~Env();

    R getValue(const string& key);
    
    R get(const string& key);

    void set_var(const string& key, R value);
    
    void set_let(const string& key, R value);

    void set_const(const string& key, R value) ;
    
    R get_var_value(const string& key);

    R get_let_value(const string& key);

    R get_const_value(const string& key);

    bool is_const_key_set(const string& key);
    
    bool is_var_key_set(const string& key);
    
    bool is_let_key_set(const string& key);

    void assign(const string& key, R value);

    void setStackValue(const string& key, R value);

    void setFunctionDeclarations(const string& name, vector<unique_ptr<Expression>> argList, unique_ptr<Statement> bodyList);

    Statement* getFunctionBody(const string& name);

    vector<Expression*> getFunctionParams(const string& name);
    
    unordered_map<string, R> getStack();
    
    void clearStack();
        
    shared_ptr<JSObject> this_binding;
    shared_ptr<JSObject> global_object = make_shared<JSObject>();
    void debugPrint() const;

private:
    unordered_map<string, R> variables = {};
    unordered_map<string, R> let_variables = {};
    unordered_map<string, R> const_variables = {};

    unordered_map<string, R> stack = {};

    unordered_map<string, vector<unique_ptr<Expression>>> params;
    unordered_map<string, unique_ptr<Statement>> body;

    // Env* parent;
    shared_ptr<Env> parent;
    
};

#endif /* Environment_hpp */
