//
//  EventLoop.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 16/09/2025.
//

#include "EventLoop.hpp"

void EventLoop::post(function<Value(vector<Value>)> fn, vector<Value> args) {
    
    {
        
        std::lock_guard<std::mutex> lock(mtx);
        
        string id = to_string(tasks.size());
        
        tasks.push({ id, std::move(fn) });
        parameters[id] = args;
        
    }
    
    cv.notify_one();
    
}

void EventLoop::run() {
    
    running = true;
    
    while (running) {
        
        function<Value(vector<Value>)> fn;
        vector<Value> parameter;
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]{ return !tasks.empty() || !running; });
            if (!running) break;
            auto front = tasks.front();
            
            fn = std::move(front.fn);
            parameter = parameters[front.id];
            
            tasks.pop();
            
            if (tasks.empty()) {
                running = false;
            }
            
        }
        
        Value return_p = fn(parameter);
        
    }
    
}

void EventLoop::stop() {
    
    running = false;
    cv.notify_all();
    
}
