//
//  Bytecode.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "Bytecode.hpp"

const char* opcodeToString(OpCode op) {
    switch (op) {
        case OpCode::OP_NOP:            return "NOP";
        case OpCode::OP_CONSTANT:       return "CONSTANT";
        case OpCode::OP_POP:            return "POP";
        case OpCode::OP_DUP:            return "DUP";
        case OpCode::OP_GET_LOCAL:      return "GET_LOCAL";
        case OpCode::OP_SET_LOCAL:      return "SET_LOCAL";
        case OpCode::OP_GET_GLOBAL:     return "GET_GLOBAL";
        case OpCode::OP_SET_GLOBAL:     return "SET_GLOBAL";
        case OpCode::OP_DEFINE_GLOBAL:  return "DEFINE_GLOBAL";
        case OpCode::OP_NEW_OBJECT:     return "NEW_OBJECT";
        case OpCode::OP_SET_PROPERTY:   return "SET_PROPERTY";
        case OpCode::OP_GET_PROPERTY:   return "GET_PROPERTY";
        case OpCode::OP_NEW_ARRAY:      return "NEW_ARRAY";
        case OpCode::OP_ARRAY_PUSH:     return "ARRAY_PUSH";
        case OpCode::OP_ADD:            return "ADD";
        case OpCode::OP_SUB:            return "SUB";
        case OpCode::OP_MUL:            return "MUL";
        case OpCode::OP_DIV:            return "DIV";
        case OpCode::OP_MOD:            return "MOD";
        case OpCode::OP_POW:            return "POW";
        case OpCode::OP_NEGATE:         return "NEGATE";
        case OpCode::OP_NOT:            return "NOT";
        case OpCode::OP_EQUAL:          return "EQUAL";
        case OpCode::OP_NOTEQUAL:       return "NOTEQUAL";
        case OpCode::OP_LESS:           return "LESS";
        case OpCode::OP_LESSEQUAL:      return "LESSEQUAL";
        case OpCode::OP_GREATER:        return "GREATER";
        case OpCode::OP_GREATEREQUAL:   return "GREATEREQUAL";
        case OpCode::OP_JUMP:           return "JUMP";
        case OpCode::OP_JUMP_IF_FALSE:  return "JUMP_IF_FALSE";
        case OpCode::OP_LOOP:           return "LOOP";
        case OpCode::OP_CALL:           return "CALL";
        case OpCode::OP_RETURN:         return "RETURN";
        case OpCode::OP_HALT:           return "HALT";
        case OpCode::OP_INCREMENT:      return "INCREMENT";
            
        case OpCode::LOGICAL_AND: return "LOGICAL_AND"; // &&
        case OpCode::LOGICAL_OR: return "LOGICAL_OR"; // ||
        case OpCode::NULLISH_COALESCING: return "NULLISH_COALESCING";  // ? :
        case OpCode::REFERENCE_EQUAL: return "REFERENCE_EQUAL";  // ===
        case OpCode::STRICT_INEQUALITY: return "STRICT_INEQUALITY";  // !==
            
        case OpCode::OP_DECREMENT: return "OP_DECREMENT";
            
            // bitwise
        case OpCode::OP_BIT_AND: return "OP_BIT_AND";
        case OpCode::OP_BIT_OR: return "OP_BIT_OR";
        case OpCode::OP_BIT_XOR: return "OP_BIT_XOR";
        case OpCode::OP_SHL: return "OP_SHL";
        case OpCode::OP_SHR: return "OP_SHR";
        case OpCode::OP_USHR: return "OP_USHR";
        case OpCode::OP_POSITIVE: return "OP_POSITIVE";
        case OpCode::OP_GET_PROPERTY_DYNAMIC: return "OP_GET_PROPERTY_DYNAMIC";
        case OpCode::OP_DUP2: return "OP_DUP2";
        case OpCode::OP_SET_PROPERTY_DYNAMIC: return "OP_SET_PROPERTY_DYNAMIC";
        case OpCode::OP_NEW_CLASS: return "OP_NEW_CLASS";
        case OpCode::OP_TRY: return "OP_TRY";
        case OpCode::OP_END_TRY: return "OP_END_TRY";
        case OpCode::OP_END_FINALLY: return "OP_END_FINALLY";
        case OpCode::OP_THROW: return "OP_THROW";
        case OpCode::OP_ENUM_KEYS: return "OP_ENUM_KEYS";
        case OpCode::OP_GET_OBJ_LENGTH: return "OP_GET_OBJ_LENGTH";
        case OpCode::OP_GET_INDEX_PROPERTY_DYNAMIC: return "OP_GET_INDEX_PROPERTY_DYNAMIC";
        case OpCode::OP_DEBUG: return "OP_DEBUG";
        case OpCode::OP_LOAD_CHUNK_INDEX: return "OP_LOAD_CHUNK_INDEX";
        case OpCode::OP_LOAD_ARGUMENT: return "OP_LOAD_ARGUMENT";
        case OpCode::OP_LOAD_ARGUMENTS: return "OP_LOAD_ARGUMENTS";
        case OpCode::OP_SLICE: return "OP_SLICE";
        case OpCode::OP_LOAD_ARGUMENTS_LENGTH: return "OP_LOAD_ARGUMENTS_LENGTH";

        case OpCode::OP_CLOSURE:
            return "OP_CLOSURE";
        case OpCode::OP_GET_UPVALUE:
            return "OP_GET_UPVALUE";
        case OpCode::OP_SET_UPVALUE:
            return "OP_SET_UPVALUE";
        case OpCode::OP_CLOSE_UPVALUE:
            return "OP_CLOSE_UPVALUE";
    }
    return "UNKNOWN";
}
