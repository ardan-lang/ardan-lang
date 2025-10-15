//
//  VM.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#ifndef VM_hpp
#define VM_hpp

#pragma once
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>
#include <cmath>
#include <stdexcept>

#include "Bytecode.hpp"
#include "Chunk.hpp"
#include "../../Interpreter/ExecutionContext/Value/Value.h"
#include "../../Interpreter/ExecutionContext/JSArray/JSArray.h"
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../Interpreter/Utils/Utils.h"
#include "../../builtin/Print/Print.hpp"
#include "Module.hpp"

#include "../../builtin/Print/Print.hpp"
#include "../../builtin/builtin-includes.h"
#include "../../GUI/gui.h"
#include "../../Interpreter/Promise/Promise.hpp"
#include "../../builtin/Server/Server.hpp"
#include "../../Interpreter/Env.h"

using namespace std;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

struct CallFrame {
    shared_ptr<Chunk> chunk;
    size_t ip = 0;                    // instruction pointer for this frame
    deque<Value> locals;        // local slots for this frame
    size_t slotsStart = 0;            // if you want stack-based locals later (not used here)
    
    vector<Value> args;  // call arguments are stored here
    shared_ptr<Closure> closure;
};

struct TryFrame {
    int catchIP;      // -1 if none
    int finallyIP;    // -1 if none
    int stackDepth;   // stack size at entry
    int ipAfterTry;   // where the linear try block ends (for normal flow)
};

class VM {

public:
    
    VM();
    VM(shared_ptr<Module> module_ = nullptr);
    ~VM();

    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<Chunk> chunk, const vector<Value>& args = {});
    
    // Globals
    Env* env;
    EventLoop* event_loop;

    Value callFunction(Value callee, const vector<Value>& args);
    // Value callFunction(Value callee, const vector<Value>& args);

private:
    shared_ptr<Module> module_ = nullptr;               // set at construction or by caller
    
    vector<CallFrame> callStack;        // call frames stack
    Upvalue* openUpvalues = nullptr;
    // helper to pop N args into a vector (left-to-right order)
    std::vector<Value> popArgs(size_t count);
    shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    Value addCtor();
    const unordered_map<string, Value> enumerateKeys(Value obj);
    void set_js_object_closure(Value objVal);
    
    // execute the top-most frame until it returns (Return)
    Value runFrame(CallFrame &frame);
    void handleRethrow();
    vector<TryFrame> tryStack;
    
    // execution state for a run
    deque<Value> stack;
    CallFrame* frame;
    
    void makeObjectInstance(Value klass, shared_ptr<JSObject> obj);
    void invokeConstructor(Value obj_value, vector<Value> args);
    void invokeMethod(Value obj_value, string name, vector<Value> args);
    Value callMethod(Value callee, vector<Value>& args, Value js_object);
    
    void push(const Value &v) { stack.push_back(v); }
    Value pop();
    Value peek(int distance = 0);
    uint8_t readByte();
    uint32_t readUint32();
    uint8_t readUint8();
    void init_builtins();
    Value binaryAdd(const Value &a, const Value &b);
    bool isTruthy(const Value &v);
    bool equals(const Value &a, const Value &b);
    Value getProperty(const Value &objVal, const string &propName);
    void setProperty(const Value &objVal, const string &propName, const Value &val);
    void setStaticProperty(const Value &objVal, const string &propName, const Value &val);
    int getValueLength(Value& v);
    void closeUpvalues(Value* last);
    shared_ptr<Upvalue> captureUpvalue(Value* local);
    
};

#endif /* VM_hpp */
