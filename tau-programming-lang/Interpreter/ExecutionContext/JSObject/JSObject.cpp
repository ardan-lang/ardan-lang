//
//  JSObject.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include <stdio.h>
#include "JSObject.h"

bool JSObject::operator==(const JSObject& other) const {
    return this == &other;
}

bool JSObject::operator!=(const JSObject& other) const {
    return !(*this == other);
}

// TODO: fix to set let and const too.
void JSObject::set(const string& key, const Value& val) {
    
    // Look in own properties
    auto it = var_properties.find(key);
    if (it != var_properties.end()) {
        var_properties[key] = val;
    }
    
    auto let_it = let_properties.find(key);
    if (let_it != let_properties.end()) {
        let_properties[key] = val;
    }
    
    auto const_it = const_properties.find(key);
    if (const_it != const_properties.end()) {
        throw runtime_error("Cannot set value to an already assigned value to const.");
    }

}

void JSObject::set(const string& key, const Value& val, string type) {
    if (type == "LET") {
        let_properties[key] = val;
    } else if(type == "CONST") {
        const_properties[key] = val;
    } else {
        var_properties[key] = val;
    }
}

Value JSObject::get(const string& key) {
    
    // Look in own properties
    auto it = var_properties.find(key);
    if (it != var_properties.end()) {
        return it->second;
    }
    
    auto let_it = let_properties.find(key);
    if (let_it != let_properties.end()) {
        return let_it->second;
    }
    
    auto const_it = const_properties.find(key);
    if (const_it != const_properties.end()) {
        return const_it->second;
    }

    // Walk prototype chain (parent object)
    // parent_js_object is nullptr here.
    // TODO: fix it
    if (parent_object) {
        Value val = parent_object->get(key);
        if (!val.isUndefined()) {
            return val;
        }
    }

    // Look in class (and superclass chain)

    return Value::undefined();
    
}

void JSObject::setClass(shared_ptr<JSClass> js_klass) {
    js_class = js_klass;
}

const unordered_map<string, Value> JSObject::get_all_properties() {
    
    unordered_map<string, Value> all_properties = {};
    
    for (auto& prop : var_properties) {
        all_properties[prop.first] = prop.second;
    }
    
    for (auto& let_prop : let_properties) {
        all_properties[let_prop.first] = let_prop.second;
    }
    
    for (auto& const_prop : const_properties) {
        all_properties[const_prop.first] = const_prop.second;
    }

    return all_properties;
}

shared_ptr<JSClass> JSObject::getKlass() {
    return js_class;
}
