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

#ifdef __OBJC__
@class NSView;
#else
typedef void NSView;
#endif

class View : public JSClass {
public:
    virtual NSView* getNativeView() = 0;
};

#endif /* View_hpp */
