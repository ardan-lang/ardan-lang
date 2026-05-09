//
//  Server.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 18/09/2025.
//

#ifndef Server_hpp
#define Server_hpp

#include <stdio.h>
#include <string>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../Interpreter/ExecutionContext/Value/Value.h"
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../Statements/Statements.hpp"

#include "../../EventLoop/EventLoop.hpp"

class Server : public JSClass {
public:
    EventLoop* event_loop;
    Server(EventLoop* event_loop)
            : event_loop(event_loop) {   
            is_native = true;
        }

    shared_ptr<JSObject> obj = std::make_shared<JSObject>();

    std::shared_ptr<JSObject> construct() override;

    ~Server();

private:
    bool listening = false;
    unordered_map<string, Value> event_callbacks;

    int server_fd = -1;
    std::thread server_thread;
    std::atomic<bool> running = false;
};

#endif /* Server_hpp */
