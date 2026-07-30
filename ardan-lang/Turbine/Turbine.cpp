//
//  BytecodeLowering.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 21/07/2026.
//

#include "Turbine.hpp"

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
            
        case IROp::Add:
            
            break;
            
        case IROp::Constant:
            
            break;
            
        default:
            break;
    }
    
}
