//
//  BytecodeLowering.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 21/07/2026.
//

#include "Turbine.hpp"

void Turbine::loadIntoAccumulator(const std::shared_ptr<IRValue>& v) {
    emitByte(Bytecode::kLdar);
    emitU32(regFor(v));
}

void Turbine::storeFromAccumulator(const std::shared_ptr<IRValue>& v) {
    emitByte(Bytecode::kStar);
    emitU32(regFor(v));
}

int Turbine::regFor(const std::shared_ptr<IRValue>& v) {
    
    auto it = registerOf.find(v.get());
    
    if (it != registerOf.end()) return it->second;
    int r = static_cast<int>(registerOf.size());
    registerOf[v.get()] = r;
    
    return r;
    
}

void Turbine::start(IRModule& irModule) {
    
    for (int i = 0; i < irModule.functions.size(); i++) {
        
        IRFunction* currentFunction = irModule.functions[i].get();
        
        for (int j = 0; j < currentFunction->blocks.size(); j++) {
            
            lowerBlock(*currentFunction->blocks[j].get());
            
        }

    }

}

void Turbine::lowerBlock(BasicBlock& block) {

    for (int j = 0; j < block.instructions.size(); j++) {
        
        auto instruction = block.instructions[j];
        
        lowerInstruction(instruction);
        
    }
    
}

void Turbine::lowerInstruction(IRInstruction& instruction) {
    
    switch (instruction.op) {
            
        case IROp::Add: {
            
            break;
        }
            
        case IROp::Zero: {
            break;
        }
            
        case IROp::Constant: {
            
            // load the constant to accumulator
            
            break;
        }
            
        case IROp::StringConstant: {
            // load into constant pool
            
            break;
        }
            
        case IROp::HeapNumber: {
            break;
        }
            
        default:
            break;
    }
    
}
