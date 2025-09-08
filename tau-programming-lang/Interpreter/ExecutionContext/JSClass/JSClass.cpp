//
//  JSClass.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "JSClass.h"

static Value native(NativeFn fn) { Value v; v.type = ValueType::NATIVE_FUNCTION; v.nativeFunction = fn; return v; }

// this fetches data from static_fields
Value JSClass::get(const string& key, bool perform_privacy_check) {
    
    //TODO: we need to check if the key is truly a static field
    
    // Look in fields
    auto value = var_static_fields.find(key);
    if (value != var_static_fields.end()) {
        
        // when found, check if it
        return value->second;
    }
    
    // Look in let fields
    auto let_value = let_static_fields.find(key);
    if (let_value != let_static_fields.end()) {
        
        // when found, check if it
        return let_value->second;
    }
    
    // Look in const fields
    auto const_value = const_static_fields.find(key);
    if (const_value != const_static_fields.end()) {
        
        // when found, check if it
        return const_value->second;
    }

    // Walk superclass chain
    if (superClass) {
        return superClass->get(key, perform_privacy_check);
    }
    
    throw runtime_error( key + " does not exist as static in this class.");
        
}

void JSClass::set(const string& key, Value value, bool perform_privacy_check) {

    // check type of static field.

    //search in var, let and const.
    if (var_static_fields.find(key) != var_static_fields.end()) {
        var_static_fields[key] = value;
    }
    
    // Look in let fields
    else if (let_static_fields.find(key) != let_static_fields.end()) {
        let_static_fields[key] = value;
    }
    
    // TODO: check if we need to verify if a const value has already been set
    // Look in const fields
    else if (const_static_fields.find(key) != const_static_fields.end()) {
        const_static_fields[key] = value;
    }

}
