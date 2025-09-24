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

// forward
struct Chunk;

// A tiny Function object: holds chunk id/index and arity (and optional name)
struct FunctionObject {
    uint32_t chunkIndex;   // index into module/file chunk table
    uint32_t arity;
    std::string name;

    // optional: other metadata (source location, flags)
};

enum class ValueType {
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
    FUNCTION_REF
};

class Value;
class JSObject; // forward declare
class JSClass;
class JSArray;
class Promise;

using NativeFn = function<Value(const vector<Value>&)>;

class Value {
    
public:
    
    ValueType type;
    
    double numberValue;
    string stringValue;
    bool boolValue;
    shared_ptr<JSObject> objectValue;
    shared_ptr<JSArray> arrayValue;
    shared_ptr<JSClass> classValue;
    NativeFn nativeFunction;
    function<Value(std::vector<Value>)> functionValue;
    shared_ptr<Promise> promiseValue;
    std::shared_ptr<FunctionObject> fnRef; // if FUNCTION_REF
    
    Value() : type(ValueType::UNDEFINED), numberValue(0), boolValue(false) {}
    
    static Value number(double n) { Value v; v.type = ValueType::NUMBER; v.numberValue = n; return v; }
    static Value str(const string& s) { Value v; v.type = ValueType::STRING; v.stringValue = s; return v; }
    static Value boolean(bool b) { Value v; v.type = ValueType::BOOLEAN; v.boolValue = b; return v; }
    static Value object(shared_ptr<JSObject> obj) { Value v; v.type = ValueType::OBJECT; v.objectValue = obj; return v; }
    static Value array(shared_ptr<JSArray> array) {
        Value v; v.type = ValueType::ARRAY; v.arrayValue = array; return v;
    }
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
    static Value method(shared_ptr<JSClass> klass) {
        Value v;
        v.type = ValueType::METHOD;
        v.classValue = klass;
        return v;
    }
    
    static Value promise(shared_ptr<JSObject> promise_value) {
        Value v;
        v.type = ValueType::OBJECT;
        v.objectValue = promise_value;
        return v;
    }
    
    static Value functionRef(std::shared_ptr<FunctionObject> f) {
        Value v; v.type = ValueType::FUNCTION_REF; v.fnRef = f; return v;
    }
    
    Value(int n);
    
    Value(double n);
    Value(long n);
    Value(long long n);
    Value(short n);
    Value(unsigned short n);
    Value(unsigned int n);
    Value(unsigned long n);
    Value(unsigned long long n);
    Value(float n);
    Value(long double n);

    Value(const string& str);
    
    std::string toString() const;
    bool isTruthy() const;
    
    bool isUndefined();
    int integer();
    bool boolean();
    bool isNull();
    
    bool operator==(const Value& rhs);
    
    friend std::ostream& operator<<(std::ostream& os, const Value& v) {
        switch(v.type) {
//            case NUMBER: os << v.num; break;
//            case BOOL: os << (v.b ? "true" : "false"); break;
//            case STRING: os << v.str; break;
//            case NIL_: os << "nil"; break;
//            case OBJECT: os << "[object]"; break;
        }
        return os;
    }
    
};

struct ValueField {
    string key;
    vector<string> modifiers;
    Value value;
};

#endif /* Value_h */

