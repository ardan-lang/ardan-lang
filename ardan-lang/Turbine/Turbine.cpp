//
//  BytecodeLowering.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 21/07/2026.
//

#include "Turbine.hpp"

/**
 * Load value from register into the accumulator
 * acc = register value
 */
void Turbine::loadIntoAccumulator(const std::shared_ptr<IRValue>& v) {
    emitByte(Bytecode::kLdar);
    emitU32(regFor(v));
}

/**
 * Load value from the accumulator to the register
 * register = accumulator
 */
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

Compiled AssemblyLine::start(IRModule &irModule) {

    Compiled compiled;
    
    unordered_map<IRFunction*, int> functionIndex;
    for (size_t i = 0; i < irModule.functions.size(); i++) {
        functionIndex[irModule.functions[i].get()] = static_cast<int>(i);
    }

    for (int i = 0; i < irModule.functions.size(); i++) {

        Turbine turbine(&functionIndex);

        IRFunction* currentFunction = irModule.functions[i].get();
        
        if (currentFunction->entry_point) {
            compiled.entry_index = static_cast<int>(modules.size());
        }
        
        BytecodeModule bytecode_module = turbine.start(currentFunction);
        bytecode_module.constantPool = turbine.constantPool;
        
        modules.push_back(bytecode_module);
        
    }
    
    compiled.modules = modules;
    
    return compiled;
    
}

void Turbine::start(IRModule& irModule) {
        
    for (int i = 0; i < irModule.functions.size(); i++) {
        
        IRFunction* currentFunction = irModule.functions[i].get();
        
        lowerFunction(currentFunction);
        
    }
    
}

BytecodeModule Turbine::start(IRFunction* function) {
    
    bytecodeModule.id = function->name;
    
    this->lowerFunction(function);
    
    bytecodeModule.code = code;
    
    return bytecodeModule;

}

void Turbine::lowerFunction(IRFunction* function) {
    
    resolvePhis(function->blocks);
    
    for (int j = 0; j < function->blocks.size(); j++) {
        
        lowerBlock(function->blocks[j].get());
        
    }

}

void Turbine::lowerBlock(BasicBlock* block) {

    auto instructions = block->instructions;
    
    for (int j = 0; j < instructions.size() - 1; j++) {
        
        auto instruction = block->instructions[j];
        
        lowerInstruction(instruction, block, j);
        
    }
    
    flushPendingPhis(block);

    // we also have: "jump", or "if" at the end of a block
    lowerInstruction(instructions.back(), block, instructions.size() - 1);

}

void Turbine::lowerInstruction(IRInstruction& instruction, BasicBlock* block, size_t instIndex) {
    
    switch (instruction.op) {
            
        case IROp::Subtract: {
            emitByte(Bytecode::kSub);
            for (int i = 0; i < instruction.operands.size(); i++) {
                emitU32(regFor(instruction.operands[i]));
            }
            storeFromAccumulator(instruction.result);
            break;
        }

        case IROp::Add: {
            emitByte(Bytecode::kAdd);
            for (int i = 0; i < instruction.operands.size(); i++) {
                emitU32(regFor(instruction.operands[i]));
            }
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::Multiply: {
            emitByte(Bytecode::kMul);
            for (int i = 0; i < instruction.operands.size(); i++) {
                emitU32(regFor(instruction.operands[i]));
            }
            storeFromAccumulator(instruction.result);

            break;
        }
            
        case IROp::Zero: {
            emitByte(Bytecode::kLdaZero);
            emitU32(0);
            
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::Constant: {
            
            // load the constant to accumulator
            emitByte(Bytecode::kLdaSmi);
            emitU32(get<short>(instruction.immediate));
            
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::StringConstant: {
            
            // load into constant pool
            Value val = toValue(get<string>(instruction.immediate));
            int const_index = (int)constantPool.constants.size() - 1;
            
            constantPool.constants.push_back(val);
            
            emitByte(Bytecode::kLdaConstant);
            emitU32(const_index);
            
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::HeapNumber: {
            
            // load into constant pool
            Value val = toValue(instruction.immediate);
            int const_index = (int)constantPool.constants.size() - 1;
            
            constantPool.constants.push_back(val);
            
            emitByte(Bytecode::kLdaConstant);
            emitU32(const_index);
            
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::Undefined: {
            emitByte(Bytecode::kLdaUndefined);
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::Null: {
            emitByte(Bytecode::kLdaNull);
            storeFromAccumulator(instruction.result);

            break;
        }
            
        case IROp::True: {
            emitByte(Bytecode::kLdaTrue);
            storeFromAccumulator(instruction.result);
            break;
        }
            
        case IROp::False: {
            emitByte(Bytecode::kLdaFalse);
            storeFromAccumulator(instruction.result);
            break;
        }

        case IROp::Call: {
            
            emitByte(Bytecode::kCallUndefinedReceiver);
            
            emitU32(regFor(instruction.operands[0])); // function name
                                    
            emitU32((int)instruction.operands.size() - 1);
            
            for (int i = 1; i < instruction.operands.size(); i++) {
                auto op = instruction.operands[i];
                emitU32(regFor(op));
            }

            storeFromAccumulator(instruction.result);
            
            break;
        }
            
        case IROp::Print: {
            emitByte(Bytecode::kPrint);
            emitU32((int)instruction.operands.size());
            
            for (int i = 0; i < instruction.operands.size(); i++) {
                auto op = instruction.operands[i];
                emitU32(regFor(op));
            }

            break;
        }
            
        case IROp::If: {
            
            // condition
            
            break;
        }
        
        case IROp::Closure: {
            int kNoContext = -1;
            emitByte(Bytecode::kCreateClosure);
            int idx = functionIndex->at(instruction.childFunction);
            emitU32(static_cast<uint32_t>(idx));
            
            if (instruction.operands.empty()) {
                emitU32(kNoContext);
            } else {
                emitU32(static_cast<uint32_t>(regFor(instruction.operands[0])));
            }
            
            storeFromAccumulator(instruction.result);
            
            break;
        }
            
        case IROp::CreateContext: {
            //
            emitByte(Bytecode::kCreateFunctionContext);
            emitU32(instruction.contextSlot);
            
            storeFromAccumulator(instruction.result);
            
            break;
        }

        case IROp::LoadCurrentContextSlot: {
            
            emitByte(Bytecode::kLdaCurrentContextSlot);
            emitU32(instruction.contextSlot);
            
            // load the value in the dst reg to the acc
            storeFromAccumulator(instruction.result);

            break;
        }
            
        case IROp::LoadContextSlot: {
            
            // LdaContextSlot [slot], depth
            // reaches for the context depth in the context chain
            // from the context, go to index slot, get the value there
            // push the value to the dst reg
            
            emitByte(Bytecode::kLdaContextSlot);
            emitU32(instruction.contextDepth);
            emitU32(instruction.contextSlot);
            
            // load the value in the dst reg to the acc
            storeFromAccumulator(instruction.result);

            break;
        }

        case IROp::StoreCurrentContextSlot: {
            
            loadIntoAccumulator(instruction.operands[0]);
            
            emitByte(Bytecode::kStaCurrentContextSlot);
            emitU32(instruction.contextSlot);

            break;
        }
            
        case IROp::StoreContextSlot: {
            
            loadIntoAccumulator(instruction.operands[0]);
            
            emitByte(Bytecode::kStaContextSlot);
            emitU32(instruction.contextDepth);
            emitU32(instruction.contextSlot);
            
            break;
        }
            
        case IROp::Return: {
            
            emitByte(Bytecode::kReturn);
            
            for (auto arg : instruction.operands) {
                emitU32(static_cast<uint32_t>(regFor(arg)));
            }
            
            break;
        }
            
        default:
            break;
    }
    
}

void Turbine::resolvePhis(vector<unique_ptr<BasicBlock>>& blocks) {
    
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
