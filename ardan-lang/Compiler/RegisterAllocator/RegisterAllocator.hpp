//
//  RegisterAllocator.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 14/05/2026.
//

#ifndef RegisterAllocator_hpp
#define RegisterAllocator_hpp

#include <stdio.h>
#include <vector>

using namespace std;

enum class BindingKind {
    Var,
    Let,
    Const,
};

class RegisterAllocator {
    uint32_t nextReg = 0; // reserve 0 for special uses if needed
    vector<uint32_t> freeRegs;
public:
    uint32_t alloc() {
        if (!freeRegs.empty()) { uint32_t r = freeRegs.back(); freeRegs.pop_back(); return r; }
        return nextReg++;
    }
    void free(uint32_t r) {
        if (r==0) return; // don't free 0
        freeRegs.push_back(r);
    }
    void reset() { nextReg = 1; freeRegs.clear(); }
    int getNextReg() { return nextReg; }
};

#endif /* RegisterAllocator_hpp */
