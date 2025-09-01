//
//  JSClass.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef JSClass_h
#define JSClass_h

#include <memory>
#include <string>
#include <vector>

class PropertyDeclaration;
class MethodDefinition;

using namespace std;

class JSClass {
public:
    string name;
    shared_ptr<JSClass> superClass;
    unordered_map<string, unique_ptr<PropertyDeclaration>> fields;
    unordered_map<string, unique_ptr<MethodDefinition>> methods;

    Value get(const string& key) {
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
};

#endif /* JSClass_h */
