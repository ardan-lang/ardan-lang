//
//  JSObject.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef JSObject_h
#define JSObject_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>
#include "Value.h"
#include "JSClass.h"

using namespace std;

// class JSClass;

class JSObject {
    unordered_map<string, Value> properties;

    // class of this object
    shared_ptr<JSClass> js_class;

public:

    shared_ptr<JSClass> parent_class;
    shared_ptr<JSObject> parent_object;

    bool operator==(const JSObject& other) const {
        return this == &other;
    }
    
    bool operator!=(const JSObject& other) const {
        return !(*this == other);
    }

    void set(const string& key, const Value& val) {
        properties[key] = val;
    }
    
    Value get(const string& key) {
        // Look in own properties
        auto it = properties.find(key);
        if (it != properties.end()) {
            return it->second;
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
        if (parent_class) {
            Value val = parent_class->get(key);
            if (!val.isUndefined()) {
                return val;
            }
        }

        return Value::undefined();
    }

    void setClass(shared_ptr<JSClass> js_klass) {
        js_class = js_klass;
    }
    
    const unordered_map<string, Value>* get_all_properties() {
        return &properties;
    }
    
    shared_ptr<JSClass> getKlass() {
        return js_class;
    }
    
};

#endif /* JSObject_h */
