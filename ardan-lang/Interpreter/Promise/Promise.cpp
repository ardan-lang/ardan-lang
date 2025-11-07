//
//  Promise.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 17/09/2025.
//

#include "Promise.hpp"
#include "../../Compiler/Turbo/PeregrineVM.hpp"
#include "../ExecutionContext/JSObject/JSObject.h"

Promise::Promise(PeregrineVM* vm) : vm(vm) {
    loop = &EventLoop::getInstance();

    set_builtin_value("then", Value::native([this](vector<Value> args) -> Value {
        if (args.empty()) return Value::undefined();

        if (args[0].type == ValueType::CLOSURE) {
            return Value::promise(std::static_pointer_cast<JSObject>(then(args[0])));
        }

        return Value::promise(std::static_pointer_cast<JSObject>(then(args[0].functionValue)));
    }));

    set_builtin_value("catch", Value::native([this](vector<Value> args) -> Value {
        if (args.empty()) return Value::undefined();
        return Value::promise(static_pointer_cast<JSObject>(catchError(args[0].functionValue)));
    }));
}

shared_ptr<Promise> Promise::then(Value cb) {
    
    auto callback = [this, cb](vector<Value> args) -> Value {

        return vm->callFunction(cb, args);
        
    };
    
    return then(callback);

}

shared_ptr<Promise> Promise::then(Callback cb) {
    auto next = std::make_shared<Promise>(vm);
    auto wrapper = [cb, next, this](vector<Value> v) -> Value {
        try {
            Value result = cb(v);
            next->resolve(result);
        } catch (...) {
            next->reject(Value::str("Error in then callback"));
        }
        return Value::nullVal();
        //return Value::promise(shared_ptr<Promise>(then(cb)));
    };

    if (resolved) loop->post(wrapper, {value});
    else { then_callbacks.push_back(wrapper); loop->post(wrapper, {value}); }

    return next;
}

shared_ptr<Promise> Promise::catchError(Errback cb) {
    auto next = std::make_shared<Promise>(vm);
    auto wrapper = [cb, next, this](vector<Value> err) -> Value {
        try {
            Value result = cb(err);
            next->resolve(result);
        } catch (...) {
            next->reject(Value::str("Error in catch callback"));
        }
        return Value::nullVal();
    };

    if (rejected) loop->post(wrapper, {error});
    else { catch_callbacks.push_back(wrapper); loop->post(wrapper, {error}); }

    return next;
}

void Promise::resolve(Value v) {
    if (resolved || rejected) return;
    resolved = true;
    value = v;
    for (auto& cb : then_callbacks) cb({v});
    then_callbacks.clear();
}

void Promise::reject(Value err) {
    if (resolved || rejected) return;
    rejected = true;
    error = err;
    for (auto& cb : catch_callbacks) cb({err});
    catch_callbacks.clear();
}
