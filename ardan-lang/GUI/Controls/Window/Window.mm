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

std::shared_ptr<JSObject> Window::construct() {
    
    NSApplication *app = [NSApplication sharedApplication];
    globalDelegate = [[MyAppDelegate alloc] init];
    globalDelegate.callbacks = [NSMutableArray array];
    [app setDelegate:globalDelegate];

    NSRect frame = NSMakeRect(200, 200, 400, 300);
    globalDelegate.window = [[NSWindow alloc]
        initWithContentRect:frame
                  styleMask:(NSWindowStyleMaskTitled |
                             NSWindowStyleMaskClosable |
                             NSWindowStyleMaskResizable)
                    backing:NSBackingStoreBuffered
                      defer:NO];

    [globalDelegate.window makeKeyAndOrderFront:nil];
    [globalDelegate.window setTitle:@"Default Window"];

    obj->set_builtin_value("setTitle", Value::native([this](const std::vector<Value>& args) -> Value {
        setTitle(args[0].stringValue);
        return Value();
    }));

    obj->set_builtin_value("run", Value::native([this](const std::vector<Value>& args) -> Value {
        run();
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

void Window::addComponent(Value object) {
//    NSButton *btn = [[NSButton alloc] initWithFrame:NSMakeRect(100, 100, 120, 40)];
//    [btn setTitle:@"Click Me!"];
//    [btn setBezelStyle:NSBezelStyleRounded];
//    [btn setTarget:nil];
//    [btn setAction:@selector(performClick:)];
    auto control = dynamic_cast<View*>(object.objectValue->getKlass().get());
    [[globalDelegate.window contentView] addSubview: control->getNativeView()];
}

void Window::run() {
    NSApplication *app = [NSApplication sharedApplication];
    [app run];
}
