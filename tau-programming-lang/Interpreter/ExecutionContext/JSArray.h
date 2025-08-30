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

using namespace std;

class JSArray : public JSObject {
    int elements_size;
public:
    void setIndex(size_t i, const Value& val) {
        elements_size++;
        set(to_string(i), val);
        set("length", Value(elements_size));
    }

    Value getIndex(size_t i) {
        return (i < elements_size) ? get(to_string(i)) : Value::undefined();
    }

    void updateLength(size_t len) {
        set("length", Value((int)len));
    }
        
};

#endif /* JSArray_h */
