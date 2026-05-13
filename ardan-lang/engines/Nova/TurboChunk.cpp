//
//  Chunk.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "TurboChunk.hpp"

int TurboChunk::addConstant(const Value &v) {
    constants.push_back(v);
    return (int)constants.size() - 1;
}

size_t TurboChunk::size() const { return code.size(); }
