//
//  BytecodeLowering.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 21/07/2026.
//

#ifndef Turbine_hpp
#define Turbine_hpp

#include <stdio.h>
#include <cstdint>
#include <vector>

#include "ir/IRModule/IRModule.hpp"
#include "Interpreter/Utils/Utils.h"

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

using namespace ardan::internal::interpreter;
using namespace std;


struct CompiledFunction {
    
};

struct CompiledModule {
    
};

struct ConstantPool {
    vector<Value> constants;
};

// accumulator + register vm
struct Instruction {
    Bytecode op;
};

struct BytecodeModule {
    string id;
    vector<Instruction> insructions;
    vector<uint8_t> code;
    ConstantPool constantPool;
};

struct Compiled {
    vector<BytecodeModule> modules;
};

struct PendingPhi {
    shared_ptr<IRValue> from;
    shared_ptr<IRValue> to;
};

class AssemblyLine {
private:
    vector<BytecodeModule> modules;

public:
    Compiled start(IRModule& irModule);
};

class Turbine {

public:
    ConstantPool constantPool;
    void start(IRModule& irModule);
    BytecodeModule start(IRFunction* function);
    
private:
    vector<uint8_t> code;
    unordered_map<IRValue*, int> registerOf;
    unordered_map<BasicBlock*, vector<PendingPhi>> pendingPhis;

    void emitByte(uint8_t b) { code.push_back(b); }
    void emitByte(Bytecode b) { code.push_back(static_cast<uint8_t>(b)); }
    void emitU32(uint32_t v) { for (int i = 0; i < 4; i++) code.push_back((v >> (8 * i)) & 0xFF); }
    
    void loadIntoAccumulator(const std::shared_ptr<IRValue>& v);
    void storeFromAccumulator(const std::shared_ptr<IRValue>& v);
    int regFor(const std::shared_ptr<IRValue>& v);
    
    BytecodeModule bytecodeModule;
    
    void lowerFunction(IRFunction* function);
    void lowerBlock(BasicBlock* block);
    void lowerInstruction(IRInstruction& instruction, BasicBlock* block, size_t instIndex);
    
    void resolvePhis(vector<unique_ptr<BasicBlock>> blocks);
    void flushPendingPhis(BasicBlock* block);
};

#endif /* Turbine_hpp */
