//
//  HStack.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 28/10/2025.
//

#ifndef HStack_hpp
#define HStack_hpp

#include <stdio.h>

#include "../View/View.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

using namespace std;

class VM;
class TurboVM;

// Forward declare ObjC classes as opaque types to avoid importing Cocoa
#ifdef __OBJC__
@class NSStackView;
#else
typedef void NSStackView;
#endif

class HStack : public View {
public:
    HStack() {
        is_native = true;
    }
    
    shared_ptr<JSObject> construct() override;
    void addComponent(Value component, shared_ptr<JSObject> this_object);
    
};

#endif /* HStack_hpp */
