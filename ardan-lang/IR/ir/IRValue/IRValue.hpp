//
//  IRValue.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 13/05/2026.
//

#ifndef IRValue_hpp
#define IRValue_hpp

#include <stdio.h>
#include <string>

enum class IRValueType {
    NUMBER,
    STRING,
    BOOLEAN,
    OBJECT,
    ARRAY,
    UNDEFINED,
    NULLTYPE,
    NATIVE_FUNCTION,
    FUNCTION,
    METHOD,
    PROMISE,
    FUNCTION_REF,
    CLOSURE,
    CLASS,
    ANY
};

class IRValue {
public:
    
    IRValueType type;
    
    double numberValue;
    std::string stringValue;
    bool boolValue;

};

#endif /* IRValue_hpp */
