//
//  Button.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#ifndef Button_hpp
#define Button_hpp

#include <stdio.h>

#pragma once
// #import <Cocoa/Cocoa.h>
#include <string>
#include <memory>
#include <functional>
#include <ctime>

#include "../View/View.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

using namespace std;

class VM;
class TurboVM;

// Forward declare ObjC classes as opaque types to avoid importing Cocoa
#ifdef __OBJC__
@class NSButton;
#else
typedef void NSButton;
#endif

class Button : public View {
public:
    Button() {
        is_native = true;
        
        set_proto_vm_var("constructor", Value::native([this](const std::vector<Value>& args) -> Value {
            
            return Value();
            
        }), { "public" } );

    }

    unordered_map<string, shared_ptr<JSObject>> button_objects;
    shared_ptr<JSObject> construct() override;

    void setTitle(std::string title, shared_ptr<JSObject> button);
    void setPosition(float x, float y, float width, float height, shared_ptr<JSObject> obj_button);
    void onClick(std::function<void()> callback, shared_ptr<JSObject> obj_button);
    
    NSView* getNativeView() override;
    
private:
    // NSButton *button = nullptr;
    std::function<void()> clickHandler;
};

#endif /* Button_hpp */
