//
//  EventLoop.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 16/09/2025.
//

#ifndef EventLoop_hpp
#define EventLoop_hpp

#include <stdio.h>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "../Interpreter/ExecutionContext/Value/Value.h"

using namespace std;

struct QueueItem {
    string id;
    function<Value(vector<Value>)> fn;
};

class EventLoop {

    queue<QueueItem> tasks;
    unordered_map<string, vector<Value>> parameters;
    
    mutex mtx;
    condition_variable cv;
    
    bool running = false;

public:
    
    void post(function<Value(vector<Value>)> fn, vector<Value> args);

    void run();

    void stop();
    
};

#endif /* EventLoop_hpp */
