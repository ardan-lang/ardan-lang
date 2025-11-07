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
#include "../../EventLoop/EventLoop.hpp"

class PeregrineVM;

using namespace std;

class Promise : public JSObject {
public:
    using Callback = std::function<Value(vector<Value>)>;
    using Errback  = std::function<Value(vector<Value>)>;
    
    EventLoop* loop;
    PeregrineVM* vm;
    explicit Promise(PeregrineVM* vm);

    shared_ptr<Promise> then(Value cb);
    shared_ptr<Promise> then(Callback cb);
    shared_ptr<Promise> catchError(Errback cb);

    void resolve(Value v);
    void reject(Value err);

private:
    bool resolved = false;
    bool rejected = false;
    Value value;
    Value error;

    std::vector<Callback> then_callbacks;
    std::vector<Errback>  catch_callbacks;
};

#endif /* Promise_hpp */
