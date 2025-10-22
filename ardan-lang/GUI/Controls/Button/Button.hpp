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

#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

using namespace std;

// Forward declare ObjC classes as opaque types to avoid importing Cocoa
#ifdef __OBJC__
@class NSButton;
#else
typedef void NSButton;
#endif

class Button : public JSClass {
public:
    Button() {
        is_native = true;
    }

    shared_ptr<JSObject> obj;

    shared_ptr<JSObject> construct() override;

    void setTitle(std::string title);
    void setPosition(float x, float y, float width, float height);
    void onClick(std::function<void()> callback);
    
private:
    NSButton *button = nullptr;
    std::function<void()> clickHandler;
};

#endif /* Button_hpp */
