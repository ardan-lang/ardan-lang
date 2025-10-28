//
//  TextBox.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 28/10/2025.
//

#ifndef TextBox_hpp
#define TextBox_hpp

#include <stdio.h>

#include "../View/View.hpp"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"

using namespace std;

class VM;
class TurboVM;

// Forward declare ObjC classes as opaque types to avoid importing Cocoa
#ifdef __OBJC__
@class NSTextField;
#else
typedef void NSTextField;
#endif

class Text : public View {
public:
    Text() {
        is_native = true;
    }
    
    shared_ptr<JSObject> construct() override;
    void setText(string text, shared_ptr<JSObject> this_obj);
    
    void setBezeled(bool value, shared_ptr<JSObject> this_obj);
    
    void setDrawsBackground(bool value, shared_ptr<JSObject> this_obj);

    void setEditable(bool value, shared_ptr<JSObject> this_obj);

    void setSelectable(bool value, shared_ptr<JSObject> this_obj);

};

#endif /* TextBox_hpp */
