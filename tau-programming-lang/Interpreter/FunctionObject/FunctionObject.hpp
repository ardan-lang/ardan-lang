//
//  FunctionObject.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 26/08/2025.
//

#ifndef FunctionObject_hpp
#define FunctionObject_hpp

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <stdexcept>

#include "../Env.h"
#include "../R.hpp"
#include "../Interpreter.h"
#include "../../Statements/Statements.hpp"

using namespace std;

//struct FunctionObject {
//    string name;
//    vector<string> params;
//    unique_ptr<Statement> body;
//    Env* closure;
//
//    FunctionObject(string name, vector<string> params, unique_ptr<Statement> body, Env* closure)
//        : name(std::move(name)), params(std::move(params)), body(std::move(body)), closure(closure) {}
//
//    Value call(Interpreter& interp, const vector<Value>& args) {
//        Env local(closure);
//
//        for (size_t i = 0; i < params.size(); ++i) {
//            local.setValue(params[i], args[i]);
//        }
//
//        return true;//interp.executeBlock(*body, &local);
//    }
//};

#endif /* FunctionObject_hpp */
