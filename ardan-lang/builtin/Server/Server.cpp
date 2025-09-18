//
//  Server.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 18/09/2025.
//

#include "Server.hpp"

std::shared_ptr<JSObject> Server::construct() {

    obj->set_builtin_value("listen", Value::native([this](const vector<Value>& args) {
        
        if (args.size() < 2) {
            throw runtime_error("listen expects (port, callback)");
        }
        
        int port = (int)args[0].numberValue;
        Value callback = args[1];
        
        if (running) {
            throw runtime_error("Server already running");
        }
        
        running = true;
        cout << running << endl;
        
        // Start actual TCP server in a new thread
        server_thread = std::thread([this, port, callback]() {
            cout << running << endl;

            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == -1) {
                //running = false;
                throw runtime_error("Failed to create socket");
            }
            
            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                perror("setsockopt failed");
                close(server_fd);
                //running = false;
                return;
            }
            
            sockaddr_in address;
            memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);
            
            if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                close(server_fd);
                //running = false;
                throw runtime_error("Failed to bind socket");
            }
            if (listen(server_fd, 3) < 0) {
                close(server_fd);
                //running = false;
                throw runtime_error("Failed to listen on socket");
            }
            // Call callback after socket is up
            callback.functionValue({});
            cout << running << endl;

            while (running) {
                int addrlen = sizeof(address);
                int client_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);
                if (client_socket < 0) {
                    if (!running) break; // Server stopped
                    continue;
                }
                // create req/res objects (dummy for now)
                // Note: These are placeholders; HTTP parsing is not implemented.
                shared_ptr<JSObject> req_obj = make_shared<JSObject>();
                shared_ptr<JSObject> res_obj = make_shared<JSObject>();
                Value req = Value::object(req_obj);
                Value res = Value::object(res_obj);
                auto it = event_callbacks.find("request");
                if (it != event_callbacks.end()) {
                    Value cb = it->second;
                    cb.functionValue({req, res});
                }
                close(client_socket);
            }
        });
        
        return Value::nullVal();
        
    }));

    obj->set_builtin_value("on", Value::native([this](const vector<Value>& args) {
        if (args.size() < 2) {
            throw runtime_error("on expects (event, callback)");
        }
        string event = args[0].toString();
        Value callback = args[1];
        event_callbacks[event] = callback;
        return Value::nullVal();
    }));
    
    return (obj);
}

Server::~Server() {
    
    running = false;
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
}
