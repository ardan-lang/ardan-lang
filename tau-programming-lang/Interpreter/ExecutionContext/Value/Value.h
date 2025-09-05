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
#include <vector>

using namespace std;

enum class ValueType {
    NUMBER,
    STRING,
    BOOLEAN,
    OBJECT,
    UNDEFINED,
    NULLTYPE,
    NATIVE_FUNCTION,
    FUNCTION,
    METHOD
};

class Value;
class JSObject; // forward declare

using NativeFn = function<Value(const vector<Value>&)>;

class Value {
    
public:
    
    ValueType type;

    double numberValue;
    string stringValue;
    bool boolValue;
    shared_ptr<JSObject> objectValue;
    NativeFn nativeFunction;
    std::function<Value(std::vector<Value>)> functionValue;
    
    Value();

    static Value number(double n) { Value v; v.type = ValueType::NUMBER; v.numberValue = n; return v; }
    static Value str(const string& s) { Value v; v.type = ValueType::STRING; v.stringValue = s; return v; }
    static Value boolean(bool b) { Value v; v.type = ValueType::BOOLEAN; v.boolValue = b; return v; }
    static Value object(shared_ptr<JSObject> obj) { Value v; v.type = ValueType::OBJECT; v.objectValue = obj; return v; }
    static Value undefined() { return Value(); }
    static Value nullVal() { Value v; v.type = ValueType::NULLTYPE; return v; }
    static Value native(NativeFn fn) { Value v; v.type = ValueType::NATIVE_FUNCTION; v.nativeFunction = fn; return v; }
    static Value function(function<Value(vector<Value>)> fn) {
        Value v;
        v.type = ValueType::FUNCTION;
        v.functionValue = std::move(fn);
        return v;
    }
    static Value method(shared_ptr<JSObject> obj) {
        Value v;
        v.type = ValueType::METHOD;
        v.objectValue = obj;
        return v;
    }
    
    Value(int n);

    Value(double n);
    
    Value(const string& str);

    std::string toString() const;
    
    bool isUndefined();
    
};

#endif /* Value_h */
