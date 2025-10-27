//
//  JSString.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "JSString.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> JSString::construct() {

    shared_ptr<JSObject> obj = make_shared<JSObject>();
    
    obj->set("value", Value(), "VAR", {} );

    obj->set_builtin_value("constructor", Value::native([this, obj](const std::vector<Value>& args) -> Value {

        obj->set("value", args[0], "VAR", {} );
        
        return Value();

    }));

    obj->set_builtin_value("toLowerCase", Value::native([this, obj](const std::vector<Value>& args) -> Value {

        string str = obj->get("value").toString();
        
        std::transform(str.begin(), str.end(), str.begin(), [this](unsigned char c){ return std::tolower(c); });

        return Value::str(str);

    }));
    
    obj->set_builtin_value("toUpperCase", Value::native([this, obj](const std::vector<Value>& args) -> Value {

        string str = obj->get("value").toString();
        
        std::transform(str.begin(), str.end(), str.begin(), [this](unsigned char c){ return std::toupper(c); });

        return Value::str(str);

    }));

    return obj;

}
