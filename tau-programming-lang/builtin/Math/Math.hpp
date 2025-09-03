//
//  Math.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#ifndef Math_hpp
#define Math_hpp

#include <stdio.h>
#include <iostream>

#include "../../Interpreter/R.hpp"

using namespace std;

class Math : public JSObject {

public:
    
    Math() {
        set("abs", Value::native([](const std::vector<Value>& args) {
            double n = args.size() > 0 ? args[0].numberValue : 0;
            return Value::number(std::abs(n));
        }));
        
        set("pow", Value::native([](const std::vector<Value>& args) {
            double num = args.size() > 0 ? args[0].numberValue : 0;
            double exp = args.size() > 1 ? args[1].numberValue : 0;
            return Value::number(Math::pow(num, exp));
        }));
        
        set("PI", Value::number(Math::PI));
        
    }
    
    const static int PI = (22/7);
    
    static double pow(double num, double exp) {

        double summation = 0;

        for (int index = 0; index < exp; index++) {
            cout << summation << endl;
            summation = num * num;
        }
        
        return summation;
        
    }
    
};

#endif /* Math_hpp */
