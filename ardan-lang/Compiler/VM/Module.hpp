//
//  Module.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 24/09/2025.
//

#ifndef Module_hpp
#define Module_hpp

#include <stdio.h>
#include "Chunk.hpp"

struct Module {
    vector<shared_ptr<Chunk>> chunks;      // all compiled function chunks
    vector<Value> constants;               // constant pool
    uint32_t entryChunkIndex;
    uint32_t version;
    
    uint32_t addChunk(shared_ptr<Chunk> c) {
        chunks.push_back(c);
        return (uint32_t)chunks.size() - 1;
    }
    
    uint32_t addConstant(const Value &v) {
        constants.push_back(v);
        return (uint32_t)constants.size() - 1;
    }
    
};

#endif /* Module_hpp */
