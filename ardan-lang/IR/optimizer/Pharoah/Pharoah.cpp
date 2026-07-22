//
//  Pharoah.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 20/07/2026.
//

#include "Pharoah.hpp"

void ConstantFold::run(std::vector<IRInstruction>& instructions) {

    vector<shared_ptr<IRValue>> constants;
    
    for (int i = 0; i < instructions.size(); i++) {
        
        IRInstruction& instruction = instructions[i];
        
        if (instruction.op == IROp::Constant) {
            //constants.push_back(instruction.label)
        }
        
        if (instruction.op == IROp::Add) {
            shared_ptr<IRValue> lhs = instruction.operands[0];
            shared_ptr<IRValue> rhs = instruction.operands[1];
            
            if (lhs->type == IRType::Number && rhs->type == IRType::Number) {
                
                // we have two numbers
                // double result = lhs->
                // we add them
                
            }
        }
        
    }
    
}

void Pharoah::start(IRModule& irModule) {
    
    for (int i = 0; i < irModule.functions.size(); i++) {
        
        IRFunction* currentFunction = irModule.functions[i].get();
        
        for (int j = 0; j < currentFunction->blocks.size(); j++) {
            
            BasicBlock* currentBlock = currentFunction->blocks[j].get();
            
            for (int k = 0; k < passes.size(); k++) {
                
                OptimizationPass* pass = passes[k];
                
                pass->run(currentBlock->instructions);
                
            }
            
        }
    }
}
