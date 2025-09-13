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

    if (perform_privacy_check) {
        // if key is private throw error
        check_privacy(key);

    }

    //TODO: we need to check if the key is truly a static field
    
    // Look in fields
    auto value = var_static_fields.find(key);
    if (value != var_static_fields.end()) {
        
        // when found, check if it
        return value->second.value;
    }
    
    // Look in let fields
    auto let_value = let_static_fields.find(key);
    if (let_value != let_static_fields.end()) {
        
        // when found, check if it
        return let_value->second.value;
    }
    
    // Look in const fields
    auto const_value = const_static_fields.find(key);
    if (const_value != const_static_fields.end()) {
        
        // when found, check if it
        return const_value->second.value;
    }

    // Walk superclass chain
    if (superClass) {
        return superClass->get(key, perform_privacy_check);
    }
    
    throw runtime_error( key + " does not exist as static in this class.");
        
}

// calling this, we don't need the modifiers because it has been set by visitClassDeclarartions
void JSClass::set(const string& key, Value value, bool perform_privacy_check) {

    // if there is a this_binding, then we know, we are being called from inside a class.
    // so we can access all both private/public fields
    
    // if perform_privacy_check is true, then we need to check if the key is private.
    
    // check type of static field.
    
    if (perform_privacy_check) {
        // if key is private throw error
        check_privacy(key);

    }

    //search in var, let and const.
    if (var_static_fields.find(key) != var_static_fields.end()) {
        var_static_fields[key].value = value;
    }
    
    // Look in let fields
    else if (let_static_fields.find(key) != let_static_fields.end()) {
        let_static_fields[key].value = value;
    }
    
    // TODO: check if we need to verify if a const value has already been set
    // Look in const fields
    else {
        
        if (const_static_fields.find(key) != const_static_fields.end()) {
            throw runtime_error("Cannot assign a value to a const variable.");
        } else {
            const_static_fields[key].value = value;
        }
        
    }

}

void JSClass::check_privacy(const std::string& key) {
    // check in var static fields
    auto value = var_static_fields.find(key);
    if (value != var_static_fields.end()) {
        if (hasModifier(value->second.modifiers, "private")) {
            throw std::runtime_error("Access denied: '" + key + "' is private");
        }
    }

    // check in let static fields
    auto let_value = let_static_fields.find(key);
    if (let_value != let_static_fields.end()) {
        if (hasModifier(let_value->second.modifiers, "private")) {
            throw runtime_error("Access denied: '" + key + "' is private");
        }
    }

    // check in const static fields
    auto const_value = const_static_fields.find(key);
    if (const_value != const_static_fields.end()) {
        if (hasModifier(const_value->second.modifiers, "private")) {
            throw runtime_error("Access denied: '" + key + "' is private");
        }
    }

    // throw std::runtime_error("Property not found: " + key);
}

bool JSClass::hasModifier(const std::vector<std::string>& mods, const std::string& name) {
    return std::find(mods.begin(), mods.end(), name) != mods.end();
}

// only called in visitClassDeclarartions
void JSClass::set_var(const string& key, Value value, const vector<string> modifiers) {
    var_static_fields[key] = { key, modifiers, value };
}

void JSClass::set_let(const string& key, Value value, const vector<string> modifiers) {
    let_static_fields[key] = { key, modifiers, value };
}

void JSClass::set_const(const string& key, Value value, const vector<string> modifiers) {
    const_static_fields[key] = { key, modifiers, value };
}
