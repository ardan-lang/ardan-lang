//
//  JSArray.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "JSArray.h"

// void JSObject::set(const string& key, const Value& val, string type, vector<string> modifiers)

void JSArray::setIndex(size_t i, const Value& val) {
    elements_size++;
    set(to_string(i), val, "VAR", {});
    set("length", Value(elements_size), "VAR", {});
}

Value JSArray::getIndex(size_t i) {
    return (i < elements_size) ? get(to_string(i)) : Value::undefined();
}

void JSArray::updateLength(size_t len) {
    set("length", Value((int)len), "VAR", {});
}

string JSArray::toString() {
    
    string concat = "[";
    int index = 0;
    for (auto prop : get_all_properties()) {
        
        if (prop.first == "length") {
            index++;
            continue;
        }
        
        if (prop.second.type == ValueType::ARRAY) {
            concat += prop.second.toString();
        }
        
        concat += prop.second.stringValue + ( index >= (get_all_properties().size() - 1) ? "" : ", ");
        
        index++;
        
    }
    concat += "]";
    
    return concat;
    
}
