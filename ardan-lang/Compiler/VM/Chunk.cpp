//
//  Chunk.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "Chunk.hpp"

int Chunk::addConstant(const Value &v) {
    constants.push_back(v);
    return (int)constants.size() - 1;
}

void Chunk::writeByte(uint8_t b) {
    code.push_back(b);
}

void Chunk::writeUint32(uint32_t v) {
    // little endian
    code.push_back((v >> 0) & 0xFF);
    code.push_back((v >> 8) & 0xFF);
    code.push_back((v >> 16) & 0xFF);
    code.push_back((v >> 24) & 0xFF);
}

void Chunk::writeUint8(uint8_t v) {
    code.push_back(v);
}

size_t Chunk::size() const { return code.size(); }
