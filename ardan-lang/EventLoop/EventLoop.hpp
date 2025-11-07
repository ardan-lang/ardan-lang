//
//  EventLoop.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 16/09/2025.
//

#ifndef EventLoop_hpp
#define EventLoop_hpp

//#include <stdio.h>
//#include <queue>
//#include <functional>
//#include <mutex>
//#include <condition_variable>
//#include <vector>
//#include "../Interpreter/ExecutionContext/Value/Value.h"
//
//using namespace std;

//class EventLoop {
//
//    queue<QueueItem> tasks;
//    unordered_map<string, vector<Value>> parameters;
//    
//    mutex mtx;
//    condition_variable cv;
//    
//    bool running = false;
//
//public:
//    
//    void post(function<Value(vector<Value>)> fn, vector<Value> args);
//
//    void run();
//
//    void stop();
//    
//};

#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <sys/types.h>

#include "../Interpreter/ExecutionContext/Value/Value.h"

struct Task {
    std::string id;
    std::function<Value(std::vector<Value>)> fn;
};

struct SocketHandle {
    int fd;
    std::function<void(int)> onReadable;
    std::function<void(int)> onWritable;
};

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    
    // scheduling
    void post(std::function<Value(std::vector<Value>)> fn, std::vector<Value> args);
    
    // socket management (register/unregister)
    void addSocket(int fd,
                   std::function<void(int)> onReadable,
                   std::function<void(int)> onWritable = nullptr);
    
    // remove socket and close FD
    void removeSocket(int fd);
    
    // loop control (run blocks on current thread)
    void run();
    void stop();
    
    static EventLoop& getInstance() {
        static EventLoop* instance = new EventLoop(); // created once, thread-safe since C++11
        return *instance;
    }

private:
    // tasks
    std::queue<Task> tasks;
    std::unordered_map<std::string, std::vector<Value>> parameters;

    // socket handles registered with kqueue
    std::unordered_map<int, SocketHandle> socketHandles;

    // kqueue + wake pipe
    int kq_fd;
    int wake_read_fd;
    int wake_write_fd;

    // concurrency
    std::mutex mtx;
    bool running;
    
    // small helper to generate unique task ids
    size_t next_task_id();
    size_t task_counter;
};

#endif /* EventLoop_hpp */
