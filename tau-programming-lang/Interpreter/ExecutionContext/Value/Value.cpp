//
//  Value.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "Value.h"

Value::Value(int n) {
    type = ValueType::NUMBER;
    numberValue = static_cast<int>(n);
}

Value::Value(double n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(const string& str) {
    stringValue = str;
    type = ValueType::STRING;
}

string Value::toString() const {
    switch (type) {
        case ValueType::NUMBER: {
            // handle integers vs doubles
            if (std::floor(numberValue) == numberValue)
                return std::to_string(static_cast<long long>(numberValue));
            else
                return std::to_string(numberValue);
        }
        case ValueType::STRING: return stringValue;
        case ValueType::BOOLEAN: return boolValue ? "true" : "false";
        case ValueType::OBJECT: return "[object Object]";
        case ValueType::NULLTYPE: return "null";
        case ValueType::UNDEFINED: return "undefined";
        case ValueType::NATIVE_FUNCTION: return "[native function]";
        case ValueType::FUNCTION: return "[function]";
        case ValueType::METHOD: return "[method]";
    }
    return "undefined";
}

bool Value::isTruthy() const {
    switch (type) {
        case ValueType::BOOLEAN: return boolValue;
        case ValueType::NUMBER: return numberValue != 0 && !std::isnan(numberValue);
        case ValueType::STRING: return !stringValue.empty();
        case ValueType::NULLTYPE: return false;
        case ValueType::UNDEFINED: return false;
        case ValueType::OBJECT: return true;
        case ValueType::FUNCTION: return true;
        case ValueType::NATIVE_FUNCTION: return true;
        case ValueType::METHOD: return true;
    }
    return false;
}

Value operator+(const Value& lhs, const Value& rhs) {
    // If either is string → do string concatenation
    if (lhs.type == ValueType::STRING || rhs.type == ValueType::STRING) {
        return Value::str(lhs.toString() + rhs.toString());
    }

    // Else treat both as numbers
    return Value::number(lhs.numberValue + rhs.numberValue);
}

bool operator==(const Value& lhs, const Value& rhs) {
    if (lhs.type != rhs.type) return false;
    switch (lhs.type) {
        case ValueType::NUMBER: return lhs.numberValue == rhs.numberValue;
        case ValueType::STRING: return lhs.stringValue == rhs.stringValue;
        case ValueType::BOOLEAN: return lhs.boolValue == rhs.boolValue;
        case ValueType::NULLTYPE: return true;
        case ValueType::UNDEFINED: return true;
        case ValueType::OBJECT: return lhs.objectValue == rhs.objectValue; // pointer equality
        default: return false;
    }
}

bool Value::isUndefined() {
    return type == ValueType::UNDEFINED ? true : false;
}
