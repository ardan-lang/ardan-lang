//
//  BaseVM.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 31/10/2025.
//

#ifndef BaseVM_hpp
#define BaseVM_hpp

#include <stdio.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>
#include <cmath>
#include <stdexcept>

#include "Bytecode.hpp"
#include "Chunk.hpp"
#include "../Interpreter/ExecutionContext/Value/Value.h"
#include "../Interpreter/ExecutionContext/JSArray/JSArray.h"
#include "../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../Interpreter/Utils/Utils.h"
#include "../builtin/Print/Print.hpp"
#include "Module.hpp"

#include "../builtin/Print/Print.hpp"
#include "../builtin/builtin-includes.h"
#include "../GUI/gui.h"
#include "../Interpreter/Promise/Promise.hpp"
#include "../builtin/Server/Server.hpp"
#include "../Interpreter/Env.h"

class IVM {
public:
    virtual ~IVM() = default;
    virtual Value run(shared_ptr<void> chunk, const vector<Value>& args = {}) = 0;
    virtual void init_builtins() = 0;
};

template <typename DerivedVM, typename ModuleT, typename ChunkT>
class BaseVM {
public:
    void init_builtins();
    Value getProperty(const Value &objVal, const string &propName);

    void setStaticProperty(const Value &objVal, const string &propName, const Value &val) {
        if (objVal.type == ValueType::CLASS) {
            // objVal.classValue->set_static_vm(propName, val);
            return;
        }
        throw std::runtime_error("Cannot set static property on non-class");
    }

    int getValueLength(Value& v) {
        
        if (v.type == ValueType::OBJECT) {
            return (int)v.objectValue->get_all_properties().size();
        }
        
        if (v.type == ValueType::ARRAY) {
            return v.arrayValue->get("length").numberValue;
        }
        
        return v.numberValue;

    }
    
    void closeUpvalues(Value* last);
    shared_ptr<Upvalue> captureUpvalue(Value* local);
    
    Value CreateInstance(Value klass);
    void CreateObjectLiteralProperty(Value obj_val, string prop_name, Value object);
    void InvokeConstructor(Value obj_value, vector<Value> args);
    
    const unordered_map<string, Value> enumerateKeys(Value obj) {
        
        if (obj.type == ValueType::OBJECT) {
            return obj.objectValue->get_all_properties();
        }
        
        if (obj.type == ValueType::ARRAY) {
            return obj.arrayValue->get_indexed_properties();
        }
        
        return {};
        
    }

    shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    void set_js_object_closure(Value objVal);
    void makeObjectInstance(Value klass, shared_ptr<JSObject> obj);
    
    Value binaryAdd(const Value &a, const Value &b) {
        if (a.type == ValueType::STRING || b.type == ValueType::STRING) {
            return Value::str(a.toString() + b.toString());
        }
        return Value(a.numberValue + b.numberValue);
    }

    bool isTruthy(const Value &v) {
        if (v.type == ValueType::NULLTYPE) return false;
        if (v.type == ValueType::UNDEFINED) return false;
        if (v.type == ValueType::BOOLEAN) return v.boolValue;
        if (v.type == ValueType::NUMBER) return v.numberValue != 0;
        if (v.type == ValueType::STRING) return !v.stringValue.empty();
        // objects/arrays considered truthy
        return true;
    }

    bool equals(const Value &a, const Value &b) {
        // shallow equality similar to interpreter
        if (a.type != b.type) {
            // try numeric-string comparisons etc is omitted for brevity
            return a.toString() == b.toString();
        }
        switch (a.type) {
            case ValueType::NUMBER: return a.numberValue == b.numberValue;
            case ValueType::STRING: return a.stringValue == b.stringValue;
            case ValueType::BOOLEAN: return a.boolValue == b.boolValue;
            case ValueType::NULLTYPE:
            case ValueType::UNDEFINED:
                return true;
            default:
                // object identity
                if (a.type == ValueType::OBJECT && b.type == ValueType::OBJECT)
                    return a.objectValue == b.objectValue;
                if (a.type == ValueType::ARRAY && b.type == ValueType::ARRAY)
                    return a.arrayValue == b.arrayValue;
                return false;
        }
    }

    // MDN: The in operator returns true if the specified property is in the specified object or its prototype chain.
    bool in(Value objVal, Value b) {
        std::string propName = b.toString();

        // Standard object
        if (objVal.type == ValueType::OBJECT) {
            auto object = objVal.objectValue;
            while (object) {
                if (object->has(propName)) {
                    return true;
                }
                object = object->parent_object;
            }
            return false;
        }

        // Array
        if (objVal.type == ValueType::ARRAY) {
            if (objVal.arrayValue->has(propName)) {
                return true;
            }
            return false;
        }

        // Class (static fields)
        if (objVal.type == ValueType::CLASS) {
            auto cls = objVal.classValue;
            while (cls) {
                if (cls->has_static(propName)) {
                    return true;
                }
                cls = cls->superClass;
            }
            return false;
        }

        // primitives
        if (objVal.type == ValueType::STRING) {
            auto jsString = make_shared<JSString>();
            auto native_object = jsString->construct();
            if (native_object->has(propName)) {
                return true;
            }
            return false;
        }

        return false;
    }

    string type_of(Value value) {
        return value.type_of();
    }

    // checks if an object is an instance of a specific class or constructor function,
    // or if its prototype chain includes the prototype of the specified constructor.
    // obj, class
    bool instance_of(Value a, Value b) {
        if (a.type != ValueType::OBJECT || b.type != ValueType::CLASS) {
            return false;
        }
        auto proto = b.classValue;//->getPrototypeObject();
        auto obj = a.objectValue;
        while (obj) {
            if (obj->getKlass().get() == proto.get()) return true;
            obj = obj->parent_object; //prototype;
        }
        return false;
    }

    // delete property from object
    bool delete_op(Value object, Value property) {
        setProperty(object, property.toString(), Value::undefined());
        return true;
    }
    
    void setProperty(const Value &objVal, const string &propName, const Value &val) {
        if (objVal.type == ValueType::OBJECT) {
            objVal.objectValue->set(propName, val, "VAR", {});
            return;
        }
        if (objVal.type == ValueType::ARRAY) {
            objVal.arrayValue->set(propName, val);
            return;
        }
        
        // TODO: make sure to check for privacy
        // if objVal is a class then the property to et is a static.
        if (objVal.type == ValueType::CLASS) {
            objVal.classValue->set(propName, val, false);
            return;
        }
        throw std::runtime_error("Cannot set property on non-object");
    }


protected:
    DerivedVM* self() { return static_cast<DerivedVM*>(this); }
};

#endif /* BaseVM_hpp */
