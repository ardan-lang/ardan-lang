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

std::shared_ptr<JSObject> Button::construct() {
    if (!buttonTargets) buttonTargets = [NSMutableArray array];

    button = [[NSButton alloc] initWithFrame:NSMakeRect(100, 100, 100, 40)];
    [button setBezelStyle:NSBezelStyleRounded];
    [button setTitle:@"Button"];
    
    // Wrap in JSObject
    obj = make_shared<JSObject>();

    obj->set_builtin_value("setTitle", Value::native([this](const std::vector<Value>& args) -> Value {
        setTitle(args[0].stringValue);
        return Value();
    }));

    obj->set_builtin_value("setFrame", Value::native([this](const std::vector<Value>& args) -> Value {
        if (args.size() >= 4)
            setPosition(args[0].numberValue, args[1].numberValue, args[2].numberValue, args[3].numberValue);
        return Value();
    }));

    obj->set_builtin_value("onClick", Value::native([this](const std::vector<Value>& args) -> Value {
        // args[0] expected to be JS function wrapped in Value
        clickHandler = [fn = args[0]] {
            // fn.call({});
        };
        onClick(clickHandler);
        return Value();
    }));
    
    // obj->set_builtin_value("this", Value::)

    return obj;
}

void Button::setTitle(std::string title) {
    [button setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

void Button::setPosition(float x, float y, float width, float height) {
    [button setFrame:NSMakeRect(x, y, width, height)];
}

void Button::onClick(std::function<void()> callback) {
    ButtonTarget *target = [[ButtonTarget alloc] init];
    target.callback = ^{
        callback();
    };
    [button setTarget:target];
    [button setAction:@selector(onClick:)];
    [buttonTargets addObject:target]; // retain target
}
