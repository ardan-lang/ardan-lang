//
//  VStack.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 24/10/2025.
//

#include "VStack.hpp"
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../Compiler/VM/VM.hpp"
#include "../../../Compiler/Turbo/TurboVM.hpp"

shared_ptr<JSObject> VStack::construct() {
    
    NSStackView* stackView = [[NSStackView alloc] init];
    [stackView setOrientation:NSUserInterfaceLayoutOrientationHorizontal];

    auto obj = make_shared<JSObject>();
    
    obj->set_builtin_value("addComponent", Value::native([this, obj](const std::vector<Value>& args) -> Value {

        if (args.size() < 1) {
            throw runtime_error("addComponent expects 1 argument");
        }

        addComponent(args[0], obj);
        
        return Value();
        
    }));

    obj->set("view", Value::any((NSView*)stackView), "VAR", {});

    return obj;
}

void VStack::addComponent(Value component, shared_ptr<JSObject> this_object) {

    NSStackView* this_stack = (NSStackView*)this_object->get("view").as<NSView*>();
    NSView* child = (NSView*)component.objectValue->get("view").as<NSView*>();

    [this_stack addArrangedSubview:child];
    
}
