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

#import <Cocoa/Cocoa.h>
#include <objc/runtime.h>

#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include <string>
#include <functional>

using namespace std;

class Button : public JSClass {
public:
    
    shared_ptr<JSObject> obj = std::make_shared<JSObject>();
    shared_ptr<JSObject> construct() override;

    using OnClickHandler = std::function<void()>;

    Button(const string& title);
    ~Button();

    // No copy (NSButton isn’t copyable)
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;

    // Move support (optional)
    Button(Button&& other) noexcept;
    Button& operator=(Button&& other) noexcept;

    void setTitle(const string& title);
    void setEnabled(bool enabled);
    void setOnClick(OnClickHandler handler);

    void show();
    void hide();

    // Returns underlying Cocoa button (if needed)
    void* nativeHandle() const;

private:
    NSButton* _button;
    OnClickHandler _onClick;

    void setup();
    void attachAction();
};

#endif /* Button_hpp */
