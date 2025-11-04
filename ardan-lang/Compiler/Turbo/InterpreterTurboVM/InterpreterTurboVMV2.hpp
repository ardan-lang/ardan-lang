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
#include "../../../Interpreter/ExecutionContext/Value/Value.h"
#include "../../../Interpreter/ExecutionContext/JSArray/JSArray.h"
#include "../../../Interpreter/ExecutionContext/JSObject/JSObject.h"
#include "../../../Interpreter/ExecutionContext/JSClass/JSClass.h"
#include "../../../Interpreter/Utils/Utils.h"
#include "../../../builtin/Print/Print.hpp"
#include "TurboModule.hpp"

#include "../../../builtin/builtin-includes.h"
#include "../../../GUI/gui.h"
#include "../../../Interpreter/Promise/Promise.hpp"
#include "../../../builtin/Server/Server.hpp"
#include "../../../Interpreter/Env.h"

#include "../../BaseVM.hpp"

#include "FunctionInvoker/FunctionInvoker.hpp"
#include "ExecutionContext/ExecutionContext.hpp"
#include "CallStackManager/CallStackManager.hpp"
#include "ExceptionManager/ExceptionManager.hpp"
#include "ObjectModel/ObjectModel.hpp"

using namespace std;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

class InterpreterTurboVMV2 : public BaseVM<InterpreterTurboVMV2, TurboModule, TurboChunk> {
    
//    struct TryFrame {
//        int catchIP;      // -1 if none
//        int finallyIP;    // -1 if none
//        int stackDepth;   // stack size at entry
//        int ipAfterTry;   // where the linear try block ends (for normal flow)
//        uint8_t regCatch;   // register index to store the thrown value
//    };
    
public:
    InterpreterTurboVMV2();
    
    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<TurboChunk> chunk, const vector<Value>& args = {});
    
    // Globals
    Env* env;
    ExecutionContext* executionCtx;
    EventLoop* event_loop;
    
    InterpreterTurboVMV2(shared_ptr<TurboModule> module_ = nullptr);
    ~InterpreterTurboVMV2();
    Value callFunction(const Value& callee, const vector<Value>& args);
    
private:
    shared_ptr<TurboModule> module_ = nullptr; // set at construction or by caller
    
    CallStackManager callStackManager;
    EnvironmentManager envManager;
    ExceptionManager exceptionManager;
    unique_ptr<FunctionInvoker> invoker;
    unique_ptr<ObjectModel> objectModel;
    
    // shared_ptr<JSObject> createJSObject(shared_ptr<JSClass> klass);
    Value addCtor();
//    void set_js_object_closure(Value objVal);
//    void makeObjectInstance(Value klass, shared_ptr<JSObject> obj);
    void invokeMethod(const Value& obj_value, const string& name, const vector<Value>& args);
    Value callMethod(const Value& callee, const vector<Value>& args, const Value& js_object);
    
    // execute the top-most frame until it returns (OP_RETURN)
    Value runFrame(CallFrame &current_frame);
    Value runFrameRunner(CallFrame& frame);
    void handleRethrow();
    
//    vector<TryFrame> tryStack;
//    deque<Value> argStack;
    
    // execution state for a run
//    CallFrame* frame;
//    vector<ExecutionContext*> contextStack;
        
    Instruction readInstruction(CallFrame* frame);
    void init_builtins();
    //Value getProperty(const Value &objVal, const string &propName);
    
//    Value CreateInstance(Value klass);
//    void CreateObjectLiteralProperty(const Value& obj_val, const string& prop_name, const Value& object);
    void InvokeConstructor(const Value& obj_value, const vector<Value>& args);
//    Value getVariable(const string& key) const;
//    void putVariable(const string& key, const Value& v) const;
    //ExecutionContext* createNewExecutionContext(const Value& callee) const;
        
};

#endif /* TurboVM_hpp */
