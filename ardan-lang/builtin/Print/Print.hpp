//
//  Print.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#ifndef Print_hpp
#define Print_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include "../../Interpreter/R.hpp"

using namespace std;

void printValue(const R&);

class Print : public JSObject {
    
public:
    
    Print() {
        
        set("log", Value::native([](const vector<Value>& args) {
            
            vector<R> r_args;
            
            for (auto& arg : args) {
                r_args.push_back(arg);
            }
            
            Print::print(r_args);
            
            return Value();
            
        }));
        
    }
    
    static void print(const vector<R>& args) {
        
        int index = 0;
        
        for (auto& arg : args) {
            
            printValue(arg);
            
            if (index < (args.size() - 1)) {
                cout << ", ";
            }
            
            index++;
        }
        
        cout << endl;

    }
    
};

#endif /* Print_hpp */
