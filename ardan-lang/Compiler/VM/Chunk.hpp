//
//  Chunk.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#ifndef Chunk_hpp
#define Chunk_hpp

#pragma once

#include <stdio.h>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>

#include "../../Interpreter/ExecutionContext/Value/Value.h"

using namespace std;

using std::vector;
using std::shared_ptr;
using std::string;

struct Chunk {
    vector<uint8_t> code;
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

#endif /* Chunk_hpp */
