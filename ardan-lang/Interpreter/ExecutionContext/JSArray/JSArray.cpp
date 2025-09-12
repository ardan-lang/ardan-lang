//
//  JSArray.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "JSArray.h"

void JSArray::set(const string& key,
                    const Value& val) {
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

bool JSArray::isNumeric(const std::string& s) {
    return !s.empty() &&
           std::all_of(s.begin(), s.end(), ::isdigit);
}

//bool is_native_name = std::any_of(
//                             native_names.begin(),
//                             native_names.end(),
//                             [&](const std::string &name) {
//                                 return prop.first.find(name) != std::string::npos;
//                             }
//                             );

