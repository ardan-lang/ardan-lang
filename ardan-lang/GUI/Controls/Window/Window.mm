//
//  Window.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#include "Window.hpp"
#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

@interface MyAppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@property (strong) NSMutableArray *callbacks;
@end

@implementation MyAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {}
@end

static MyAppDelegate *globalDelegate = nil;

shared_ptr<JSObject> Window::construct() {
    
    NSApplication *app = [NSApplication sharedApplication];
    globalDelegate = [[MyAppDelegate alloc] init];
    globalDelegate.callbacks = [NSMutableArray array];
    [app setDelegate:globalDelegate];

    NSRect frame = NSMakeRect(0, 0, 0, 0);
    globalDelegate.window = [[NSWindow alloc] /*init*/
        initWithContentRect:frame
                  styleMask:(NSWindowStyleMaskTitled |
                             NSWindowStyleMaskClosable |
                             NSWindowStyleMaskResizable)
                    backing:NSBackingStoreBuffered
                      defer:NO];

    [globalDelegate.window makeKeyAndOrderFront:nil];
    
    // obj->set_builtin_value("constructor"
    obj->set_builtin_value("constructor", Value::native([this](const std::vector<Value>& args) -> Value {
        // first arg is window title
        setTitle(args[0].toString());
        // second arg is width
        int width = args[1].numberValue;
        // third arg is window height
        int height = args[2].numberValue;
        
        int x = 100;
        int y = 100;
        
        NSRect newFrame = NSMakeRect(x, y, width, height);
        [globalDelegate.window setFrame:newFrame display:YES];
        
        return Value();
        
    }));

    obj->set_builtin_value("setTitle", Value::native([this](const std::vector<Value>& args) -> Value {
        setTitle(args[0].stringValue);
        return Value();
    }));

    obj->set_builtin_value("run", Value::native([this](const std::vector<Value>& args) -> Value {
        run();
        return Value();
    }));
    
    obj->set_builtin_value("setFrame", Value::native([this](const std::vector<Value>& args) -> Value {
        if (args.size() >= 4)
            setPosition(args[0].numberValue, args[1].numberValue, args[2].numberValue, args[3].numberValue);
        return Value();
    }));

    obj->set_builtin_value("addComponent", Value::native([this](const std::vector<Value>& args) -> Value {
        addComponent(args[0]);
        return Value();
    }));

    return obj;
}

void Window::setTitle(std::string title) {
    [globalDelegate.window setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

void Window::setPosition(float x, float y, float width, float height) {
    NSRect newFrame = NSMakeRect(x, y, width, height);
    [globalDelegate.window setFrame:newFrame display:YES];
}

void Window::addComponent(Value object) {
    auto control_view = object.objectValue->get("view").as<NSView*>();
    
//    NSRect contentBounds = contentView.bounds;
//    CGFloat topY = contentBounds.size.height - subview.frame.size.height;
//    subview.frame = NSMakeRect(0, topY, subview.frame.size.width, subview.frame.size.height);
//    [contentView addSubview:subview];

    [[globalDelegate.window contentView] addSubview: control_view];
}

void Window::run() {
    NSApplication *app = [NSApplication sharedApplication];
    [app run];
}
