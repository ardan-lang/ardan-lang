//
//  Button.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#include "Button.hpp"
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../Compiler/VM/VM.hpp"
#include "../../../Compiler/Turbo/TurboVM.hpp"

@interface ButtonTarget : NSObject
@property (nonatomic, copy) void (^callback)(void);
- (void)onClick:(id)sender;
@end

@implementation ButtonTarget
- (void)onClick:(id)sender {
    if (self.callback) self.callback();
}
@end

static NSMutableArray<ButtonTarget *> *buttonTargets;

NSView* Button::getNativeView() {
    return nullptr;
}

shared_ptr<JSObject> Button::construct() {
    if (!buttonTargets) buttonTargets = [NSMutableArray array];

    NSButton* button = [[NSButton alloc] initWithFrame:NSMakeRect(100, 100, 100, 40)];
    [button setBezelStyle:NSBezelStyleRounded];

    // Wrap in JSObject
    shared_ptr<JSObject> obj = make_shared<JSObject>();
    
    int timeInInt = static_cast<int>(time(nullptr));
    obj->_id = timeInInt;
    
    button_objects[to_string(obj->_id)] = obj;
    
    obj->set_builtin_value("constructor", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        
        string name = args[0].stringValue;
        setTitle(name, obj);
        
        clickHandler = [this, args, obj] {
            obj->turboVM->callFunction(args[1], {});
        };
        
        onClick(clickHandler, obj);
        
        return Value();
    }));

    obj->set_builtin_value("setTitle", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        setTitle(args[0].stringValue, obj);
        return Value();
    }));

    obj->set_builtin_value("setFrame", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        if (args.size() >= 4)
            setPosition(args[0].numberValue, args[1].numberValue, args[2].numberValue, args[3].numberValue, obj);
        return Value();
    }));

    obj->set_builtin_value("onClick", Value::native([this, obj](const std::vector<Value>& args) -> Value {
        
        clickHandler = [args, this, obj] {
            obj->turboVM->callFunction(args[1], {});
        };
        
        onClick(clickHandler, obj);
        
        return Value();
    }));
    
    obj->set("view", Value::any((NSView*)button), "VAR", {});

    return obj;
}

void Button::setTitle(std::string title, shared_ptr<JSObject> obj_button) {
    NSButton* button = (NSButton*)obj_button->get("view").as<NSView*>();
    [button setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

void Button::setPosition(float x, float y, float width, float height, shared_ptr<JSObject> obj_button) {
    NSButton* button = (NSButton*)obj_button->get("view").as<NSView*>();
    [button setFrame:NSMakeRect(x, y, width, height)];
}

void Button::onClick(std::function<void()> callback, shared_ptr<JSObject> obj_button) {
    NSButton* button = (NSButton*)obj_button->get("view").as<NSView*>();
    ButtonTarget *target = [[ButtonTarget alloc] init];
    target.callback = ^{
        callback();
    };
    [button setTarget:target];
    [button setAction:@selector(onClick:)];
    [buttonTargets addObject:target]; // retain target
}
