//
//  JSBoolean.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "JSBoolean.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> JSBoolean::construct() {
    
    shared_ptr<JSObject> obj = make_shared<JSObject>();
    
    return obj;
    
}

Value JSBoolean::call(const std::vector<Value>& args) {
    // it will return a string
    
    if (args.size() <= 0) {
        return Value::str("");
    }
    
    return args[0];
}
