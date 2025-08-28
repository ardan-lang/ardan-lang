//
//  R.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 25/08/2025.
//

#ifndef R_hpp
#define R_hpp

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>

#include "ExecutionContext/JSObject.h"

using namespace std;

using R = variant<monostate, nullptr_t, double, size_t, int, char, string, bool, std::shared_ptr<JSObject>, std::shared_ptr<Value>>;

#endif /* R_hpp */
