//
//  Button.mm
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#import <Cocoa/Cocoa.h>
#include <objc/runtime.h>

#import "Button.hpp"
#include "../../../Statements/Statements.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

shared_ptr<JSObject> Button::construct() {
    
    _button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 32)];
    [_button setButtonType:NSButtonTypeMomentaryPushIn];
    [_button setBezelStyle:NSBezelStyleRounded];
    [_button setTarget:nil];
    [_button setAction:nil];
    setup();

    obj->set_builtin_value("setTitle", Value::native([this](const vector<Value>& args) -> Value {
        [_button setTitle:[NSString stringWithUTF8String:args[0].stringValue.c_str()]];
        return Value();
    }));
    
    return obj;
}

Button::Button(const std::string& title) {
    @autoreleasepool {
        _button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 32)];
        [_button setTitle:[NSString stringWithUTF8String:title.c_str()]];
        [_button setButtonType:NSButtonTypeMomentaryPushIn];
        [_button setBezelStyle:NSBezelStyleRounded];
        [_button setTarget:nil];
        [_button setAction:nil];
        setup();
    }
}

Button::~Button() {
    @autoreleasepool {
        if (_button) {
            // The NSButton is managed by ARC; no manual release
            objc_setAssociatedObject(_button, "cppButtonTrampoline", nil, OBJC_ASSOCIATION_ASSIGN);
            _button = nil;
        }
    }
}

Button::Button(Button&& other) noexcept {
    _button = other._button;
    _onClick = std::move(other._onClick);
    other._button = nullptr;
}

Button& Button::operator=(Button&& other) noexcept {
    if (this != &other) {
        _button = other._button;
        _onClick = std::move(other._onClick);
        other._button = nullptr;
    }
    return *this;
}

void Button::setup() {
    attachAction();
}

void Button::attachAction() {
    if (!_button) return;
    //ButtonActionTrampoline* trampoline = [[ButtonActionTrampoline alloc] initWithCPPButton:this];
    //[_button setTarget:trampoline];
    [_button setAction:@selector(handleClick:)];
    //objc_setAssociatedObject(_button, "cppButtonTrampoline", trampoline, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

void Button::setTitle(const std::string& title) {
    if (_button) {
        [_button setTitle:[NSString stringWithUTF8String:title.c_str()]];
    }
}

void Button::setEnabled(bool enabled) {
    if (_button) {
        [_button setEnabled:enabled];
    }
}

void Button::setOnClick(OnClickHandler handler) {
    _onClick = std::move(handler);
    attachAction();
}

void Button::show() {
    if (_button) [_button setHidden:NO];
}

void Button::hide() {
    if (_button) [_button setHidden:YES];
}

void* Button::nativeHandle() const {
    return (__bridge void*)_button;
}
