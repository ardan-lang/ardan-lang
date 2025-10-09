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

enum class OpCode_v2 : uint8_t {
    OP_NOP = 0,

    // constants & stack
    LoadConstant,        // u32 constant index -> push constant
    OP_POP,             // pop stack
    OP_DUP,             // duplicate top

    // locals / globals
    OP_GET_LOCAL,       // u32 local index -> push locals[index]
    OP_SET_LOCAL,       // u32 local index -> pop -> locals[index] (push value back)
    OP_GET_GLOBAL,      // u32 constname -> push globals[name]
    OP_SET_GLOBAL,      // u32 constname -> pop -> globals[name] (push value back)
    OP_DEFINE_GLOBAL,   // u32 constname -> pop -> define global name

    // object/array helpers
    OP_NEW_OBJECT,      // push new empty object
    OP_SET_PROPERTY,    // u32 constname -> pops value, obj; sets obj[name] = value; pushes obj
    OP_GET_PROPERTY,    // u32 constname -> pops obj; pushes obj[name]

    OP_NEW_ARRAY,       // push new empty array
    OP_ARRAY_PUSH,      // pops value, arr; arr.push(value); pushes arr

    // binary/unary ops
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,

    OP_NEGATE,
    OP_NOT,

    // comparisons
    OP_EQUAL,
    OP_NOTEQUAL,
    OP_LESS,
    OP_LESSEQUAL,
    OP_GREATER,
    OP_GREATEREQUAL,
    
    LOGICAL_AND, // &&
    LOGICAL_OR, // ||
    NULLISH_COALESCING, // ? :
    REFERENCE_EQUAL, // ===
    STRICT_INEQUALITY, // !==
    
    OP_INCREMENT,
    OP_DECREMENT,
    
    // bitwise
    OP_BIT_AND, // &
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_SHL,
    OP_SHR,
    OP_USHR,
    
    OP_POSITIVE,
    
    // jumps
    OP_JUMP,            // u32 offset (forward)
    OP_JUMP_IF_FALSE,   // u32 offset
    OP_LOOP,            // u32 offset (backwards)

    // calls & returns
    OP_CALL,            // u8 arg_count
    OP_RETURN,          // return top of stack (pop)
    
    OP_GET_PROPERTY_DYNAMIC,
    
    OP_DUP2,
    OP_SET_PROPERTY_DYNAMIC,
    
    // class
    OP_NEW_CLASS,
    
    // try-catch-finally
    OP_TRY,
    OP_END_TRY,
    OP_END_FINALLY,
    OP_THROW,
    
    // object
    OP_ENUM_KEYS,
    OP_GET_OBJ_LENGTH,
    OP_GET_INDEX_PROPERTY_DYNAMIC,
    OP_DEBUG,
    
    // load chunk index
    OP_LOAD_CHUNK_INDEX,
    
    OP_LOAD_ARGUMENT,
    OP_LOAD_ARGUMENTS,
    OP_SLICE,
    OP_LOAD_ARGUMENTS_LENGTH,
    
    // closure
    OP_CLOSURE,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLOSE_UPVALUE,
    
    OP_CLEAR_STACK,
    OP_CLEAR_LOCALS,
    
    OP_SET_STATIC_PROPERTY,
    CreateInstance,
    InvokeConstructor,
    GetThisProperty,
    SetThisProperty,
    GetThis,
    GetParentObject,
    SuperCall,
    CreateObjectLiteral,
    
    // debug / sentinel
    OP_HALT
};

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
    Closure,
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

    // Debug / Sentinel
    Halt
};

const char* opcodeToString(OpCode op);

#endif /* Bytecode_hpp */
