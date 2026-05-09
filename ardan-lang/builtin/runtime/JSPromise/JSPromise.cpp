//
//  JSPromise.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 05/11/2025.
//

#include "JSPromise.hpp"
#include "../../Compiler/Turbo/PeregrineVM.hpp"

shared_ptr<JSObject> JSPromise::construct() {
    
    auto promise = make_shared<Promise>(vm);
    
    promise->set_builtin_value("constructor", Value::native([this, promise](const std::vector<Value>& args) -> Value {

        auto resolve = Value::native([this, promise](vector<Value> args) -> Value {
            promise->resolve(args[0]);
            return Value();
        });
        
        auto reject = Value::native([this, promise](vector<Value> args) -> Value {
            promise->reject(args[0]);
            return Value();
        });

        return vm->callFunction(args[0], { resolve, reject });

    }));

    return promise;
    
}
