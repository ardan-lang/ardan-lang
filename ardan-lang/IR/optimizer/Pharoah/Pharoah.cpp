//
//  Pharoah.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 20/07/2026.
//

#include "Pharoah.hpp"

void DeadCode::run(std::vector<IRInstruction> &instructions) {
    
    vector<IRInstruction> used_instructions;
    
    unordered_map<string, int> use;
    
    for (int i = 0; i < instructions.size(); i++) {
        
        IRInstruction& instruction = instructions[i];
        
        use[instruction.result->name] = 0;
        
    }
    
    for (int i = 0; i < instructions.size(); i++) {
        
        IRInstruction& instruction = instructions[i];
        
        for (auto operand : instruction.operands) {
            use[operand.get()->name]++;
        }
                
    }

    
    // remove un-used code
    for (int i = 0; i < instructions.size(); i++) {
        
        IRInstruction& instruction = instructions[i];
        
        int used = use[instruction.result->name];
        if (used ) {
            used_instructions.push_back(instruction);
        }
    }
    
    instructions = used_instructions;


}

void ConstantFold::run(std::vector<IRInstruction>& instructions) {

    unordered_map<string, R> constants;

    for (int i = 0; i < instructions.size(); i++) {
        
        IRInstruction& instruction = instructions[i];
        
        if (instruction.op == IROp::Constant) {
            constants[instruction.result->name] = instruction.immediate;
        }
    }
    
    for (int i = 0; i < instructions.size(); i++) {
                
        IRInstruction& instruction = instructions[i];

        if (instruction.op == IROp::Add) {
            shared_ptr<IRValue> lhs = instruction.operands[0];
            shared_ptr<IRValue> rhs = instruction.operands[1];
            
            if (lhs->type == IRType::Number && rhs->type == IRType::Number) {
                
                // we have two numbers
                // double result = lhs->
                // we add them
                
                R result_lhs = constants[lhs->name];
                R result_rhs = constants[rhs->name];
                
                R result = toValue(result_lhs)
                    .numberValue + toValue(result_rhs).numberValue;
                
                // replace Add with constant
                instruction.op = IROp::Constant;
                instruction.operands.clear();
                instruction.immediate = result;
                
                constants[instruction.result->name] = result;

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
