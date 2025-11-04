//
//  ObjectModel.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 04/11/2025.
//

#ifndef ObjectModel_hpp
#define ObjectModel_hpp

#include <stdio.h>

#pragma once
#include <memory>
#include <functional>
#include <string>
#include <vector>

#include "../../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../../Interpreter/ExecutionContext/Value/Value.h"
#include "CallStackManager.hpp"
#include "FunctionInvoker.hpp"
#include "TurboModule.hpp"

/*
 ObjectModel (aka ClassManager)
 - Responsible for:
   * CreateInstance(Value klass)
   * createJSObject(shared_ptr<JSClass>)
   * makeObjectInstance(Value klass, shared_ptr<JSObject>)
   * CreateObjectLiteralProperty(const Value& obj_val, const string& prop_name, const Value& object)
   * getProperty
   * setJSObjectClosure
 - It depends on:
   * TurboModule (for constant evaluation if needed)
   * A FunctionInvoker to evaluate initializer closures/fields
   * A callback to obtain the current "accessor" JSObject used for privacy checks (frame closure->js_object)
*/

class ObjectModel {
public:
    using CurrentObjectGetter = std::function<std::shared_ptr<JSObject>()>;

    ObjectModel(std::shared_ptr<TurboModule> module_,
                FunctionInvoker* invoker,
                CurrentObjectGetter currentGetter);

    // Create instance from class Value (handles native and non-native classes)
    Value createInstance(const Value& klassVal);

    // Create a plain JSObject from JSClass
    std::shared_ptr<JSObject> createJSObject(const std::shared_ptr<JSClass>& klass);

    // Called when constructing object instances to populate proto fields & consts
    void makeObjectInstance(const Value& klassVal, const std::shared_ptr<JSObject>& obj);

    // Create a named property on an object which may be a closure or direct value
    void createObjectLiteralProperty(const Value& propValue, const std::string& propName, const Value& targetObject);

    // Access property with privacy/protected checks.
    Value getProperty(const Value& objVal, const std::string& propName);

    // Walk through object properties and attach closures' js_object to owner
    void setJSObjectClosure(const Value& objVal);

private:
    std::shared_ptr<TurboModule> module_;
    FunctionInvoker* invoker_;
    CurrentObjectGetter currentObjectGetter_;

    // helpers
    bool isModifierPrivate(const std::vector<std::string>& modifiers) const;
    bool isModifierProtected(const std::vector<std::string>& modifiers) const;
};

#endif /* ObjectModel_hpp */
