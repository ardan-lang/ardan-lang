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

#include "TurboBytecode.hpp"
#include "TurboChunk.hpp"
#include "../../Interpreter/ExecutionContext/Value/Value.h"
#include "../../Interpreter/ExecutionContext/JSArray/JSArray.h"
#include "../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../Interpreter/Utils/Utils.h"
#include "../../builtin/Print/Print.hpp"
#include "TurboModule.hpp"

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

class TurboVM {
    
    struct CallFrame {
        shared_ptr<TurboChunk> chunk;
        size_t ip = 0;                    // instruction pointer for this frame
        deque<Value> locals;        // local slots for this frame
        size_t slotsStart = 0;            // if you want stack-based locals later (not used here)
        
        vector<Value> args;
        shared_ptr<Closure> closure;
        Value registers[256];
    };

    struct TryFrame {
        int catchIP;      // -1 if none
        int finallyIP;    // -1 if none
        int stackDepth;   // stack size at entry
        int ipAfterTry;   // where the linear try block ends (for normal flow)
        uint8_t regCatch;   // register index to store the thrown value
    };

public:
    TurboVM();
    
    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<TurboChunk> chunk, const vector<Value>& args = {});
    
    // Globals
    Env* env;
    
    TurboVM(shared_ptr<TurboModule> module_ = nullptr);
    ~TurboVM();
    Value callFunction(Value callee, const vector<Value>& args);
    
private:
    shared_ptr<TurboModule> module_ = nullptr; // set at construction or by caller
    
    vector<CallFrame> callStack; // call frames stack
    
    // helper to pop N args into a vector (left-to-right order)
    vector<Value> popArgs(size_t count);
    shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    Value addCtor();
    const unordered_map<string, Value> enumerateKeys(Value obj);
    void set_js_object_closure(Value objVal);
    void makeObjectInstance(Value klass, shared_ptr<JSObject> obj);
    void invokeConstructor(Value obj_value, vector<Value> args);
    void invokeMethod(Value obj_value, string name, vector<Value> args);
    Value callMethod(Value callee, vector<Value>& args, Value js_object);
    void setStaticProperty(const Value &objVal, const string &propName, const Value &val);

    Upvalue* openUpvalues = nullptr;

    // execute the top-most frame until it returns (OP_RETURN)
    Value runFrame(CallFrame &current_frame);
    void handleRethrow();
    bool running = true;
    vector<TryFrame> tryStack;
    deque<Value> argStack;
    
    // execution state for a run
    CallFrame* frame;
    
    Instruction readInstruction();
    void init_builtins();
    Value binaryAdd(const Value &a, const Value &b);
    bool isTruthy(const Value &v);
    bool equals(const Value &a, const Value &b);
    Value getProperty(const Value &objVal, const string &propName);
    void setProperty(const Value &objVal, const string &propName, const Value &val);
    int getValueLength(Value& v);
    void closeUpvalues(Value* last);
    shared_ptr<Upvalue> captureUpvalue(Value* local);
    
};

#endif /* TurboVM_hpp */
