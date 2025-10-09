//
//  JSArray.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "JSArray.h"
#include "../../../Compiler/VM/VM.hpp"

void JSArray::set(const string& key, const Value& val) {
    if (isNumeric(key)) {
        size_t idx = std::stoull(key);
        if (idx >= elements_size) {
            elements_size = static_cast<int>(idx) + 1;
            set("length", Value(elements_size));
        }
        var_properties[key] = { key, {}, val };
    } else {
        var_properties[key] = { key, {}, val };
    }
}

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

const unordered_map<string, Value> JSArray::get_indexed_properties() {
    
    unordered_map<string, Value> indexed_properties = {};

    for (auto prop : var_properties) {
        
        if (isNumeric(prop.first)) {
            indexed_properties[prop.first] = prop.second.value;
        }
        
    }
    
    return indexed_properties;
    
}

string JSArray::toString() {
    
    string concat = "[";
    int index = 0;
    
    auto all_properties = get_indexed_properties();
    
    for (auto prop : all_properties) {
                        
        if (prop.second.type == ValueType::ARRAY) {
            concat += prop.second.toString();
        }
        
        if (prop.second.type == ValueType::OBJECT) {
            concat += prop.second.toString();
        }
        
        concat += prop.second.toString() + ( index >= (all_properties.size() - 1) ? "" : ", ");
        
        index++;
        
    }
    concat += "]";
    
    return concat;
    
}

size_t JSArray::length() {
    return elements_size;
}

bool JSArray::isNumeric(const std::string& s) {
    return !s.empty() &&
           std::all_of(s.begin(), s.end(), ::isdigit);
}

void JSArray::push(const vector<Value> &args) {
    setIndex((elements_size), args[0]);
}

void JSArray::pop() {
    if (elements_size == 0)
        return;
    // Get the last index as a string
    size_t lastIndex = elements_size - 1;
    var_properties.erase(to_string(lastIndex));
    elements_size--;
    set("length", Value(elements_size));
}

void JSArray::init_builtins() {

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
        if (callback.type != ValueType::FUNCTION && callback.type != ValueType::CLOSURE) {
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
                acc = vm->callFunction(callback, cbArgs);
            }
        }
        
        return acc;
        
    }));

    set("length", elements_size);

}
