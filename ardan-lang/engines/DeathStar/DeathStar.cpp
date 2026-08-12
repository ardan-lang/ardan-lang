//
//  DeathStar.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 01/08/2026.
//

#include "DeathStar.hpp"

Value DeathStar::reg(ardan::CallFrame& f, int index) {
    return f.registers[index];
}

uint8_t DeathStar::fetchByte(ardan::CallFrame& f) {
    return f.bytecode_module.code[f.ip++];
}

Bytecode DeathStar::fetchOp(ardan::CallFrame& f) {
    return static_cast<Bytecode>(fetchByte(f));
}

uint32_t DeathStar::fetchU32(ardan::CallFrame& f) {
    uint32_t v = 0;
    for (int i = 0; i < 4; i++) v |= static_cast<uint32_t>(fetchByte(f)) << (8 * i);
    return v;
}

uint8_t DeathStar::next(ardan::CallFrame& frame) {
    return frame.bytecode_module.code[frame.ip++];
}

void DeathStar::runProgram() {
    
    vector<BytecodeModule> modules_ = compiled.modules;
    int entry_index = compiled.entry_index;
        
    call(entry_index, {});

}

Value DeathStar::run(ardan::CallFrame& frame) {
            
    while (true) {

        Bytecode op = static_cast<Bytecode>(next(frame));

        switch (op) {

            case Bytecode::kLdaZero: {
                frame.accumulator = Value(0);
                break;
            }

            case Bytecode::kLdaSmi: {
                frame.accumulator = fetchU32(frame);
                break;
            }
                
            case Bytecode::kStar: {
                // fetch from acc, load to register
                Value register_ = fetchU32(frame);
                frame.registers[register_.numberValue] = frame.accumulator;
                break;
            }
                
            case Bytecode::kLdar: {
                // fetch from register, load into acc
                Value register_ = fetchU32(frame);
                frame.accumulator = frame.registers[register_.numberValue];
                break;
            }
                
            case Bytecode::kAdd: {
                auto lhs = reg(frame, fetchU32(frame));
                auto rhs = reg(frame, fetchU32(frame));
                Value result = lhs.numberValue + rhs.numberValue;
                frame.accumulator = result;

                break;
            }
                
            case Bytecode::kSub: {
                auto lhs = reg(frame, fetchU32(frame));
                auto rhs = reg(frame, fetchU32(frame));
                Value result = lhs.numberValue - rhs.numberValue;
                frame.accumulator = result;
                break;
            }
                
            case Bytecode::kMul: {
                auto lhs = reg(frame, static_cast<int>(fetchU32(frame)));
                auto rhs = reg(frame, static_cast<int>(fetchU32(frame)));
                Value result = lhs.numberValue * rhs.numberValue;
                frame.accumulator = result;
                break;
            }
                
            case Bytecode::kCreateClosure: {
                
                int kNoContext = -1;
                
                auto closure = make_shared<ardan::Closure>();
                int fnIdx = static_cast<int>(fetchU32(frame));
                closure->functionIndex = fnIdx;
                
                uint32_t ctxReg = fetchU32(frame);
                                
                closure->capturedContext = (ctxReg == kNoContext) ? nullptr : std::any_cast<shared_ptr<ardan::Context>>(frame.registers[static_cast<int>(ctxReg)].anyValue);
                
                Value closure_val = Value::any(closure);
                closure_val.type = ValueType::CLOSURE;
                
                frame.accumulator = closure_val;
                
                break;
                
            }

            case Bytecode::kCreateFunctionContext: {
                int slotCount = static_cast<int>(fetchU32(frame));
                auto ctx = std::make_shared<ardan::Context>(slotCount,
                                                            frame.parentContext);
                frame.ownContext = ctx;
                frame.accumulator = Value::any(ctx);
                
                break;
            }
                
            case Bytecode::kLdaCurrentContextSlot: {
                // fetch from own context, loads the current context into acc
                int slotCount = static_cast<int>(fetchU32(frame));
                auto ctx = frame.ownContext;
                auto slot = ctx->slots[slotCount];
                
                frame.accumulator = slot;

                break;
            }

            case Bytecode::kLdaContextSlot: {
                
                // fetch from reg, loads the current context into acc
                
                int depth = static_cast<int>(fetchU32(frame));
                int slotCount = static_cast<int>(fetchU32(frame));
                
                // loop through context chain froom parent context
                shared_ptr<ardan::Context> c = frame.parentContext;
                for (int i = 0; i < depth; i++) {
                    c = c->parent;
                }
                
                auto slot = c->slots[slotCount];
                frame.accumulator = slot;

                break;
            }

            case Bytecode::kStaCurrentContextSlot: {
                
                int slotCount = static_cast<int>(fetchU32(frame));
                frame.ownContext->slots[slotCount] = frame.accumulator;

                break;
            }
                
            case Bytecode::kStaContextSlot: {
                
                int depth = static_cast<int>(fetchU32(frame));
                int slotCount = static_cast<int>(fetchU32(frame));

                // loop through context chain froom parent context
                shared_ptr<ardan::Context> c = frame.parentContext;
                for (int i = 0; i < depth; i++) {
                    c = c->parent;
                }
                
                c->slots[slotCount] = frame.accumulator;

                break;
            }
                
            case Bytecode::kCallUndefinedReceiver: {
                int calleeReg = static_cast<int>(fetchU32(frame));
                int argCount = static_cast<int>(fetchU32(frame));

                std::vector<Value> args;
                
                if (argCount < 0) {
                    uint32_t count = fetchU32(frame);
                    for (uint32_t i = 0; i < count; i++) args.push_back(reg(frame, static_cast<int>(fetchU32(frame))));
                } else {
                    for (int i = 0; i < argCount; i++) args.push_back(reg(frame, static_cast<int>(fetchU32(frame))));
                }

                Value callee = frame.registers[calleeReg];
                
                shared_ptr<ardan::Closure> closure = std::any_cast<shared_ptr<ardan::Closure>>(callee.anyValue);
                
                int fnIdx = closure->functionIndex;
                
                frame.accumulator = call(fnIdx, args, closure->capturedContext);
                
                break;
                
            }
                
            case Bytecode::kPrint: {
                std::vector<Value> args;
                int argCount = static_cast<int>(fetchU32(frame));
                for (int i = 0; i < argCount; i++) args
                    .push_back(frame.registers[ static_cast<int>(fetchU32(frame))]);
                for (auto v : args) {
                    printValue(v);
                    cout << " ";
                }
                std::cout << '\n' << std::endl;
                break;
            }
                
            case Bytecode::kReturn: {
                Value result = reg(frame, static_cast<int>(fetchU32(frame)));
                frame.accumulator = result;
                return frame.accumulator;
            }


            default:
                break;
        }
                
    }
    
}

Value DeathStar::call(int entry_index,
                     vector<Value> args,
                     shared_ptr<ardan::Context> capturedCtx) {
    
    ardan::CallFrame frame;
    frame.parentContext = capturedCtx;
    frame.bytecode_module = compiled.modules[entry_index];
    
    return run(frame);
    
}
