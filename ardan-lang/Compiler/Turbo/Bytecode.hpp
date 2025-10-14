//
//  Bytecode.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 13/10/2025.
//

#ifndef Bytecode_hpp
#define Bytecode_hpp

#include <stdio.h>
#include <cstdint>

namespace ArdanTurboCodeGen {

enum class OpCode : uint8_t {
    
    Nop = 0,
    LoadConst, // dest, const_index. loads from constants
    LoadVar,
    
    CreateLocalVar, // creates a new local. moves data inside slot reg into the locals.
    CreateLocalLet,
    CreateLocalConst,
    CreateGlobalVar,
    CreateGlobalLet,
    CreateGlobalConst,

    Move,
    Add, // dest, lhs, rhs
    Sub, // dest, lhs, rhs
    Mul, // dest, lhs, rhs
    Div, // dest, lhs, rhs
    Call,
    Return, // src
    
    Jump,
    JumpIfFalse,
    JumpIfTrue,
    
    NewArray,
    NewObject,
    ArrayPush,
    SetProperty,
    GetProperty,
    Throw,
    
    Halt
    
};

};

#endif /* Bytecode_hpp */
