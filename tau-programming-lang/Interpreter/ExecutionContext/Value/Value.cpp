//
//  Value.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "Value.h"

Value::Value() : type(ValueType::UNDEFINED) {}

Value::Value(int n) {
    type = ValueType::NUMBER;
    numberValue = static_cast<double>(n);
}

Value::Value(double n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(const string& str) {
    stringValue = str;
    type = ValueType::STRING;
}

std::string Value::toString() const {
    switch(type) {
        case ValueType::NUMBER: return std::to_string(numberValue);
        case ValueType::STRING: return stringValue;
        case ValueType::BOOLEAN: return boolValue ? "true" : "false";
        case ValueType::OBJECT: return "[object Object]";
        case ValueType::UNDEFINED: return "undefined";
        case ValueType::NULLTYPE: return "null";
        case ValueType::NATIVE_FUNCTION: return "[native Function]";
    }
    return "unknown";
}

bool Value::isUndefined() {
    return type == ValueType::UNDEFINED ? true : false;
}
