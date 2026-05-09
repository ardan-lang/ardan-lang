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
#include "../../Interpreter/Promise/Promise.hpp"
#include "../../builtin/Server/Server.hpp"
#include "../../Interpreter/Env.h"

#include "../BaseVM.hpp"

using namespace std;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

class TurboVM : public BaseVM<TurboVM, TurboModule, TurboChunk> {
    
    struct CallFrame {
        shared_ptr<TurboChunk> chunk;
        size_t ip = 0;                    
        deque<Value> locals;        
        size_t slotsStart = 0;            
        
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
    
    Value run(shared_ptr<TurboChunk> chunk, const vector<Value>& args = {});
    
    Env* env;
    EventLoop* event_loop;

    TurboVM(shared_ptr<TurboModule> module_ = nullptr);
    ~TurboVM();
    Value callFunction(Value callee, const vector<Value>& args);
    
private:
    shared_ptr<TurboModule> module_ = nullptr; 
    
    vector<CallFrame> callStack; 
    
    vector<Value> popArgs(size_t count);
    shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    Value addCtor();
    void set_js_object_closure(Value objVal);
    void makeObjectInstance(Value klass, shared_ptr<JSObject> obj);
    void invokeConstructor(Value obj_value, vector<Value> args);
    void invokeMethod(Value obj_value, string name, vector<Value> args);
    Value callMethod(Value callee, vector<Value>& args, Value js_object);

    Upvalue* openUpvalues = nullptr;

    Value runFrame(CallFrame &current_frame);
    void handleRethrow();
    bool running = true;
    vector<TryFrame> tryStack;
    deque<Value> argStack;
    
    CallFrame* frame;
    
    Instruction readInstruction();
    void init_builtins();
    Value getProperty(const Value &objVal, const string &propName);
    void closeUpvalues(Value* last);
    shared_ptr<Upvalue> captureUpvalue(Value* local);
    
    Value CreateInstance(Value klass);
    void CreateObjectLiteralProperty(Value obj_val, string prop_name, Value object);
    void InvokeConstructor(Value obj_value, vector<Value> args);
    
    // UI
    void runCreateUIView(Instruction i);
    void runAddChildSubView(Instruction i);
    void runSetUIViewArgument(Instruction i);
    void runCallUIViewModifier(Instruction i);

};

#endif /* TurboVM_hpp */
