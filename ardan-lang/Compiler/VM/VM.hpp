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

using namespace std;

// class Value;
// class JSArray;
// class JSObject;
// class JSClass;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

struct CallFrame {
    std::shared_ptr<Chunk> chunk;
    size_t ip = 0;                    // instruction pointer for this frame
    std::vector<Value> locals;        // local slots for this frame
    size_t slotsStart = 0;            // if you want stack-based locals later (not used here)
};

class VM {
public:
    VM();
    
    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<Chunk> chunk, const vector<Value>& args = {});
    
    // Globals
    unordered_map<string, Value> globals;
    
    VM(shared_ptr<Module> module_ = nullptr);
    
    Value callFunction(Value callee, vector<Value>& args);
    
private:
    shared_ptr<Module> module_ = nullptr;               // set at construction or by caller
    
    std::vector<CallFrame> callStack;        // call frames stack
    
    // helper to pop N args into a vector (left-to-right order)
    std::vector<Value> popArgs(size_t count);
    
    // execute the top-most frame until it returns (OP_RETURN)
    Value runFrame();
    
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
    
    Value binaryAdd(const Value &a, const Value &b);
    bool isTruthy(const Value &v);
    bool equals(const Value &a, const Value &b);
    Value getProperty(const Value &objVal, const string &propName);
    void setProperty(const Value &objVal, const string &propName, const Value &val);
    Value callFunction(Value callee, const vector<Value>& args);
    
};

#endif /* VM_hpp */
