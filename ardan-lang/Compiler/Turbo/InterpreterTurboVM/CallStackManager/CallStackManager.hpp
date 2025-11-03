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

struct CallFrame {
    std::shared_ptr<TurboChunk> chunk;
    size_t ip = 0;
    std::deque<Value> locals;
    std::vector<Value> args;
    std::shared_ptr<Closure> closure;
    Value registers[256];
};

class CallStackManager {
public:
    void pushFrame(CallFrame&& frame);
    void popFrame();
    CallFrame* top();
    bool empty() const;

private:
    std::vector<CallFrame> stack;
};

#endif /* CallStackManager_hpp */
