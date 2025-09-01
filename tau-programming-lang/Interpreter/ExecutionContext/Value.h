//
//  Value.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef Value_h
#define Value_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>

using namespace std;

enum class ValueType {
    NUMBER,
    STRING,
    BOOLEAN,
    OBJECT,
    UNDEFINED,
    NULLTYPE
};

class JSObject; // forward declare

class Value {
public:
    ValueType type;

    double numberValue;
    std::string stringValue;
    bool boolValue;
    std::shared_ptr<JSObject> objectValue;

    Value() : type(ValueType::UNDEFINED) {}
    static Value number(double n) { Value v; v.type=ValueType::NUMBER; v.numberValue=n; return v; }
    static Value str(const std::string& s) { Value v; v.type=ValueType::STRING; v.stringValue=s; return v; }
    static Value boolean(bool b) { Value v; v.type=ValueType::BOOLEAN; v.boolValue=b; return v; }
    static Value object(std::shared_ptr<JSObject> obj) { Value v; v.type=ValueType::OBJECT; v.objectValue=obj; return v; }
    static Value undefined() { return Value(); }
    static Value nullVal() { Value v; v.type=ValueType::NULLTYPE; return v; }

    Value(int n) {
        type = ValueType::NUMBER;
        numberValue = static_cast<double>(n);
    }

    Value(double n) {
        type = ValueType::NUMBER;
        numberValue = n;
    }
    
    Value(const string& str) {
        stringValue = str;
        type = ValueType::STRING;
    }

    std::string toString() const {
        switch(type) {
            case ValueType::NUMBER: return std::to_string(numberValue);
            case ValueType::STRING: return stringValue;
            case ValueType::BOOLEAN: return boolValue ? "true" : "false";
            case ValueType::OBJECT: return "[object Object]";
            case ValueType::UNDEFINED: return "undefined";
            case ValueType::NULLTYPE: return "null";
        }
        return "unknown";
    }
    
    bool isUndefined() {
        return type == ValueType::UNDEFINED ? true : false;
    }
    
};

#endif /* Value_h */
