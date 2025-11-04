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
class Env;
struct Closure;
class JSObject;
class ExecutionContext;

// A tiny Function object: holds chunk id/index and arity (and optional name)
struct FunctionObject {
    uint32_t chunkIndex;   // index into module/file chunk table
    uint32_t arity;
    string name;
    uint32_t upvalues_size;
    bool isAsync;
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
    FUNCTION_REF,
    CLOSURE,
    CLASS,
    ANY
};

class Value;
class JSObject; // forward declare
class JSClass;
class JSArray;
class Promise;
struct Closure;

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
    shared_ptr<Closure> closureValue;
    std::any anyValue;
        
    static Value any(std::any any) {
        Value v;
        v.type = ValueType::ANY;
        v.anyValue = std::move(any);
        return v;
    }
    
    // --- Getters ---
    template<typename T>
    T as() const {
        if (type != ValueType::ANY)
            throw runtime_error("Value is not of type ANY");
        return std::any_cast<T>(anyValue);
    }
    
    bool is(ValueType t) const { return type == t; }
    
    Value() : type(ValueType::UNDEFINED), numberValue(0), boolValue(false) {}
    
    static Value number(double n) { Value v; v.type = ValueType::NUMBER; v.numberValue = n; return v; }
    static Value str(const string& s) { Value v; v.type = ValueType::STRING; v.stringValue = s; return v; }
    static Value boolean(bool b) { Value v; v.type = ValueType::BOOLEAN; v.boolValue = b; return v; }
    static Value object(shared_ptr<JSObject> obj) { Value v; v.type = ValueType::OBJECT; v.objectValue = obj; return v; }
    static Value array(shared_ptr<JSArray> array) {
        Value v; v.type = ValueType::ARRAY; v.arrayValue = array; return v;
    }
    static Value undefined() { Value v; v.type = ValueType::UNDEFINED; return v; }
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
    static Value klass(shared_ptr<JSClass> klass) {
        Value v;
        v.type = ValueType::CLASS;
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
    
    static Value closure(shared_ptr<Closure> closure) {
        Value new_value;
        new_value.type = ValueType::CLOSURE;
        new_value.closureValue = closure;
        return new_value;
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
    int integer() const;
    bool boolean() const;
    bool isNull();
    string type_of();
    
    bool operator==(const Value& rhs);
    
    //    friend std::ostream& operator<<(std::ostream& os, const Value& v) {
    //        switch(v.type) {
    //            case ValueType::NUMBER: os << v.numberValue; break;
    //            case ValueType::STRING: os << '"' << v.stringValue << '"'; break;
    //            case ValueType::BOOLEAN: os << (v.boolValue ? "true" : "false"); break;
    //            case ValueType::OBJECT: os << "[object Object]"; break;
    //            case ValueType::ARRAY: os << "[Array]"; break;
    //            case ValueType::UNDEFINED: os << "undefined"; break;
    //            case ValueType::NULLTYPE: os << "null"; break;
    //            case ValueType::NATIVE_FUNCTION: os << "[native function]"; break;
    //            case ValueType::FUNCTION: os << "[function]"; break;
    //            case ValueType::METHOD: os << "[method]"; break;
    //            case ValueType::PROMISE: os << "[Promise]"; break;
    //            case ValueType::FUNCTION_REF: os << "[FunctionRef]"; break;
    //            case ValueType::CLOSURE: os << "[Closure]"; break;
    //            case ValueType::CLASS: os << "[class]"; break;
    //            default: os << "[Unknown ValueType]"; break;
    //        }
    //        return os;
    //    }
    
};

struct ValueField {
    string key;
    vector<string> modifiers;
    Value value;
};

// --- Closure and Upvalue support ---
struct Upvalue {
    Value* location;   // Points to stack slot or closed value
    Value closed;      // When closed, stores value
    Upvalue* next = nullptr; // For linked-list of open upvalues (optional)
    bool isClosed() const { return location == &closed; }
};

struct Closure {
    shared_ptr<FunctionObject> fn;
    vector<shared_ptr<Upvalue>> upvalues;
    shared_ptr<JSObject> js_object;
    shared_ptr<ExecutionContext> ctx;
};

#endif /* Value_h */

