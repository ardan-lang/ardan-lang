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
        
        lowerFunction(currentFunction);
        
//        for (int j = 0; j < currentFunction->blocks.size(); j++) {
//            
//            lowerBlock(*currentFunction->blocks[j].get());
//            
//        }

    }

}

void Turbine::lowerFunction(IRFunction* function) {
    
    resolvePhis(function->blocks);
    
    for (int j = 0; j < function->blocks.size(); j++) {
        
        lowerBlock(function->blocks[j].get());
        
    }

}

void Turbine::lowerBlock(BasicBlock* block) {

    for (int j = 0; j < block->instructions.size(); j++) {
        
        auto instruction = block->instructions[j];
        
        lowerInstruction(instruction);
        
    }
    
    flushPendingPhis(block);
    
}

void Turbine::lowerInstruction(IRInstruction& instruction) {
    
    switch (instruction.op) {
            
        case IROp::Add: {
            
            break;
        }
            
        case IROp::Zero: {
            emitByte(Bytecode::kLdaZero);
            emitU32(0);
            break;
        }
            
        case IROp::Constant: {
            
            // load the constant to accumulator
            emitByte(Bytecode::kLdaSmi);
            emitU32(get<int>(instruction.immediate));
            break;
        }
            
        case IROp::StringConstant: {
            
            // load into constant pool
            Value val = toValue(get<string>(instruction.immediate));
            int const_index = (int)constantPool.constants.size() - 1;
            
            constantPool.constants.push_back(val);
            
            emitByte(Bytecode::kLdaConstant);
            emitU32(const_index);
            
            break;
        }
            
        case IROp::HeapNumber: {
            
            // load into constant pool
            Value val = toValue(instruction.immediate);
            int const_index = (int)constantPool.constants.size() - 1;
            
            constantPool.constants.push_back(val);
            
            emitByte(Bytecode::kLdaConstant);
            emitU32(const_index);

            break;
        }
            
        case IROp::Call: {
            
            break;
        }
            
        default:
            break;
    }
    
}

void Turbine::resolvePhis(vector<unique_ptr<BasicBlock>> blocks) {
    
    // unordered_map<BasicBlock*, vector<pair<shared_ptr<IRValue>, shared_ptr<IRValue>>>> pendingCopies;

    for (int j = 0; j < blocks.size(); j++) {
        
        BasicBlock* block = blocks[j].get();
        
        bool removedAny = false;
        auto& instructions = block->instructions;

        for (int i = 0; i < blocks[j]->instructions.size(); i++) {
            
            IRInstruction instruction = blocks[j]->instructions[i];
            
            if (instruction.op != IROp::Phi) continue;
            
            // load from operand to phi dst
            auto dst = instruction.result;
            
            for (int k = 0; k < instruction.operands.size(); k++) {
                pendingPhis[blocks[j]->predecessors[k]]
                    .push_back({ instruction.operands[k], dst });
            }
            
            blocks[j]->instructions.erase(blocks[j]->instructions.begin() + i);
            
            removedAny = true;
            
        }
        
        if (removedAny) {
            block->terminator = instructions.empty() ? nullptr : &instructions.back();
        }
        
    }
    
    // this->pendingCopies = std::move(pendingCopies);
}

void Turbine::flushPendingPhis(BasicBlock* block) {

    vector<PendingPhi> phiCopies = pendingPhis[block];

    for (int i = 0; i < phiCopies.size(); i++) {
        
        PendingPhi pendingPhi = phiCopies[i];
        
        shared_ptr<IRValue> operand = pendingPhi.from;
        shared_ptr<IRValue> dst = pendingPhi.to;

        loadIntoAccumulator(operand);
        storeFromAccumulator(dst);
                
    }
    
}
