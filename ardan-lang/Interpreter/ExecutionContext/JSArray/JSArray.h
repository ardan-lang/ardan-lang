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
#include "../JSObject/JSObject.h"
#include "../Value/Value.h"

using namespace std;

class JSArray : public JSObject {

    int elements_size;
    const vector<string> native_names = { "pop", "push", "length" };

public:
    JSArray() {
        
        // pop removes the first item in the properties
        set("pop", Value::native([](const std::vector<Value>& args) {

            return Value::nullVal();

        }));
        
        // pushes a vlaue to the properties
        set("push", Value::native([](const std::vector<Value>& args) {

            return Value::nullVal();
            
        }));
        
        set("length", elements_size);

    }
    
    void set(const string& key, const Value& val);
    
    void setIndex(size_t i, const Value& val);

    Value getIndex(size_t i);

    const unordered_map<string, Value> get_indexed_properties();

    void updateLength(size_t len);
    
    bool isNumeric(const std::string& s);
    
    string toString();
    
};

#endif /* JSArray_h */
