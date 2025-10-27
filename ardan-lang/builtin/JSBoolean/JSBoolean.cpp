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
    
    obj->set("value", Value(), "VAR", {} );

    obj->set_builtin_value("constructor", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        
        Value value;
        
        if (args.size() > 0) {
            value = Value::boolean(args[0].boolean());
        } else {
            value = Value::boolean(false);
        }

        obj->set("value", value, "VAR", {} );
        
        return Value();

    }));

    return obj;
    
}

Value JSBoolean::call(const std::vector<Value>& args) {
    // it will return a string
    
    if (args.size() == 0) {
        return Value::boolean(false);
    } else {
        return Value::boolean(args[0].boolean());
    }
    
    return Value::boolean(args[0].boolean());
}
