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

    // debug / sentinel
    OP_HALT
};

const char* opcodeToString(OpCode op);

#endif /* Bytecode_hpp */
