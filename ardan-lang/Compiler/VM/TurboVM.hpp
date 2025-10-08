//
//  TurboVM.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#ifndef TurboVM_hpp
#define TurboVM_hpp

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

namespace ArdanTurboVM {

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

struct CallFrame {
    shared_ptr<Chunk> chunk;
    size_t ip = 0;                    // instruction pointer for this frame
    vector<Value> locals;        // local slots for this frame
    size_t slotsStart = 0;            // if you want stack-based locals later (not used here)
    
    vector<Value> args;  // <-- Store actual call arguments here
    Env* env;
};

struct TryFrame {
    int catchIP;      // -1 if none
    int finallyIP;    // -1 if none
    int stackDepth;   // stack size at entry
    int ipAfterTry;   // where the linear try block ends (for normal flow)
};

// --- Closure and Upvalue support ---
struct Upvalue {
    Value* location;   // Points to stack slot or closed value
    Value closed;      // When closed, stores value
    Upvalue* next = nullptr; // For linked-list of open upvalues (optional)
    bool isClosed() const { return location == &closed; }
};

struct Closure {
    std::shared_ptr<FunctionObject> fn; // FunctionObject (from Value.h)
    std::vector<std::shared_ptr<Upvalue>> upvalues;
};

class TurboVM {
public:
    TurboVM();
    
    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<Chunk> chunk, const vector<Value>& args = {});
    
    // Globals
    // unordered_map<string, Value> globals;
    Env* env;
    
    TurboVM(shared_ptr<Module> module_ = nullptr);
    ~TurboVM();
    Value callFunction(Value callee, vector<Value>& args);
    
private:
    shared_ptr<Module> module_ = nullptr;               // set at construction or by caller
    
    std::vector<CallFrame> callStack;        // call frames stack
    
    // helper to pop N args into a vector (left-to-right order)
    std::vector<Value> popArgs(size_t count);
    
    // execute the top-most frame until it returns (OP_RETURN)
    Value runFrame();
    void handleRethrow();
    bool running = true;
    vector<TryFrame> tryStack;
    
    // execution state for a run
    shared_ptr<Chunk> chunk;
    size_t ip = 0;
    vector<Value> stack;
    vector<Value> locals; // locals[0..maxLocals-1]
    
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
    Value callFunction(Value callee, const vector<Value>& args);
    int getValueLength(Value& v);
    //void closeUpvalues(Value* last);
    //shared_ptr<Upvalue> captureUpvalue(Value* local);
    
};

}

#endif /* TurboVM_hpp */
