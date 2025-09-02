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
#include "../../Interpreter/Utils/Utils.h"

using namespace std;

class Print {
public:
    static void print(vector<R> args) {
        
        int index = 0;
        
        for (auto& arg : args) {
            
            printValue(arg);
            
            if (index < (args.size() - 1)) {
                std::cout << ", ";
            }
            
            index++;
        }
        
        cout << std::endl;

    }
};

#endif /* Print_hpp */
