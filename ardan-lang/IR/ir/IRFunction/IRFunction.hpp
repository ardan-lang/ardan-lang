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

//| Opcode | Meaning       |
//| ------ | ------------- |
//| Const  | constant      |
//| Add    | arithmetic    |
//| Load   | memory read   |
//| Store  | memory write  |
//| Call   | function call |

enum class IROp {

    // Control
    Start,
    End,
    Region,
    If,
    Loop,
    Merge,
    Return,
    Throw,

    // SSA
    Phi,
    EffectPhi,

    // Constants
    Constant,
    Undefined,
    Null,
    Parameter,

    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Neg,

    // Bitwise
    BitAnd,
    BitOr,
    BitXor,
    ShiftLeft,
    ShiftRight,

    // Compare
    Equal,
    NotEqual,
    LessThan,
    GreaterThan,

    // Logical
    Not,

    // Memory
    Load,
    Store,
    LoadProperty,
    StoreProperty,
    LoadElement,
    StoreElement,

    // Calls
    Call,
    CallBuiltin,

    // Allocation
    NewObject,
    NewArray,
    Closure,

    // Conversion
    ToBoolean,
    ToNumber,
    ToString,

    // Runtime
    CheckType,
    CheckBounds,
    Guard,

    // Deopt
    FrameState,
    StateValues,
    Checkpoint,

    // Misc
    Projection
};

struct IRInstruction {
    IROp op;
    std::vector<IRValue> operands; // inputs
    std::string label;
    IRValue result; // output
    std::variant<
            std::monostate,
            double,
            std::string,
            bool
        > immediate;
    
    IRInstruction(IROp o, IRValue r, std::vector<IRValue> ops)
            : op(o), result(std::move(r)), operands(std::move(ops)) {}
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
