//
//  TextBox.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 28/10/2025.
//

#include "TextBox.hpp"
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../Compiler/VM/VM.hpp"
#include "../../../Compiler/Turbo/TurboVM.hpp"

shared_ptr<JSObject> Text::construct() {
    
    NSTextField *label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 200, 24)];
            
    shared_ptr<JSObject> obj = make_shared<JSObject>();
        
    obj->set_builtin_value("constructor", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        
        setText(args[0].toString(), obj);
        
        return Value();
        
    }));
    
    obj->set("view", Value::any((NSView*)label), "VAR", {});

    return obj;
    
}

void Text::setText(string text, shared_ptr<JSObject> this_obj) {

    NSTextField* label = (NSTextField*)this_obj->get("view").as<NSView*>();

    [label setStringValue:[NSString stringWithUTF8String:text.c_str()]];
    
}

void Text::setBezeled(bool value, shared_ptr<JSObject> this_obj) {
    
    NSTextField* label = (NSTextField*)this_obj->get("view").as<NSView*>();
    
    [label setBezeled:NO];
    
}

void Text::setDrawsBackground(bool value, shared_ptr<JSObject> this_obj) {
    
    NSTextField* label = (NSTextField*)this_obj->get("view").as<NSView*>();
    
    [label setDrawsBackground:NO];
    
}

void Text::setEditable(bool value, shared_ptr<JSObject> this_obj) {
    
    NSTextField* label = (NSTextField*)this_obj->get("view").as<NSView*>();
    
    [label setEditable:NO];
    
}

void Text::setSelectable(bool value, shared_ptr<JSObject> this_obj) {
    
    NSTextField* label = (NSTextField*)this_obj->get("view").as<NSView*>();
    
    [label setSelectable:NO];
    
}

