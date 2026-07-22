//
//  BytecodeLowering.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 21/07/2026.
//

#ifndef BytecodeLowering_hpp
#define BytecodeLowering_hpp

#include <stdio.h>
#include <cstdint>
#include "ir/IRModule/IRModule.hpp"

namespace ardan {
namespace internal {
namespace interpreter {

enum class Bytecode : uint8_t {
    
    kWide,
    kExtraWide,
    
    kLdaZero,
    kLdaSmi,
    kLdaUndefined,
    kLdaNull,
    kLdaTrue,
    kLdaFalse,
    kLdaConstant,
    
    kLdar,
    kStar,
    kMov,
    
    kLdaGlobal,
    kLdaGlobalInsideTypeof,
    kLdaNamedProperty,
    kLdaNamedPropertyFromSuper,
    kLdaKeyedProperty,
    
    kStaGlobalSloppy,
    kStaGlobalStrict,
    kStaNamedProperty,
    kStaNamedPropertySloppy,
    kStaNamedPropertyStrict,
    kStaNamedOwnProperty,
    kStaKeyedProperty,
    kStaKeyedPropertySloppy,
    kStaKeyedPropertyStrict,
    kStaInArrayLiteral,
    kStaDataPropertyInLiteral,
    kCollectTypeProfile,
    
    kLdaContextSlot,
    kLdaImmutableContextSlot,
    kLdaCurrentContextSlot,
    kLdaImmutableCurrentContextSlot,
    kStaContextSlot,
    kStaCurrentContextSlot,
    
    kLdaModuleVariable,
    kStaModuleVariable,
    
    kAdd,
    kSub,
    kMul,
    kDiv,
    kMod,
    kExp,
    kBitwiseOr,
    kBitwiseXor,
    kBitwiseAnd,
    kShiftLeft,
    kShiftRight,
    kShiftRightLogical,
    
    kAddSmi,
    kSubSmi,
    kMulSmi,
    kDivSmi,
    kModSmi,
    kExpSmi,
    kBitwiseOrSmi,
    kBitwiseXorSmi,
    kBitwiseAndSmi,
    kShiftLeftSmi,
    kShiftRightSmi,
    kShiftRightLogicalSmi,
    
    kInc,
    kDec,
    kNegate,
    kToBooleanLogicalNot,
    kLogicalNot,
    kTypeOf,
    kDeletePropertyStrict,
    kDeletePropertySloppy,
    kGetSuperConstructor,
    
    kTestEqual,
    kTestSameValue,
    kTestLessThan,
    kTestGreaterThan,
    kTestLessThanOrEqual,
    kTestGreaterThanOrEqual,
    kTestEqualStrict,
    kTestIn,
    kTestInstanceOf,
    kTestUndetectable,
    kTestNull,
    kTestUndefined,
    kTestTypeOf,
    
    kToName,
    kNumberToString,
    kToNumber,
    kToNumeric,
    kToObject,
    
    kJump,
    kJumpConstant,
    kJumpIfTrue,
    kJumpIfTrueConstant,
    kJumpIfFalse,
    kJumpIfFalseConstant,
    kJumpIfToBooleanTrue,
    kJumpIfToBooleanTrueConstant,
    kJumpIfToBooleanFalse,
    kJumpIfToBooleanFalseConstant,
    kJumpIfNull,
    kJumpIfNullConstant,
    kJumpIfNotNull,
    kJumpIfNotNullConstant,
    kJumpIfUndefined,
    kJumpIfUndefinedConstant,
    kJumpIfNotUndefined,
    kJumpIfNotUndefinedConstant,
    kJumpIfUndefinedOrNull,
    kJumpIfUndefinedOrNullConstant,
    kJumpIfJSReceiver,
    kJumpIfJSReceiverConstant,
    kSwitchOnSmiNoFeedback,
    
    kCallAnyReceiver,
    kCallProperty,
    kCallProperty0,
    kCallProperty1,
    kCallProperty2,
    kCallUndefinedReceiver,
    kCallUndefinedReceiver0,
    kCallUndefinedReceiver1,
    kCallUndefinedReceiver2,
    kCallWithSpread,
    kCallRuntime,
    kCallRuntimeForPair,
    kCallJSRuntime,
    kConstruct,
    kConstructWithSpread,
    
    kCreateRegExpLiteral,
    kCreateArrayLiteral,
    kCreateEmptyArrayLiteral,
    kCreateArrayFromIterable,
    kCreateObjectLiteral,
    kCreateEmptyObjectLiteral,
    kCloneObject,
    
    kCreateClosure,
    kCreateBlockContext,
    kCreateFunctionContext,
    kCreateEvalContext,
    kCreateWithContext,
    
    kCreateArguments,
    kCreateRestParameter,
    kGetTemplateObject,
    kGetIterator,
    
    kThrow,
    kReThrow,
    kReturn,
    kThrowReferenceErrorIfHole,
    kThrowSuperNotCalledIfHole,
    kThrowSuperAlreadyCalledIfNotHole,
    kThrowIfNotSuperConstructor,
    
    kDebugger,
    kIncBlockCounter,
    kAbort,
    
    kLast = kAbort
};

}
}
}

struct ConstantPool {
    vector<Value> constants;
};

// accumulator + register vm
struct Instruction {
    ardan::internal::interpreter::Bytecode op;
};

class BytecodeModule {
    int id;
    vector<Instruction> insructions;
};

class BytecodeLowering {
public:
    void start(IRModule& irModule);
    
private:
    BytecodeModule bytecodeModule;
    ConstantPool constantPool;
    void lowerBlock(BasicBlock& block);
    void lowerInstruction(IRInstruction& instruction);
};

#endif /* BytecodeLowering_hpp */
