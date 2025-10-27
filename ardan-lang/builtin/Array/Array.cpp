//
//  Array.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "Array.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> Array::construct() {
    
    auto arr = make_shared<JSArray>();
    
    arr->set_builtin_value("constructor", Value::native([this, arr](const std::vector<Value>& args) -> Value {
        
        arr->set("value", args[0]);
        
        return Value();

    }));
    
    return arr;
    
}

Value Array::call(const std::vector<Value>& args) {
    // it will return a string
    
    auto arr = make_shared<JSArray>();

    if (args.size() == 0) {
        
        return Value::array(arr);
        
    } else if (args.size() == 1) {
        
        int arg_len = args[0].numberValue;
        
        for (int i = 0; i < arg_len; i++) {
            arr->push({ Value::str("") });
        }
        
    } else {
        
        for (auto arg : args) {
            arr->push({ arg });
        }

    }

    return Value::array(arr);
    
}
