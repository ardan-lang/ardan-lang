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

class VM;
class TurboVM;

class JSArray : public JSObject {

    int elements_size;

public:
    
    JSArray() {
        
        init_builtins();

    }
    
    void init_builtins();
    
    void set(const string& key, const Value& val);
    
    void setIndex(size_t i, const Value& val);

    Value getIndex(size_t i);

    const unordered_map<string, Value> get_indexed_properties();

    void updateLength(size_t len);
    
    bool isNumeric(const std::string& s);
    
    void push(const vector<Value>& args);
    void pop();
    
    string toString();
    size_t length();
    
};

#endif /* JSArray_h */
