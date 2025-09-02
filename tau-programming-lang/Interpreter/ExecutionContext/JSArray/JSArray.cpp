//
//  JSArray.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "JSArray.h"

void JSArray::setIndex(size_t i, const Value& val) {
    elements_size++;
    set(to_string(i), val);
    set("length", Value(elements_size));
}

Value JSArray::getIndex(size_t i) {
    return (i < elements_size) ? get(to_string(i)) : Value::undefined();
}

void JSArray::updateLength(size_t len) {
    set("length", Value((int)len));
}
