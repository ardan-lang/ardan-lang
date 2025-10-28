//
//  View.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#include "View.hpp"
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../Compiler/VM/VM.hpp"
#include "../../../Compiler/Turbo/TurboVM.hpp"

shared_ptr<JSObject> View::construct() {

    NSView* view = [[NSView alloc] initWithFrame:NSMakeRect(100, 100, 100, 40)];

    // Wrap in JSObject
    shared_ptr<JSObject> obj = make_shared<JSObject>();
    obj->set_builtin_value("constructor", Value::native([this](const std::vector<Value>& args) -> Value {
        return Value();
    }));
    
    obj->set_builtin_value("addComponent", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        addComponent(args[0], obj);
        return Value();
    }));

    obj->set("view", Value::any((NSView*)view), "VAR", {});

    return obj;

}


void View::addComponent(Value object, shared_ptr<JSObject> this_object) {
    auto control_view = object.objectValue->get("view").as<NSView*>();
    auto this_control_view = this_object->get("view").as<NSView*>();
    [this_control_view addSubview:control_view];
}
