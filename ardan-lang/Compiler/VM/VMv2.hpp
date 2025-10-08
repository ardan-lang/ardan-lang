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

// --- Closure and Upvalue support ---
struct Upvalue {
    Value* location;   // Points to stack slot or closed value
    Value closed;      // When closed, stores value
    Upvalue* next = nullptr; // For linked-list of open upvalues (optional)
    bool isClosed() const { return location == &closed; }
};

struct Closure {
    shared_ptr<FunctionObject> fn;
    vector<shared_ptr<Upvalue>> upvalues;
    shared_ptr<JSObject> js_object;
};

struct CallFrame {
    shared_ptr<Chunk> chunk;
    size_t ip = 0;                    // instruction pointer for this frame
    deque<Value> locals;        // local slots for this frame
    size_t slotsStart = 0;            // if you want stack-based locals later (not used here)
    
    vector<Value> args;  // <-- Store actual call arguments here
    shared_ptr<Closure> closure;
    shared_ptr<JSObject> js_object;
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
    
    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<Chunk> chunk, const vector<Value>& args = {});
    
    // Globals
    // unordered_map<string, Value> globals;
    Env* env;
    
    VM(shared_ptr<Module> module_ = nullptr);
    ~VM();
    Value callFunction(Value callee, vector<Value>& args);
    
private:
    shared_ptr<Module> module_ = nullptr;               // set at construction or by caller
    
    vector<CallFrame> callStack;        // call frames stack
    Upvalue* openUpvalues = nullptr;
    // helper to pop N args into a vector (left-to-right order)
    std::vector<Value> popArgs(size_t count);
    shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    Value addCtor();
    
    // execute the top-most frame until it returns (OP_RETURN)
    Value runFrame(CallFrame &frame);
    void handleRethrow();
    // bool running = true;
    vector<TryFrame> tryStack;
    
    // execution state for a run
    //shared_ptr<Chunk> chunk;
    //size_t ip = 0;
    deque<Value> stack;
    //deque<Value> locals; // locals[0..maxLocals-1]
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
    Value callFunction(Value callee, const vector<Value>& args);
    int getValueLength(Value& v);
    void closeUpvalues(Value* last);
    shared_ptr<Upvalue> captureUpvalue(Value* local);
    
};

#endif /* VM_hpp */
