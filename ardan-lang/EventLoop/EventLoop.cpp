//
//  EventLoop.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 16/09/2025.
//

#include "EventLoop.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <iostream>
#include <stdexcept>

using namespace std;

EventLoop::EventLoop()
: kq_fd(-1), wake_read_fd(-1), wake_write_fd(-1), running(false), task_counter(0)
{
    kq_fd = kqueue();
    if (kq_fd == -1) {
        throw runtime_error(string("kqueue() failed: ") + strerror(errno));
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(kq_fd);
        throw runtime_error(string("pipe() failed: ") + strerror(errno));
    }
    wake_read_fd = pipefd[0];
    wake_write_fd = pipefd[1];

    int flags = fcntl(wake_read_fd, F_GETFL, 0);
    fcntl(wake_read_fd, F_SETFL, flags | O_NONBLOCK);

    struct kevent kev;
    EV_SET(&kev, wake_read_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
    if (kevent(kq_fd, &kev, 1, NULL, 0, NULL) == -1) {
        close(wake_read_fd);
        close(wake_write_fd);
        close(kq_fd);
        throw runtime_error(string("kevent register wake pipe failed: ") + strerror(errno));
    }
}

EventLoop::~EventLoop() {
    stop();

    if (wake_read_fd != -1) {
        ::close(wake_read_fd);
        wake_read_fd = -1;
    }
    if (wake_write_fd != -1) {
        ::close(wake_write_fd);
        wake_write_fd = -1;
    }
    if (kq_fd != -1) {
        ::close(kq_fd);
        kq_fd = -1;
    }
}

size_t EventLoop::next_task_id() {
    return ++task_counter;
}

void EventLoop::post(function<Value(vector<Value>)> fn, vector<Value> args) {

    string id;
    {
        lock_guard<mutex> lock(mtx);
        id = to_string(next_task_id());
        tasks.push({ id, std::move(fn) });
        parameters[id] = std::move(args);
    }

    uint8_t b = 1;
    ssize_t r = write(wake_write_fd, &b, 1);
    (void)r;
}

void EventLoop::addSocket(int fd,
                          function<void(int)> onReadable,
                          function<void(int)> onWritable)
{
    if (fd < 0) return;

    {
        lock_guard<mutex> lock(mtx);
        socketHandles[fd] = SocketHandle{fd, onReadable, onWritable};
    }

    if (onReadable) {
        struct kevent kev;
        EV_SET(&kev, fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
        if (kevent(kq_fd, &kev, 1, NULL, 0, NULL) == -1) {

            lock_guard<mutex> lock(mtx);
            socketHandles.erase(fd);
            throw runtime_error(string("kevent EVFILT_READ add failed: ") + strerror(errno));
        }
    }

    if (onWritable) {
        struct kevent kev;
        EV_SET(&kev, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, NULL);
        if (kevent(kq_fd, &kev, 1, NULL, 0, NULL) == -1) {

            struct kevent del;
            EV_SET(&del, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            kevent(kq_fd, &del, 1, NULL, 0, NULL);
            lock_guard<mutex> lock(mtx);
            socketHandles.erase(fd);
            throw runtime_error(string("kevent EVFILT_WRITE add failed: ") + strerror(errno));
        }
    }

    uint8_t b = 1;
    write(wake_write_fd, &b, 1);
}

void EventLoop::removeSocket(int fd) {
    if (fd < 0) return;

    struct kevent del[2];
    int delCount = 0;
    EV_SET(&del[delCount++], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&del[delCount++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);

    kevent(kq_fd, del, delCount, NULL, 0, NULL);

    {
        lock_guard<mutex> lock(mtx);
        socketHandles.erase(fd);
    }

    ::close(fd);

    uint8_t b = 1;
    write(wake_write_fd, &b, 1);
}

void EventLoop::run() {
    running = true;

    const int MAX_EVENTS = 64;
    struct kevent events[MAX_EVENTS];

    while (true) {

        Task task;
        bool hasTask = false;
        {
            lock_guard<mutex> lock(mtx);
            if (!tasks.empty()) {
                task = std::move(tasks.front());
                tasks.pop();
                auto it = parameters.find(task.id);
                if (it != parameters.end()) {
                } else {
                    parameters[task.id] = {};
                }
                hasTask = true;
            }
        }

        if (hasTask) {

            vector<Value> args;
            {
                lock_guard<mutex> lock(mtx);
                auto it = parameters.find(task.id);
                if (it != parameters.end()) {
                    args = std::move(it->second);
                    parameters.erase(it);
                }
            }

            try {
                task.fn(args);
            } catch (const std::exception &e) {
                cerr << "EventLoop task exception: " << e.what() << "\n";
            }

            continue;
        }

        {
            lock_guard<mutex> lock(mtx);
            if (!running && tasks.empty() && socketHandles.empty()) {
                break;
            }
        }

        int nev = kevent(kq_fd, NULL, 0, events, MAX_EVENTS, NULL);
        if (nev == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                cerr << "kevent error: " << strerror(errno) << "\n";
                break;
            }
        }

        for (int i = 0; i < nev; ++i) {
            struct kevent &ev = events[i];

            if (ev.ident == (uintptr_t)wake_read_fd && ev.filter == EVFILT_READ) {
                uint8_t buf[256];
                while (true) {
                    ssize_t r = read(wake_read_fd, buf, sizeof(buf));
                    if (r <= 0) break;
                }

                continue;
            }

            int fd = (int)ev.ident;

            SocketHandle handle;
            {
                lock_guard<mutex> lock(mtx);
                auto it = socketHandles.find(fd);
                if (it != socketHandles.end()) {
                    handle = it->second;
                } else {
                    continue;
                }
            }

            if (ev.filter == EVFILT_READ && handle.onReadable) {
                try {
                    handle.onReadable(fd);
                } catch (const std::exception &e) {
                    cerr << "socket onReadable exception: " << e.what() << "\n";
                }
            }

            if (ev.filter == EVFILT_WRITE && handle.onWritable) {
                try {
                    handle.onWritable(fd);
                } catch (const std::exception &e) {
                    cerr << "socket onWritable exception: " << e.what() << "\n";
                }
            }
        } 

    } 
}

void EventLoop::stop() {
    {
        lock_guard<mutex> lock(mtx);
        running = false;
    }

    uint8_t b = 1;
    write(wake_write_fd, &b, 1);
}
