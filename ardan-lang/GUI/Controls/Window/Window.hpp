//
//  Window.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

#ifndef Window_hpp
#define Window_hpp

#include <stdio.h>
#include "../View/View.hpp"

#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include <string>
#include <functional>

using namespace std;

class Window : public JSClass {
public:
    Window() {
        is_native = true;

        set_proto_vm_var("constructor", Value::native([this](const std::vector<Value>& args) -> Value {
            
            return Value();
            
        }), { "public" } );
    }

    shared_ptr<JSObject> obj = make_shared<JSObject>();
    shared_ptr<JSObject> construct() override;
    
    void run();
    void addComponent(Value object);
    void setTitle(string title);
    void setFrame(int x, int y, int w, int h);
    void getFrame(int &x, int &y, int &w, int &h) const;
    void setPosition(float x, float y, float width, float height);
    
};

#endif /* Window_hpp */
