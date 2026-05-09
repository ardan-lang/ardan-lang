//
//  JSBoolean.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#ifndef JSBoolean_hpp
#define JSBoolean_hpp

#include <stdio.h>
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"

class JSBoolean : public JSClass {
    
public:
    JSBoolean() {
        is_native = true;
    }
        
    shared_ptr<JSObject> construct() override;
    Value call(const std::vector<Value>& args) override;
    
};

#endif /* JSBoolean_hpp */
