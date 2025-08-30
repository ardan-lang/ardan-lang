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

using namespace std;

class JSClass;

class JSObject {
    unordered_map<string, Value> properties;
    shared_ptr<JSClass> js_class;
public:

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
        return properties.count(key) ? properties.at(key) : Value::undefined();
    }
    
    void setClass(shared_ptr<JSClass> js_klass) {
        js_class = js_klass;
    }
    
    shared_ptr<JSClass> getKlass() {
        return js_class;
    }
    
};

#endif /* JSObject_h */
