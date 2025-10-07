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

public:
    JSArray() {
        
        init_builtins();

    }
    
    void init_builtins() {
        
        // pop removes the first item in the properties
        set("pop", Value::native([this](const std::vector<Value>& args) {

            pop();
            
            return Value::nullVal();

        }));
        
        // pushes a vlaue to the properties
        set("push", Value::native([this](const vector<Value>& args) {

            push(args);
            
            return Value::nullVal();
            
        }));
        
        set("join", Value::native([this](const vector<Value>& args) {

            string concat;
            string delimiter = args[0].toString();
            
            int index = 0;
            auto indexed_properties = get_indexed_properties();
            
            for(auto indexed_property : indexed_properties) {
                concat += indexed_property.second.toString() + ((index == (indexed_properties.size() - 1)) ? "" : delimiter);
                index++;
            }

            return Value(concat);
            
        }));
        
        set("reduce", Value::native([this](const vector<Value>& args) {
            
            if (args.size() < 1) {
                throw runtime_error("reduce expects at least (callback)");
            }
            
            Value callback = args[0];
            if (callback.type != ValueType::FUNCTION || callback.type != ValueType::CLOSURE) {
                throw std::runtime_error("First argument to reduce must be a function");
            }
            
            size_t len = elements_size;
            
            // Setup accumulator
            Value acc;
            size_t start = 0;
            if (args.size() >= 3) {
                acc = args[2]; // initial value
            } else {
                if (len == 0) {
                    throw runtime_error("reduce of empty array with no initial value");
                }
                acc = getIndex(0);
                start = 1;
            }
            
            for (size_t i = start; i < len; i++) {
                //vector<Value> cbArgs = { acc, getIndex(i), Value((double)i), this };
                std::vector<Value> cbArgs = {
                    acc,
                    getIndex(i),
                    Value((double)i)
                };
                
                if (callback.type == ValueType::FUNCTION) {
                    acc = callback.functionValue(cbArgs);
                } else if (callback.type == ValueType::CLOSURE) {
                    // TODO: check this out
                    acc = args[args.size() - 1].functionValue(cbArgs);
                }
            }
            
            return acc;
            
        }));

        set("length", elements_size);

    }
    
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
