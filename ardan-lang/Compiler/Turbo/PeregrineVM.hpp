//
//  PeregrineVM.hpp
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

#include "../BaseVM.hpp"

using namespace std;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

//struct Coroutine {
//    PeregrineVM* vm;
//    shared_ptr<TurboChunk> chunk;
//    vector<Value> args;
//    std::shared_ptr<Promise> promise;
//    int ip = 0;
//    // Add any state you need (registers, etc.)
//
//    Coroutine(PeregrineVM* vm_, shared_ptr<TurboChunk> chunk_, vector<Value> args_)
//        : vm(vm_), chunk(chunk_), args(args_), promise(std::make_shared<Promise>()) {}
//
//    void resume(Value resumeValue = Value::undefined()) {
//        // Run until next await or return
//        try {
//            Value result = vm->runFrame(*this /*or via a special coroutine frame*/);
//            promise->resolve(result);
//        } catch (const std::exception& e) {
//            promise->reject(Value::str(e.what()));
//        }
//    }
//    // More: suspend(), handling of yields, etc.
//};
//
//struct Coroutine {
//    // ...
//    void resume(Value resumeValue = Value::undefined()) {
//        // Set up VM state (registers, etc.)
//        // Loop: interpret bytecode until return/await
//        // On await, yield control, and resume later
//    }
//};

struct ExecutionContext {
    shared_ptr<Env> lexicalEnv;
    shared_ptr<Env> variableEnv;
};

class PeregrineVM : public BaseVM<PeregrineVM, TurboModule, TurboChunk> {
    
    struct CallFrame {
        shared_ptr<TurboChunk> chunk;
        size_t ip = 0;                    // instruction pointer for this frame
        
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
    PeregrineVM();
    
    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<TurboChunk> chunk, const vector<Value>& args = {});
    
    // Globals
    Env* env;
    ExecutionContext* executionCtx;
    EventLoop* event_loop;

    PeregrineVM(shared_ptr<TurboModule> module_ = nullptr);
    ~PeregrineVM();
    Value callFunction(const Value& callee, const vector<Value>& args);
    
private:
    shared_ptr<TurboModule> module_ = nullptr; // set at construction or by caller
    
    vector<CallFrame> callStack; // call frames stack
    
    shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    Value addCtor();
    void set_js_object_closure(Value objVal);
    void makeObjectInstance(Value klass, shared_ptr<JSObject> obj);
    void invokeMethod(const Value& obj_value, const string& name, const vector<Value>& args);
    Value callMethod(const Value& callee, const vector<Value>& args, const Value& js_object);

    // execute the top-most frame until it returns (OP_RETURN)
    Value runFrame(CallFrame &current_frame);
    void handleRethrow();

    vector<TryFrame> tryStack;
    deque<Value> argStack;
    
    // execution state for a run
    CallFrame* frame;
    vector<ExecutionContext*> contextStack;
    
    Instruction readInstruction();
    void init_builtins();
    Value getProperty(const Value &objVal, const string &propName);
    
    Value CreateInstance(Value klass);
    void CreateObjectLiteralProperty(const Value& obj_val, const string& prop_name, const Value& object);
    void InvokeConstructor(const Value& obj_value, const vector<Value>& args);
    Value getVariable(const string& key) const;
    void putVariable(const string& key, const Value& v) const;
    ExecutionContext* createNewExecutionContext(const Value& callee) const;
    
};

#endif /* TurboVM_hpp */
