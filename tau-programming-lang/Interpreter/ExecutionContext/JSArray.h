//
//  JSArray.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef JSArray_h
#define JSArray_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>
#include "JSObject.h"
#include "Value.h"

using namespace std;

class JSArray : public JSObject {
    vector<Value> elements;
public:
    void setIndex(size_t i, const Value& val) {
        if (i >= elements.size()) {
            elements.resize(i+1);
        }
        elements[i] = val;
        set("length", Value((int)elements.size())); // keep length in sync
    }

    Value getIndex(size_t i) {
        return (i < elements.size()) ? elements[i] : Value::undefined();
    }

    void updateLength(size_t len) {
        set("length", Value((int)len));
    }
};

#endif /* JSArray_h */
