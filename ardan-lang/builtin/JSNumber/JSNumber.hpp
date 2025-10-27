//
//  JSNumber.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#ifndef JSNumber_hpp
#define JSNumber_hpp

#include <stdio.h>
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"

class JSNumber : public JSClass {
    
public:
    JSNumber() {
        is_native = true;
    }
        
    shared_ptr<JSObject> construct() override;
    Value call(const std::vector<Value>& args) override;
    
};

#endif /* JSNumber_hpp */
