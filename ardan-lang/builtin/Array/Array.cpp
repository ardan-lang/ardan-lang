//
//  Array.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "Array.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> Array::construct() {
    
    shared_ptr<JSObject> obj = make_shared<JSObject>();
    
    return obj;
    
}

Value Array::call(const std::vector<Value>& args) {
    // it will return a string
    
    if (args.size() <= 0) {
        return Value::str("");
    }
    
    return args[0];
}
