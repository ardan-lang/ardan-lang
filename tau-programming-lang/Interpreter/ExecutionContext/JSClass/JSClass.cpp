//
//  JSClass.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "JSClass.h"

Value JSClass::get(const string& key) {
    // Look in fields
    auto fieldIt = fields.find(key);
    if (fieldIt != fields.end()) {
        // Wrap property declaration as a Value (object or something appropriate)
        // For now: return name of field
        return Value::str("field:" + key);
    }
    
    // Look in methods
    auto methodIt = methods.find(key);
    if (methodIt != methods.end()) {
        // Wrap method definition into a callable Value
        // For now: return name of method
        return Value::str("method:" + key);
    }
    
    // Walk superclass chain
    if (superClass) {
        return superClass->get(key);
    }
    
    return Value::undefined();
}
