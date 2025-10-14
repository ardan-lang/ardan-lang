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
    Subtract, // dest, lhs, rhs
    Multiply, // dest, lhs, rhs
    Divide, // dest, lhs, rhs
    Modulo,
    Power,
    
    Call,
    Return, // src
    
    Negate,
    LogicalNot,

    // Comparisons
    Equal,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
    
    LogicalAnd,
    LogicalOr,
    NullishCoalescing,
    StrictEqual,
    StrictNotEqual,
    
    Increment,
    Decrement,

    // Bitwise
    BitAnd,
    BitOr,
    BitXor,
    ShiftLeft,
    ShiftRight,
    UnsignedShiftRight,
    
    Positive,

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
