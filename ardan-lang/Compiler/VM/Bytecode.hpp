//
//  Bytecode.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#ifndef Bytecode_hpp
#define Bytecode_hpp

#pragma once
#include <stdio.h>
#include <cstdint>

enum class OpCode : uint8_t {
    Nop = 0,

    // Constants & stack
    LoadConstant,        // u32 constant index -> push constant
    Pop,                 // pop top of stack
    Dup,                 // duplicate top of stack

    // Locals / Globals
    LoadLocal,           // push locals[index]
    StoreLocal,          // pop -> locals[index] (push value back)
    LoadGlobal,          // push globals[name]
    StoreGlobal,         // pop -> globals[name] (push value back)
    CreateGlobal,        // pop -> define new global

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
    
    StoreThisProperty,
    
    LoadUpvalue,
    StoreUpvalueVar,
    StoreUpvalueLet,
    StoreUpvalueConst,
    
    LoadThisProperty,
    
    TypeOf,
    Delete,
    InstanceOf,
    
    CreateObjectLiteralProperty,

    CreateEnum,
    SetEnumProperty,

    // Object / Array
    NewObject,           // push new empty object
    SetProperty,         // u32 constname -> pop value, obj; set obj[name] = value
    GetProperty,         // u32 constname -> pop obj; push obj[name]

    NewArray,            // push new empty array
    ArrayPush,           // pop value, arr; arr.push(value); push arr

    // Arithmetic / Unary
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Power,

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

    // Jumps
    Jump,                // u32 offset (forward)
    JumpIfFalse,         // u32 offset (forward)
    Loop,                // u32 offset (backward)

    // Calls & returns
    Call,                // u8 arg_count
    Return,              // pop & return top of stack

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
        
    CreateClassPrivatePropertyVar,
    CreateClassPublicPropertyVar,
    CreateClassProtectedPropertyVar,
    CreateClassPrivatePropertyConst,
    CreateClassPublicPropertyConst,
    CreateClassProtectedPropertyConst,

    CreateClassPrivateStaticPropertyVar,
    CreateClassPublicStaticPropertyVar,
    CreateClassProtectedStaticPropertyVar,
    CreateClassPrivateStaticPropertyConst,
    CreateClassPublicStaticPropertyConst,
    CreateClassProtectedStaticPropertyConst,

    CreateClassProtectedStaticMethod,
    CreateClassPrivateStaticMethod,
    CreateClassPublicStaticMethod,
    CreateClassProtectedMethod,
    CreateClassPrivateMethod,
    CreateClassPublicMethod,
    
    In,
    Void,

    // Debug / Sentinel
    Halt
};

const char* opcodeToString(OpCode op);

#endif /* Bytecode_hpp */
