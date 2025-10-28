//
//  HStack.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 28/10/2025.
//

#include "HStack.hpp"
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../Compiler/VM/VM.hpp"
#include "../../../Compiler/Turbo/TurboVM.hpp"

shared_ptr<JSObject> HStack::construct() {
    
    NSStackView* stackView = [[NSStackView alloc] initWithFrame:NSMakeRect(250, 0, 500, 500)];
    [stackView setOrientation:NSUserInterfaceLayoutOrientationHorizontal];
    stackView.spacing = 10;
    stackView.alignment = NSLayoutAttributeCenterY;
    stackView.translatesAutoresizingMaskIntoConstraints = NO;
    
//    [NSLayoutConstraint activateConstraints:@[
//        [stackView.leadingAnchor constraintEqualToAnchor:parent.leadingAnchor constant:10],
//        [stackView.topAnchor constraintEqualToAnchor:parent.topAnchor constant:10]
//    ]];
    
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

//void HStack::addComponent(Value component, shared_ptr<JSObject> this_object) {
//
//    NSStackView* this_stack = (NSStackView*)this_object->get("view").as<NSView*>();
//    NSView* child = (NSView*)component.objectValue->get("view").as<NSView*>();
//
//    [this_stack addArrangedSubview:child];
//    
//}

void HStack::addComponent(Value component, shared_ptr<JSObject> this_object) {
    NSStackView* this_stack = (NSStackView*)this_object->get("view").as<NSView*>();
    NSView* child = (NSView*)component.objectValue->get("view").as<NSView*>();

    // Remove automatic layout behavior by not using addArrangedSubview:
    [this_stack addSubview:child];

    // Get the stack's frame size
    NSRect bounds = [this_stack bounds];
    CGFloat maxX = bounds.size.width - 100;  // 100 = assumed view width
    CGFloat maxY = bounds.size.height - 100; // 100 = assumed view height

    // Generate random x, y within bounds
    CGFloat randomX = arc4random_uniform((u_int32_t)maxX);
    CGFloat randomY = arc4random_uniform((u_int32_t)maxY);

    // Set a random position and size for the child
    [child setFrame:NSMakeRect(randomX, randomY, 100, 100)];
}
