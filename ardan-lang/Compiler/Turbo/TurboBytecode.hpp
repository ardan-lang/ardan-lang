//
//  Bytecode.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 13/10/2025.
//

#ifndef TurboBytecode_hpp
#define TurboBytecode_hpp

#include <stdio.h>
#include <cstdint>

enum class TurboOpCode : uint8_t {
    
    Nop = 0,
    LoadConst, // dest, const_index. loads from constants into dest register
    LoadVar,
    LoadLocalVar,
    LoadGlobalVar,
    
    StoreLocalVar,
    StoreGlobalVar,
    StoreLocalLet,
    StoreGlobalLet,

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
    PushArg,
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
    
    // Dynamic property access
    GetPropertyDynamic,  // pop key, obj; push obj[key]
    Dup2,                // duplicate top 2 values
    SetPropertyDynamic,  // pop value, key, obj; set obj[key] = value

    // Classes
    NewClass,

    // Exception handling
    Try,
    EndTry,
    EndFinally,
    Throw,

    // Object utilities
    EnumKeys,
    GetObjectLength,
    GetIndexPropertyDynamic,
    Debug,

    // Chunk / arguments
    LoadChunkIndex,
    LoadArgument,
    LoadArguments,
    Slice,
    LoadArgumentsLength,

    // Closures
    CreateClosure,
    GetUpvalue,
    SetUpvalue,
    CloseUpvalue,

    // Misc
    ClearStack,
    ClearLocals,

    SetStaticProperty,
    CreateInstance,
    InvokeConstructor,
    GetThisProperty,
    SetThisProperty,
    GetThis,
    GetParentObject,
    SuperCall,
    CreateObjectLiteral,

    Halt
    
};

#endif /* TurboBytecode_hpp */
