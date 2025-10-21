//
//  JSObject.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include <stdio.h>
#include "JSObject.h"
#include "../../../Compiler/VM/VM.hpp"
#include "../../../Compiler/Turbo/TurboVM.hpp"

bool JSObject::operator==(const JSObject& other) const {
    return this == &other;
}

bool JSObject::operator!=(const JSObject& other) const {
    return !(*this == other);
}

// TODO: fix to set let and const too.
void JSObject::set(const string& key, const Value& val) {
    
    if (is_object_literal) {
        
        // do not check to see if the key exists before setting the value
        // always set the value
        var_properties[key] = { key, {}, val };
        
    } else {
        
        // Look in own properties
        auto it = var_properties.find(key);
        if (it != var_properties.end()) {
            var_properties[key].value = val;
        }
        
        auto let_it = let_properties.find(key);
        if (let_it != let_properties.end()) {
            let_properties[key].value = val;
        }
        
        auto const_it = const_properties.find(key);
        if (const_it != const_properties.end()) {
            throw runtime_error("Cannot set value to an already assigned value to const.");
        }
        
    }

}

void JSObject::set(const string& key, const Value& val, string type, vector<string> modifiers) {
    if (type == "LET") {
        let_properties[key] = { key, modifiers, val };
    } else if(type == "CONST") {
        const_properties[key] = { key, modifiers, val };
    } else {
        var_properties[key] = { key, modifiers, val };
    }
}

Value JSObject::get(const string& key) {
    
    // Look in own properties
    auto it = var_properties.find(key);
    if (it != var_properties.end()) {
        return it->second.value;
    }
    
    auto let_it = let_properties.find(key);
    if (let_it != let_properties.end()) {
        return let_it->second.value;
    }
    
    auto const_it = const_properties.find(key);
    if (const_it != const_properties.end()) {
        return const_it->second.value;
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

vector<string> JSObject::get_modifiers(const string& key) {
    
    // Look in own properties
    auto it = var_properties.find(key);
    if (it != var_properties.end()) {
        return it->second.modifiers;
    }
    
    auto let_it = let_properties.find(key);
    if (let_it != let_properties.end()) {
        return let_it->second.modifiers;
    }
    
    auto const_it = const_properties.find(key);
    if (const_it != const_properties.end()) {
        return const_it->second.modifiers;
    }

    // Walk prototype chain (parent object)
    // parent_js_object is nullptr here.
    // TODO: fix it
//    if (parent_object) {
//        auto val = parent_object->get_modifiers(key);
//        if (val.size()) {
//            return val;
//        }
//    }

    // Look in class (and superclass chain)

    return {};

}

void JSObject::setClass(shared_ptr<JSClass> js_klass) {
    js_class = js_klass;
}

const unordered_map<string, Value> JSObject::get_all_properties() {
    
    unordered_map<string, Value> all_properties = {};
    
    for (auto& prop : var_properties) {
        all_properties[prop.first] = prop.second.value;
    }
    
    for (auto& let_prop : let_properties) {
        all_properties[let_prop.first] = let_prop.second.value;
    }
    
    for (auto& const_prop : const_properties) {
        all_properties[const_prop.first] = const_prop.second.value;
    }

    return all_properties;
}

shared_ptr<JSClass> JSObject::getKlass() {
    return js_class;
}

string JSObject::toString() {
    
    string concat = "{";
    int index = 0;
    
    for (auto prop : get_all_properties()) {
                
        if (prop.second.type == ValueType::ARRAY) {
            concat += prop.second.toString();
        }

        concat += prop.first + ": ";
        concat += prop.second.stringValue + ( index >= (get_all_properties().size() - 1) ? "" : ", ");
        
        index++;
        
    }
    
    concat += "}";
    
    return concat;
    
}

void JSObject::set_as_object_literal() {
    is_object_literal = true;
}

void JSObject::set_builtin_value(const string& key, const Value& val) {
    var_properties[key] = { key, {}, val };
}
