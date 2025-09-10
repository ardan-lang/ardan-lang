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

Value::Value(long n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(long long n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(short n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(unsigned short n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(unsigned int n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(unsigned long n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(unsigned long long n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(float n) {
    type = ValueType::NUMBER;
    numberValue = n;
}

Value::Value(long double n) {
    type = ValueType::NUMBER;
    numberValue = n;
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
        case ValueType::ARRAY: return "[array Array]";
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
        case ValueType::ARRAY: return true;
    }
    return false;
}

bool Value::isUndefined() {
    return type == ValueType::UNDEFINED ? true : false;
}
