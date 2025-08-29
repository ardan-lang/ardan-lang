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
#include "ExecutionContext/JSArray.h"
#include "ExecutionContext/JSClass.h"

using namespace std;

using R = std::variant<
    std::monostate,
    std::nullptr_t,
    double,
    size_t,
    int,
    char,
    std::string,
    bool,
    std::shared_ptr<JSObject>,
    std::shared_ptr<Value>,
    std::shared_ptr<JSClass>
>;

#endif /* R_hpp */
