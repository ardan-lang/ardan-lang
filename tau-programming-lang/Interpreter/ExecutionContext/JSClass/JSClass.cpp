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
    auto value = static_fields.find(key);
    if (value != static_fields.end()) {
        
        // when found, check if it
        
        return value->second;
    }
        
    // Walk superclass chain
    if (superClass) {
        return superClass->get(key, perform_privacy_check);
    }
    
    throw runtime_error( key + " does not exist as static in this class.");
        
}

void JSClass::set(const string& key, Value value, bool perform_privacy_check) {
    static_fields[key] = value;
}
