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

void JSObject::set(const string& key, const Value& val) {
    properties[key] = val;
}

Value JSObject::get(const string& key) {
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

void JSObject::setClass(shared_ptr<JSClass> js_klass) {
    js_class = js_klass;
}

const unordered_map<string, Value>* JSObject::get_all_properties() {
    return &properties;
}

shared_ptr<JSClass> JSObject::getKlass() {
    return js_class;
}
