//
//  R.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 25/08/2025.
//

#ifndef R_hpp
#define R_hpp

#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>
#include <variant>
#include <memory>
#include "ExecutionContext/JSArray/JSArray.h"
#include "ExecutionContext/JSClass/JSClass.h"

using namespace std;

using R = std::variant<
    std::monostate,
    std::nullptr_t,
    double,
    unsigned long,
    unsigned short,

    long,
    long long,
    short,
    unsigned int, unsigned long long,
    float, long double,

    int,
    char,
    string,
    bool,
    shared_ptr<JSObject>,
    shared_ptr<Value>,
    shared_ptr<JSClass>,
    shared_ptr<JSArray>,
    Value
>;

#endif /* R_hpp */
