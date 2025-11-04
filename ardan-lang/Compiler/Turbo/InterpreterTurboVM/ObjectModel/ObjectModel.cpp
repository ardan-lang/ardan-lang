//
//  ObjectModel.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 04/11/2025.
//

#include "ObjectModel.hpp"
#include <stdexcept>

ObjectModel::ObjectModel(std::shared_ptr<TurboModule> module_,
                         FunctionInvoker* invoker,
                         CurrentObjectGetter currentGetter)
    : module_(std::move(module_)), invoker_(invoker), currentObjectGetter_(std::move(currentGetter))
{
}

Value ObjectModel::createInstance(const Value& klassVal) {
    if (klassVal.type != ValueType::CLASS) {
        throw std::runtime_error("createInstance: value is not a class");
    }

    auto klass = klassVal.classValue;

    // Native class
    if (klass->is_native) {
        if (!klass->is_constructor_available()) {
            // add default ctor closure via class API – here we assume caller installs ctor
            // If class has method to set ctor, that should be used. For backwards compatibility,
            // we create a simple ctor closure if missing.
            // (FunctionInvoker might evaluate if necessary)
            // leave it to class internals; fall through to construct()
        }

        std::shared_ptr<JSObject> nativeObj = klass->construct();
        Value objValue = Value::object(nativeObj);

        // attach closures so they have a pointer to owner object
        setJSObjectClosure(objValue);

        return objValue;
    }

    // Non-native class: create JSObject and initialize proto/const fields
    std::shared_ptr<JSObject> obj = createJSObject(klass);
    Value objValue = Value::object(obj);

    return objValue;
}

std::shared_ptr<JSObject> ObjectModel::createJSObject(const std::shared_ptr<JSClass>& klass) {
    auto obj = std::make_shared<JSObject>();
    obj->setClass(klass);

    // initialize proto properties (fields and closures)
    makeObjectInstance(Value::klass(klass), obj);

    // if superclass exists create parent_object
    if (klass->superClass) {
        if (klass->superClass->is_native) {
            obj->parent_object = klass->superClass->construct();
            obj->parent_class = klass->superClass;
        } else {
            obj->parent_object = createJSObject(klass->superClass);
            obj->parent_class = klass->superClass;
        }
    }
    return obj;
}

void ObjectModel::makeObjectInstance(const Value& klassVal, const std::shared_ptr<JSObject>& obj) {
    if (klassVal.type != ValueType::CLASS) {
        throw std::runtime_error("makeObjectInstance: klassVal is not CLASS");
    }
    auto klass = klassVal.classValue;

    // prototype (var) properties
    for (auto& kv : klass->var_proto_props) {
        const std::string& propName = kv.first;
        const auto& propInfo = kv.second;

        if (propInfo.value.type == ValueType::CLOSURE) {
            // create a bound closure that has js_object set to this object
            auto protoClosure = propInfo.value.closureValue;
            auto newClosure = std::make_shared<Closure>();
            newClosure->fn = protoClosure->fn;
            newClosure->upvalues = protoClosure->upvalues;
            newClosure->js_object = obj;
            newClosure->ctx = protoClosure->ctx;
            obj->set(propName, Value::closure(newClosure), "VAR", propInfo.modifiers);
        } else {
            // evaluate initializer (value.numberValue is index into module constants in original design)
            if (propInfo.value.type == ValueType::NUMBER && module_) {
                int fieldReg = propInfo.value.numberValue;
                if (fieldReg >= 0 && fieldReg < (int)module_->constants.size()) {
                    Value fnValue = module_->constants[fieldReg];
                    // evaluate function/initializer via invoker if it's callable
                    Value evaluated = Value::undefined();
                    if (fnValue.type == ValueType::CLOSURE || fnValue.type == ValueType::FUNCTION || fnValue.type == ValueType::NATIVE_FUNCTION) {
                        evaluated = invoker_->call(fnValue, {});
                    } else {
                        // constant value stored directly
                        evaluated = fnValue;
                    }
                    obj->set(propName, evaluated, "VAR", propInfo.modifiers);
                } else {
                    obj->set(propName, propInfo.value, "VAR", propInfo.modifiers);
                }
            } else {
                obj->set(propName, propInfo.value, "VAR", propInfo.modifiers);
            }
        }
    }

    // const proto properties
    for (auto& kv : klass->const_proto_props) {
        const std::string& propName = kv.first;
        const auto& propInfo = kv.second;

        if (propInfo.value.type == ValueType::CLOSURE) {
            auto protoClosure = propInfo.value.closureValue;
            auto newClosure = std::make_shared<Closure>();
            newClosure->fn = protoClosure->fn;
            newClosure->upvalues = protoClosure->upvalues;
            newClosure->js_object = obj;
            newClosure->ctx = protoClosure->ctx;
            obj->set(propName, Value::closure(newClosure), "CONST", {});
        } else {
            // evaluate initializer if it is a constant reference to module constants
            if (propInfo.value.type == ValueType::NUMBER && module_) {
                int fieldReg = propInfo.value.numberValue;
                if (fieldReg >= 0 && fieldReg < (int)module_->constants.size()) {
                    Value fnValue = module_->constants[fieldReg];
                    Value evaluated = fnValue;
                    if (fnValue.type == ValueType::CLOSURE || fnValue.type == ValueType::FUNCTION || fnValue.type == ValueType::NATIVE_FUNCTION) {
                        evaluated = invoker_->call(fnValue, {});
                    }
                    obj->set(propName, evaluated, "CONST", propInfo.modifiers);
                } else {
                    obj->set(propName, propInfo.value, "CONST", propInfo.modifiers);
                }
            } else {
                obj->set(propName, propInfo.value, "CONST", propInfo.modifiers);
            }
        }
    }
}

void ObjectModel::createObjectLiteralProperty(const Value& propValue, const std::string& propName, const Value& targetObject) {
    if (targetObject.type != ValueType::OBJECT) {
        throw std::runtime_error("createObjectLiteralProperty: target not an object");
    }
    auto obj = targetObject.objectValue;

    if (propValue.type == ValueType::CLOSURE) {
        auto protoClosure = propValue.closureValue;
        auto newClosure = make_shared<Closure>();
        newClosure->fn = protoClosure->fn;
        newClosure->upvalues = protoClosure->upvalues;
        newClosure->js_object = obj;
        newClosure->ctx = protoClosure->ctx;
        obj->set(propName, Value::closure(newClosure), "VAR", { "public" });
    } else {
        obj->set(propName, propValue, "VAR", { "public" });
    }
}

Value ObjectModel::getProperty(const Value& objVal, const std::string& propName) {
    // object
    if (objVal.type == ValueType::OBJECT) {
        auto obj = objVal.objectValue;
        auto modifiers = obj->get_modifiers(propName);

        bool isPrivate = isModifierPrivate(modifiers);
        bool isProtected = isModifierProtected(modifiers);

        // privacy: only allow private if current accessor is same object
        auto accessor = currentObjectGetter_();
        if (isPrivate) {
            if (!accessor || accessor.get() != obj.get()) {
                throw std::runtime_error("Can't access '" + propName + "' — private property outside its class.");
            }
        }

        if (isProtected) {
            if (!accessor) {
                throw std::runtime_error("Can't access '" + propName + "' — protected property outside class/subclass.");
            }
            auto accessorClass = accessor->getKlass();
            auto ownerClass = obj->getKlass();
            bool allowed = false;
            while (accessorClass) {
                if (accessorClass.get() == ownerClass.get()) { allowed = true; break; }
                accessorClass = accessorClass->superClass;
            }
            if (!allowed) {
                throw std::runtime_error("Can't access '" + propName + "' — protected property outside class/subclass.");
            }
        }

        return obj->get(propName);
    }

    // array
    if (objVal.type == ValueType::ARRAY) {
        return objVal.arrayValue->get(propName);
    }

    // class static access
    if (objVal.type == ValueType::CLASS) {
        auto modifiers = objVal.classValue->get_static_modifiers(propName);
        bool isPrivate = isModifierPrivate(modifiers);
        bool isProtected = isModifierProtected(modifiers);

        auto accessor = currentObjectGetter_();
        if (isPrivate) {
            if (!accessor || accessor->getKlass().get() != objVal.classValue.get()) {
                throw std::runtime_error("Can't access private static property outside its class.");
            }
        }
        if (isProtected) {
            if (!accessor) throw std::runtime_error("Can't access protected static property outside class/subclass.");
            auto accessorClass = accessor->getKlass();
            auto targetClass = objVal.classValue;
            bool allowed = false;
            while (accessorClass) {
                if (accessorClass.get() == targetClass.get()) { allowed = true; break; }
                accessorClass = accessorClass->superClass;
            }
            if (!allowed) throw std::runtime_error("Can't access protected static property outside class/subclass.");
        }

        return objVal.classValue->get(propName, false);
    }

    // string primitive -> create temporary JSString wrapper
//    if (objVal.type == ValueType::STRING) {
//        auto jsStr = std::make_shared<JSString>();
//        auto nativeObj = jsStr->construct();
//        Value objValue = Value::object(nativeObj);
//        std::vector<Value> args{ objVal.toString() };
//        // try to call constructor; we assume invoker_ or VM will handle this; attempt via invoker if possible
//        try {
//            invoker_->call(nativeObj->get("constructor"), args);
//        } catch (...) {
//            // ignore if cannot call; user wants property access
//        }
//        return nativeObj->get(propName);
//    }

    // numbers/booleans etc -> undefined
    return Value::undefined();
}

void ObjectModel::setJSObjectClosure(const Value& objVal) {
    
    if (objVal.type == ValueType::OBJECT) {

        //objVal.objectValue->turboVM = this;

        for(auto& prop : objVal.objectValue->get_all_properties()) {
            if (prop.second.type == ValueType::CLOSURE) {
                prop.second.closureValue->js_object = objVal.objectValue;
                
            }
        }
    }
    
}

bool ObjectModel::isModifierPrivate(const std::vector<std::string>& modifiers) const {
    for (auto& m : modifiers) if (m == "private") return true;
    return false;
}
bool ObjectModel::isModifierProtected(const std::vector<std::string>& modifiers) const {
    for (auto& m : modifiers) if (m == "protected") return true;
    return false;
}
