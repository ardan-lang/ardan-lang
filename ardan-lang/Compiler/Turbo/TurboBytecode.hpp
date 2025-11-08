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
    Loop,
    
    CreateArrayLiteral,
    CreateObjectLiteral,
    CreateObjectLiteralProperty,

    ArrayPush,
    SetProperty,
    GetProperty,
    In,
    Void,
    ArraySpread,
    ObjectSpread,
    PushSpreadArg,
    
    // Dynamic property access
    GetPropertyDynamic,  // pop key, obj; push obj[key]
    Dup2,                // duplicate top 2 values
    SetPropertyDynamic,  // pop value, key, obj; set obj[key] = value

    // Classes
    NewClass,
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

    // Exception handling
    Try,
    EndTry,
    EndFinally,
    Throw,
    LoadExceptionValue,

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
    SetClosureIsLocal,
    SetClosureIndex,
    
    // GetUpvalue,
    // SetUpvalue,
    CloseUpvalue,
    LoadUpvalue,
    StoreUpvalueVar,
    StoreUpvalueLet,
    StoreUpvalueConst,

    // Misc
    ClearStack,
    ClearLocals,

    LoadThisProperty,
    StoreThisProperty,
    
    SetStaticProperty,
    CreateInstance,
    InvokeConstructor,
    GetThisProperty,
    SetThisProperty,
    GetThis,
    GetParentObject,
    SuperCall,
    
    TypeOf,
    InstanceOf,
    Delete,
    
    CreateEnum,
    SetEnumProperty,
    
    // UI
    CreateUIView,
    AddChildSubView,
    SetUIViewArgument,
    CallUIViewModifier,
    
    // for PeregrineVM
    PushLexicalEnv,
    PopLexicalEnv,
    SetExecutionContext,
    CopyIterationBinding,
    Await,
    CreatePromise,
    // end for PeregrineVM

    Halt
    
};

#endif /* TurboBytecode_hpp */
