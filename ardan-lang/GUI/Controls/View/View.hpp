//
//  View.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#ifndef View_hpp
#define View_hpp

#include <stdio.h>

#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

using namespace std;

class VM;
class TurboVM;

#ifdef __OBJC__
@class NSView;
#else
typedef void NSView;
#endif

class View : public JSClass {
    
public:
    
    View() {
        is_native = true;
    }
    
    virtual NSView* getNativeView() {
        return nullptr;
    };
    
    shared_ptr<JSObject> construct() override;
    void addComponent(Value object, shared_ptr<JSObject> this_object);
    
};

#endif /* View_hpp */
