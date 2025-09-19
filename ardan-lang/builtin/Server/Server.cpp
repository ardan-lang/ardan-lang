//
//  Server.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 18/09/2025.
//

#include "Server.hpp"

std::shared_ptr<JSObject> Server::construct() {
    
    obj->set_builtin_value("listen", Value::native([this](const vector<Value>& args) -> Value {
        if (args.size() < 2) throw runtime_error("listen expects (port, callback)");
        int port = (int)args[0].numberValue;
        Value listenCallback = args[1];

        if (running) throw runtime_error("Server already running");
        running = true;

        // create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) throw runtime_error("Failed to create socket");

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            close(server_fd);
            server_fd = -1;
            throw runtime_error("Failed to bind socket");
        }
        if (listen(server_fd, 128) < 0) {
            close(server_fd);
            server_fd = -1;
            throw runtime_error("Failed to listen");
        }

        // set non-blocking
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

        // Call the "listening started" callback (on interpreter thread)
        // httpServer.listen(4201, () => print(`Listening on port ${port}`));
        listenCallback.functionValue({});

        // Register server_fd with event loop (onReadable will accept connections)
        event_loop->addSocket(server_fd,
            [this](int fd) {
                sockaddr_in client;
                socklen_t clientlen = sizeof(client);
                int client_fd = accept(fd, (sockaddr*)&client, &clientlen);
                if (client_fd < 0) return;

                // Make client non-blocking
                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                // Build req/res objects
                auto req_obj = make_shared<JSObject>();
                auto res_obj = make_shared<JSObject>();

                Value req = Value::object(req_obj);
                Value res = Value::object(res_obj);

                // This is res.writeHead. Writes HTTP heads
                // res.writeHead(200, {"Content-Type": "text/plain"});
                res_obj->set_builtin_value("writeHead", Value::native([client_fd](const vector<Value>& args)->Value {
                    // For now, ignore status code and headers map; send a fixed header
                    std::string headers = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
                    ::send(client_fd, headers.c_str(), headers.size(), 0);
                    return Value::nullVal();
                }));

                // res.end
                res_obj->set_builtin_value("end", Value::native([client_fd](const vector<Value>& args)->Value {
                    if (!args.empty()) {
                        std::string body = args[0].toString();
                        ::send(client_fd, body.c_str(), body.size(), 0);
                    }
                    close(client_fd);
                    return Value::nullVal();
                }));

                // wait for client data
                event_loop->addSocket(client_fd,
                    [this, req, res](int cfd) {
                        char buf[4096];
                        ssize_t n = ::read(cfd, buf, sizeof(buf));
                        if (n > 0) {
                            // TODO: parse HTTP; for now, set raw body
                            auto req_obj = req.objectValue;
                            req_obj->set_builtin_value("body", Value::str(std::string(buf, n)));

                            // Dispatch "request" event callback
                            auto it = event_callbacks.find("request");
                            if (it != event_callbacks.end()) {
                                Value cb = it->second;
                                event_loop->post([cb](std::vector<Value> args)->Value {
                                    cb.functionValue(args);
                                    return Value::nullVal();
                                }, {req, res});
                            }
                        } else {
                            // cleanup
                            event_loop->removeSocket(cfd);
                            close(cfd);
                        }
                    },
                    nullptr // no write callback
                );
            },
            nullptr // no server write callback
        );

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

    return obj;
}

Server::~Server() {
    running = false;
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
}
