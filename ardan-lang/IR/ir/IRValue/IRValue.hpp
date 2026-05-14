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

//class IRValue {
//public:
//
//    IRValueType type;
//
//    double numberValue;
//    std::string stringValue;
//    bool boolValue;
//
//    IRValue(double n, IRValueType type) : numberValue(n), type(type) {}
//    IRValue(std::string n, IRValueType type) : stringValue(n), type(type) {}
//    IRValue(bool n, IRValueType type) : boolValue(n), type(type) {}
//
//};

//| Type | Meaning    |
//| ---- | ---------- |
//| i1   | boolean    |
//| i8   | 8-bit int  |
//| i32  | 32-bit int |
//| i64  | 64-bit int |
//| f32  | float      |
//| f64  | double     |
//| ptr  | pointer    |
//| void | no value   |
//| str  | string     |
//| obj  | object     |
//| fn   | function   |

enum class IRType {
    Void,
    I32,
    F64,
    Bool,
    String,
    Object,
    Function,
    Ptr,
    Number,
    Array,
    Any
};

struct IRValue {
    std::string name;
    IRType type;

    IRValue() = default;
    IRValue(std::string n, IRType t = IRType::Void)
        : name(std::move(n)), type(std::move(t)) {}
};

#endif /* IRValue_hpp */
