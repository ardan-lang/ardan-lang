//
//  Array.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#ifndef Array_hpp
#define Array_hpp

#include <stdio.h>
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"

class Array : public JSClass {
    
public:
    Array() {
        is_native = true;
    }
        
    shared_ptr<JSObject> construct() override;
    Value call(const std::vector<Value>& args) override;

};

#endif /* Array_hpp */
