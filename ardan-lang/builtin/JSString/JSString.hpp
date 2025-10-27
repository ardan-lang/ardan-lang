//
//  JSString.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#ifndef JSString_hpp
#define JSString_hpp

#include <stdio.h>
#include <string>
#include <algorithm>

#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"

class JSString : public JSClass {
    
public:
    JSString() {
        is_native = true;
    }
        
    shared_ptr<JSObject> construct() override;

};

#endif /* JSString_hpp */
