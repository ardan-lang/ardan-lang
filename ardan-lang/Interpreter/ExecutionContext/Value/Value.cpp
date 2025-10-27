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
                return std::format("{}", static_cast<long long>(numberValue)); //std::to_string(static_cast<long long>(numberValue));
            else
                return std::format("{}",
                                   numberValue); // std::to_string(numberValue);
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
        case ValueType::PROMISE: return "[promise Promise]";
        case ValueType::FUNCTION_REF: return "fn";
        case ValueType::CLOSURE: return "closure";
        case ValueType::CLASS: return "class";
        case ValueType::ANY: return "any";
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
        case ValueType::PROMISE: return true;
        case ValueType::FUNCTION_REF: return true;
        case ValueType::CLOSURE: return true;
        case ValueType::CLASS: return true;
        case ValueType::ANY: return true;
    }
    return false;
}

bool Value::isUndefined() {
    return type == ValueType::UNDEFINED ? true : false;
}

int Value::integer() const {
    switch (type) {
        case ValueType::BOOLEAN: return boolValue ? 1 : 0;
        case ValueType::NUMBER: return numberValue != 0 && !std::isnan(numberValue) ? 1 : 0;
        case ValueType::STRING: return !stringValue.empty() ? 1 : 0;
        case ValueType::NULLTYPE: return 0;
        case ValueType::UNDEFINED: return 0;
        case ValueType::OBJECT: return 1;
        case ValueType::FUNCTION: return 1;
        case ValueType::NATIVE_FUNCTION: return 1;
        case ValueType::METHOD: return 1;
        case ValueType::ARRAY: return 1;
        case ValueType::PROMISE: return 1;
        case ValueType::FUNCTION_REF: return 1;
        case ValueType::CLOSURE: return 1;
        case ValueType::CLASS: return 1;
        case ValueType::ANY: return 1;
    }
    return 0;
}

bool Value::boolean() const {
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
        case ValueType::PROMISE: return true;
        case ValueType::FUNCTION_REF: return true;
        case ValueType::CLOSURE: return true;
        case ValueType::CLASS: return true;
        case ValueType::ANY: return true;
    }
    return false;
}

bool Value::isNull() {
    
    switch (type) {
        case ValueType::BOOLEAN: return !boolValue;
        case ValueType::NUMBER: return numberValue == 0 && std::isnan(numberValue);
        case ValueType::STRING: return stringValue.empty();
        case ValueType::NULLTYPE: return true;
        case ValueType::UNDEFINED: return true;
        case ValueType::OBJECT: return false;
        case ValueType::FUNCTION: return false;
        case ValueType::NATIVE_FUNCTION: return false;
        case ValueType::METHOD: return false;
        case ValueType::ARRAY: return false;
        case ValueType::PROMISE: return false;
        case ValueType::FUNCTION_REF: return false;
        case ValueType::CLOSURE: return false;
        case ValueType::CLASS: return false;
        case ValueType::ANY: return false;
    }
    return false;

}

string Value::type_of() {
    switch (type) {
        case ValueType::NUMBER: return "number";
        case ValueType::STRING: return "string";
        case ValueType::BOOLEAN: return "boolean";
        case ValueType::OBJECT: return "object";
        case ValueType::NULLTYPE: return "null";
        case ValueType::UNDEFINED: return "undefined";
        case ValueType::NATIVE_FUNCTION: return "function";
        case ValueType::FUNCTION: return "function";
        case ValueType::METHOD: return "method";
        case ValueType::ARRAY: return "array";
        case ValueType::PROMISE: return "promise";
        case ValueType::FUNCTION_REF: return "fn";
        case ValueType::CLOSURE: return "closure";
        case ValueType::CLASS: return "class";
        case ValueType::ANY: return "any";
    }
    return "undefined";
}
