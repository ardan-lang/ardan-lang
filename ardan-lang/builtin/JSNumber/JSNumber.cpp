//
//  JSNumber.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "JSNumber.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> JSNumber::construct() {
    
    shared_ptr<JSObject> obj = make_shared<JSObject>();
    
    return obj;
    
}
