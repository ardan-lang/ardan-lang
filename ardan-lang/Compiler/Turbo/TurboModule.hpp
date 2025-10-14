//
//  Module.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 24/09/2025.
//

#ifndef TurboModule_hpp
#define TurboModule_hpp

#include <stdio.h>
#include "TurboChunk.hpp"

struct TurboModule {
    vector<shared_ptr<TurboChunk>> chunks;      // all compiled function chunks
    vector<Value> constants;               // constant pool
    uint32_t entryChunkIndex;
    uint32_t version;
    
    uint32_t addChunk(shared_ptr<TurboChunk> c) {
        chunks.push_back(c);
        return (uint32_t)chunks.size() - 1;
    }
    
    uint32_t addConstant(const Value &v) {
        constants.push_back(v);
        return (uint32_t)constants.size() - 1;
    }
    
};

#endif /* TurboModule_hpp */
