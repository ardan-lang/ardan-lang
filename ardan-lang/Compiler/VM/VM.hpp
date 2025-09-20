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

using namespace std;

// class Value;
// class JSArray;
// class JSObject;
// class JSClass;

using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::string;

class VM {
public:
    VM() = default;

    // Run a chunk as script or function. 'args' are used to populate parameter slots.
    Value run(shared_ptr<Chunk> chunk, const vector<Value>& args = {});

    // Globals
    unordered_map<string, Value> globals;

private:
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

};

#endif /* VM_hpp */
