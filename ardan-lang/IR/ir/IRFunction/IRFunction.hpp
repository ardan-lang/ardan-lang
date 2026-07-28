//
//  IRFunction.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#ifndef IRFunction_hpp
#define IRFunction_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "IR/ir/IRValue/IRValue.hpp"
#include "Interpreter/R.hpp"

//| Opcode | Meaning       |
//| ------ | ------------- |
//| Const  | constant      |
//| Add    | arithmetic    |
//| Load   | memory read   |
//| Store  | memory write  |
//| Call   | function call |

enum class IROp {
    
    Start,
    End,
    Region,
    If,
    Loop,
    Merge,
    Return,
    Throw,
    
    Phi,
    EffectPhi,
    
    Constant,
    Undefined,
    Null,
    Parameter,
    
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Neg,
    Power,
    
    BitAnd,
    BitOr,
    BitXor,
    ShiftLeft,
    ShiftRight,
    UnsignedShiftRight,
    
    Equal,
    NotEqual,
    LessThan,
    GreaterThan,
    StrictEqual,
    StrictNotEqual,
    LessThanOrEqual,
    GreaterThanOrEqual,
    
    Not,
    LogicalAnd,
    LogicalOr,
    NullishCoalescing,
    
    Load,
    Store,
    LoadProperty,
    StoreProperty,
    LoadElement,
    StoreElement,
    
    Call,
    CallBuiltin,
    
    NewObject,
    NewArray,
    Closure,
    CreateContext,
    LoadContextSlot,
    StoreContextSlot,
    
    ToBoolean,
    ToNumber,
    ToString,
    
    CheckType,
    CheckBounds,
    Guard,
    
    FrameState,
    StateValues,
    Checkpoint,
    
    Projection
};

class BasicBlock;
class IRFunction;

struct IRInstruction {
    IROp op;
    std::vector<shared_ptr<IRValue>> operands; // inputs
    std::string label;
    shared_ptr<IRValue> result; // output
    R immediate;
    
    std::vector<BasicBlock*> targets;
    IRFunction* childFunction = nullptr;
    int contextSlot = -1;
    int contextDepth = 0;
    
    IRInstruction(IROp o,
                  shared_ptr<IRValue> result,
                  std::vector<shared_ptr<IRValue>> inputs)
    : op(o), result(std::move(result)), operands(std::move(inputs)) {}
};

class BasicBlock {
public:
    std::string name;
    std::vector<IRInstruction> instructions;
    
    std::vector<BasicBlock*> successors;
    std::vector<BasicBlock*> predecessors;
    
    IRInstruction* terminator = nullptr;
    
    explicit BasicBlock(std::string n) : name(std::move(n)) {}
};

class IRFunction {
public:
    std::string name;
    std::vector<std::unique_ptr<BasicBlock>> blocks;
    
    BasicBlock* entry = nullptr;
};

#endif /* IRFunction_hpp */
