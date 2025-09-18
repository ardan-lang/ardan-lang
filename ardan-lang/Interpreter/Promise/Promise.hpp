//
//  Promise.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 17/09/2025.
//

#ifndef Promise_hpp
#define Promise_hpp

#include <stdio.h>
#include <vector>
#include "../ExecutionContext/Value/Value.h"
#include "../ExecutionContext/JSObject/JSObject.h"

using namespace std;

class Promise : public JSObject {
public:
    using Callback = std::function<Value(vector<Value>)>;
    using Errback  = std::function<Value(vector<Value>)>;

    Promise() {
        
        set_builtin_value("then", Value::native([this](vector<Value> args) -> Value {
            return Value::promise(shared_ptr<Promise>(then(args[0].functionValue)));
        }));
        
        set_builtin_value("catch", Value::native([this](vector<Value> args) -> Value {
            return Value::promise(shared_ptr<Promise>(catchError(args[0].functionValue)));
        }));
        
    }

    shared_ptr<Promise> then(Callback cb) {
        auto next = std::make_shared<Promise>();
        auto wrapper = [cb, next, this](vector<Value> v) -> Value {
            try {
                Value result = cb(v);
                next->resolve(result);
            } catch (...) {
                next->reject(Value("Error in then callback"));
            }
            return Value::nullVal();
            //return Value::promise(shared_ptr<Promise>(then(cb)));
        };

        if (resolved) wrapper({value});
        else then_callbacks.push_back(wrapper);

        return next;
    }

    shared_ptr<Promise> catchError(Errback cb) {
        auto next = std::make_shared<Promise>();
        auto wrapper = [cb, next, this](vector<Value> err) -> Value {
            try {
                Value result = cb(err);
                next->resolve(result);
            } catch (...) {
                next->reject(Value("Error in catch callback"));
            }
            return Value::nullVal();
        };

        if (rejected) wrapper({error});
        else catch_callbacks.push_back(wrapper);

        return next;
    }

    void resolve(Value v) {
        if (resolved || rejected) return;
        resolved = true;
        value = v;
        for (auto& cb : then_callbacks) cb({v});
        then_callbacks.clear();
    }

    void reject(Value err) {
        if (resolved || rejected) return;
        rejected = true;
        error = err;
        for (auto& cb : catch_callbacks) cb({err});
        catch_callbacks.clear();
    }

private:
    bool resolved = false;
    bool rejected = false;
    Value value;
    Value error;

    std::vector<Callback> then_callbacks;
    std::vector<Errback>  catch_callbacks;
};

#endif /* Promise_hpp */
