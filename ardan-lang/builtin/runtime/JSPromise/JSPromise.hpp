//
//  JSPromise.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 05/11/2025.
//

#ifndef JSPromise_hpp
#define JSPromise_hpp

#include <stdio.h>
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../Interpreter/Promise/Promise.hpp"

class PeregrineVM;

class JSPromise : public JSClass {
    
public:
    JSPromise(PeregrineVM* vm) : vm(vm) {
        is_native = true;
        
        set_var("resolve", Value::native([this, vm](const std::vector<Value>& args) -> Value {
            
            auto promise = make_shared<Promise>(vm);
            
            promise->resolve(args[0]);
            
            return Value::promise(promise);

        }), {});
        
    }
    
    PeregrineVM* vm;
    
    shared_ptr<JSObject> construct() override;

};

#endif /* JSPromise_hpp */
