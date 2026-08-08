//
//  DeathStar.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 01/08/2026.
//

#include "DeathStar.hpp"

uint8_t DeathStar::next(vector<uint8_t>& code) {
    return code[frame.ip++];
}

void DeathStar::run() {
    
    vector<BytecodeModule> modules_ = compiled.modules;
    BytecodeModule module_ = modules_[0];
    vector<uint8_t> code = module_.code;
        
    while (true) {

        Bytecode op = static_cast<Bytecode>(next(code));

        switch (op) {
                
            case Bytecode::kAdd:
                
                break;
                
            case Bytecode::kCreateClosure: {
                
                break;
            }

            case Bytecode::kCreateFunctionContext: {
                
                break;
            }
                
            case Bytecode::kLdaCurrentContextSlot: {
                
                break;
            }

            case Bytecode::kLdaContextSlot: {
                
                break;
            }

            case Bytecode::kStaCurrentContextSlot: {
                
                break;
            }
                
            case Bytecode::kStaContextSlot: {
                
                break;
            }
                
            case Bytecode::kCallUndefinedReceiver: {
                
                break;
            }


            default:
                break;
        }
                
    }
    
}
