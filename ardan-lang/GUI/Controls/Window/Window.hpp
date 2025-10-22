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
    }

    shared_ptr<JSObject> obj = make_shared<JSObject>();
    shared_ptr<JSObject> construct() override;
    
    void run();
    void addComponent(Value object);
    void setTitle(string title);
};

#endif /* Window_hpp */
