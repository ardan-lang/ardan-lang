//
//  CallStackManager.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#ifndef CallStackManager_hpp
#define CallStackManager_hpp

#pragma once
#include <stdio.h>
#include <vector>
#include <deque>
#include <memory>
#include "TurboChunk.hpp"
#include "../../../../Interpreter/ExecutionContext/Value/Value.h"

using namespace std;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

struct TurboCallFrame {
    std::shared_ptr<TurboChunk> chunk;
    size_t ip = 0;
    vector<Value> args;
    shared_ptr<Closure> closure;
    Value registers[256];
};

class CallStackManager {
public:
    void pushFrame(TurboCallFrame&& frame);
    void popFrame();
    TurboCallFrame* top();
    bool empty() const;

    void pushArg(Value arg) {
        argStack.push_back(arg);
    }
    
    void clearArgStack() { argStack.clear(); }
    
    deque<Value> getArgStack() { return argStack; }
    vector<Value> getVectorArgStack() {
        return { argStack.begin(), argStack.end() };
    }

private:
    vector<TurboCallFrame> stack;
    deque<Value> argStack;

};

#endif /* CallStackManager_hpp */
