//
//  Environment.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 25/08/2025.
//

#ifndef Environment_hpp
#define Environment_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include "R.hpp"

using namespace std;

class Env {
public:
    R getValue(const std::string& key) {
        auto it = values.find(key);
        if (it != values.end()) {
            return it->second;
        }
        throw runtime_error("Undefined variable: " + key);
    }

    void setValue(const std::string& key, R value) {
        values[key] = std::move(value);
    }

private:
    std::unordered_map<std::string, R> values = {
        {"print", string("print")}
    };
};

#endif /* Environment_hpp */
