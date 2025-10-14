//
//  Chunk.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#ifndef TurboChunk_hpp
#define TurboChunk_hpp

#pragma once

#include <stdio.h>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>

#include "../../Interpreter/ExecutionContext/Value/Value.h"
#include "./TurboBytecode.hpp"

using namespace std;

using std::vector;
using std::shared_ptr;
using std::string;

struct Instruction {
    TurboOpCode op;
    uint8_t a, b, c;
    Instruction(TurboOpCode op, uint8_t a = 0, uint8_t b = 0, uint8_t c = 0)
    : op(op), a(a), b(b), c(c) {}
};

struct TurboChunk {
    vector<Instruction> code;
    vector<Value> constants;
    
    // metadata for functions
    uint32_t maxLocals = 0;   // number of local slots required
    uint32_t arity = 0;       // parameter count (for functions)
    string name;              // optional: function/script name
    
    int addConstant(const Value &v);
    
    void writeByte(uint8_t b);
    
    void writeUint32(uint32_t v);
    
    void writeUint8(uint8_t v);
    
    size_t size() const;
    
};

#endif /* TurboChunk_hpp */
