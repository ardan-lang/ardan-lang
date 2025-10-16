//
//  TurboVM.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "TurboVM.hpp"

TurboVM::TurboVM() {
    env = new Env();
    init_builtins();

}

TurboVM::TurboVM(shared_ptr<TurboModule> module_) : module_(module_) {

    env = new Env();
    init_builtins();
    
}

TurboVM::~TurboVM() {
    if (env != nullptr) {
        delete env;
    }
}

void TurboVM::init_builtins() {
    
    env->set_var("Math", make_shared<Math>());
    env->set_var("console", make_shared<Print>());
    env->set_var("fs", make_shared<File>());
    //env->set_var("Server", make_shared<Server>(event_loop));
    
    env->set_var("print", Value::function([this](vector<Value> args) mutable -> Value {
        Print::print(args);
        return Value::nullVal();
    }));
        
}

// Upvalue* openUpvalues = nullptr; // Head of list of open upvalues
// std::vector<std::shared_ptr<Upvalue>> closureUpvalues; // Optional for management

shared_ptr<Upvalue> TurboVM::captureUpvalue(Value* local) {
    Upvalue* prev = nullptr;
    Upvalue* up = openUpvalues;
    while (up && up->location > local) {
        prev = up;
        up = up->next;
    }
    if (up && up->location == local) {
        // Return shared_ptr wrapping the existing raw pointer - dangerous if we don't manage lifetime properly
        // But for now, assume we keep lifetime with shared_ptr elsewhere
        // We'll create a shared_ptr aliasing same pointer
        return std::shared_ptr<Upvalue>(up, [](Upvalue*){}); // no-op deleter
    }
    auto created = std::make_shared<Upvalue>();
    created->location = local;
    created->next = up;
    if (prev) prev->next = created.get(); else openUpvalues = created.get();
    return created;
}

void TurboVM::closeUpvalues(Value* last) {
    while (openUpvalues && openUpvalues->location >= last) {
        openUpvalues->closed = *openUpvalues->location;
        openUpvalues->location = &openUpvalues->closed;
        openUpvalues = openUpvalues->next;
    }
}

Instruction TurboVM::readInstruction() {
    if (frame->ip >= frame->chunk->code.size())
        throw runtime_error("Empty instruction.");
    return frame->chunk->code[frame->ip++];
}

uint32_t TurboVM::readUint32() {
    if (frame->ip + 4 > frame->chunk->code.size()) throw std::runtime_error("read past end");
    uint32_t v = 0;
//    v |= (uint32_t)chunk->code[ip++];
//    v |= (uint32_t)chunk->code[ip++] << 8;
//    v |= (uint32_t)chunk->code[ip++] << 16;
//    v |= (uint32_t)chunk->code[ip++] << 24;
    return v;
}

//uint8_t TurboVM::readUint8() {
//    return readByte();
//}

Value TurboVM::binaryAdd(const Value &a, const Value &b) {
    if (a.type == ValueType::STRING || b.type == ValueType::STRING) {
        return Value::str(a.toString() + b.toString());
    }
    return Value(a.numberValue + b.numberValue);
}

bool TurboVM::isTruthy(const Value &v) {
    if (v.type == ValueType::NULLTYPE) return false;
    if (v.type == ValueType::UNDEFINED) return false;
    if (v.type == ValueType::BOOLEAN) return v.boolValue;
    if (v.type == ValueType::NUMBER) return v.numberValue != 0;
    if (v.type == ValueType::STRING) return !v.stringValue.empty();
    // objects/arrays considered truthy
    return true;
}

bool TurboVM::equals(const Value &a, const Value &b) {
    // shallow equality similar to interpreter
    if (a.type != b.type) {
        // try numeric-string comparisons etc is omitted for brevity
        return a.toString() == b.toString();
    }
    switch (a.type) {
        case ValueType::NUMBER: return a.numberValue == b.numberValue;
        case ValueType::STRING: return a.stringValue == b.stringValue;
        case ValueType::BOOLEAN: return a.boolValue == b.boolValue;
        case ValueType::NULLTYPE:
        case ValueType::UNDEFINED:
            return true;
        default:
            // object identity
            if (a.type == ValueType::OBJECT && b.type == ValueType::OBJECT)
                return a.objectValue == b.objectValue;
            if (a.type == ValueType::ARRAY && b.type == ValueType::ARRAY)
                return a.arrayValue == b.arrayValue;
            return false;
    }
}

int TurboVM::getValueLength(Value& v) {
    
    if (v.type == ValueType::OBJECT) {
        return (int)v.objectValue->get_all_properties().size();
    }
    
    if (v.type == ValueType::ARRAY) {
        return v.arrayValue->get("length").numberValue;
    }
    
    return v.numberValue;

}

Value TurboVM::getProperty(const Value &objVal, const string &propName) {
    if (objVal.type == ValueType::OBJECT) {
        return objVal.objectValue->get(propName);
    }
    if (objVal.type == ValueType::ARRAY) {
        return objVal.arrayValue->get(propName);
    }
    if (objVal.type == ValueType::CLASS) {
        return objVal.classValue->get(propName, false);
    }
    // primitives -> string -> property? For now, undefined
    return Value::undefined();
}

void TurboVM::setProperty(const Value &objVal, const string &propName, const Value &val) {
    if (objVal.type == ValueType::OBJECT) {
        objVal.objectValue->set(propName, val, "VAR", {});
        return;
    }
    if (objVal.type == ValueType::ARRAY) {
        objVal.arrayValue->set(propName, val);
        return;
    }
    if (objVal.type == ValueType::CLASS) {
        objVal.classValue->set(propName, val, false);
        return;
    }
    throw std::runtime_error("Cannot set property on non-object");
}

Value TurboVM::run(shared_ptr<TurboChunk> chunk_, const vector<Value>& args) {
    
    auto closure = make_shared<Closure>();

    // prepare a top-level frame that will be executed by runFrame()
    CallFrame new_frame;
    new_frame.chunk = chunk_;
    new_frame.ip = 0;
    new_frame.locals.resize(chunk_->maxLocals, Value::undefined());
    new_frame.args = args;
    new_frame.closure = closure;
    
    uint32_t ncopy = std::min((uint32_t)args.size(), chunk_->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) new_frame.locals[i] = args[i];
    
    callStack.push_back(std::move(new_frame));
    Value result = runFrame(callStack.back());
    callStack.pop_back();
    return result;
    
}

//Value TurboVM::runFrame(CallFrame &current_frame) {
//    
//    if (callStack.empty()) return Value::undefined();
//
//    // Point VM at this frame's chunk/locals
//    frame = &current_frame;
//    
//}

Value TurboVM::runFrame(CallFrame &current_frame) {
    
    if (callStack.empty()) return Value::undefined();
    
    frame = &current_frame;

    while (true) {
        Instruction instruction = readInstruction();
        TurboOpCode op = instruction.op;

        switch (op) {
            case TurboOpCode::Nop:
                break;

            case TurboOpCode::LoadConst: {
                uint8_t dest = instruction.a;
                uint8_t const_index = instruction.b;
                Value val = frame->chunk->constants[const_index];
                registers[dest] = val;
                break;
            }
                
            case TurboOpCode::Move: {
                uint8_t src = instruction.b;
                uint8_t dest = instruction.a;
                registers[dest] = registers[src];
                break;
            }
                
            case TurboOpCode::CreateLocalVar: {
                uint8_t local_index = instruction.a;
                uint8_t data_reg = instruction.b;
                frame->locals[local_index] = registers[data_reg];
                break;
            }
                
            case TurboOpCode::CreateLocalLet:{
                uint8_t local_index = instruction.a;
                uint8_t data_reg = instruction.b;
                frame->locals[local_index] = registers[data_reg];
                break;
            }
                
            case TurboOpCode::CreateLocalConst:{
                uint8_t local_index = instruction.a;
                uint8_t data_reg = instruction.b;
                frame->locals[local_index] = registers[data_reg];
                break;
            }
                
            case TurboOpCode::CreateGlobalVar:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                env->set_var(name, registers[data_reg]);
                
                break;
            }
                
            case TurboOpCode::CreateGlobalLet:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                env->set_let(name, registers[data_reg]);
                
                break;
            }
                
            case TurboOpCode::CreateGlobalConst:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                env->set_const(name, registers[data_reg]);
                
                break;
            }
                
                // TurboOpCode::Add, opResultReg, lhsReg, rhsReg
            case TurboOpCode::Add: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                Value result = binaryAdd(lhs, rhs);
                registers[instruction.a] = result;
                break;
            }
                
            case TurboOpCode::Subtract: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(lhs.numberValue - rhs.numberValue);
                break;
            }
                
            case TurboOpCode::Multiply: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(lhs.numberValue * rhs.numberValue);
                break;
            }
                
            case TurboOpCode::Divide: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(lhs.numberValue / rhs.numberValue);
                break;
            }
                
            case TurboOpCode::Modulo: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(fmod(lhs.numberValue, rhs.numberValue));
                break;
            }
                
            case TurboOpCode::Power: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(pow(lhs.numberValue, rhs.numberValue));
                break;
            }
                
            case TurboOpCode::ShiftLeft: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value((int)lhs.numberValue << (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::ShiftRight: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value((int)lhs.numberValue >> (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::UnsignedShiftRight: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value((unsigned int)lhs.numberValue >> (unsigned int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::BitAnd: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value((int)lhs.numberValue & (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::BitOr: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value((int)lhs.numberValue | (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::BitXor: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value((int)lhs.numberValue ^ (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::LogicalAnd: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(isTruthy(lhs) && isTruthy(rhs));
                break;
            }
                
            case TurboOpCode::LogicalOr: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = Value(isTruthy(lhs) || isTruthy(rhs));
                break;
            }
                
            case TurboOpCode::NullishCoalescing: {
                Value lhs = registers[instruction.b];
                Value rhs = registers[instruction.c];
                registers[instruction.a] = isNullish(lhs) ? rhs : lhs;
                break;
            }
                
            case TurboOpCode::StrictEqual: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                bool isEqual = false;
                // For objects/arrays: we compare pointers
                if (a.type == b.type) {
                    if (a.type == ValueType::OBJECT)
                        isEqual = (a.objectValue == b.objectValue);
                    else if (a.type == ValueType::ARRAY)
                        isEqual = (a.arrayValue == b.arrayValue);
                    // For other types, we could default to pointer or identity check
                }
                registers[instruction.a] = Value::boolean(isEqual);
                break;
            }
                
            case TurboOpCode::StrictNotEqual: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                bool notEqual = false;
                // For numbers, strings, booleans, etc
                if (a.type != b.type) {
                    notEqual = true;
                } else {
                    // For numbers
                    if (a.type == ValueType::NUMBER)
                        notEqual = a.numberValue != b.numberValue;
                    // For strings
                    else if (a.type == ValueType::STRING)
                        notEqual = a.stringValue != b.stringValue;
                    // For booleans
                    else if (a.type == ValueType::BOOLEAN)
                        notEqual = a.boolValue != b.boolValue;
                    // For objects/arrays, compare pointers
                    else if (a.type == ValueType::OBJECT)
                        notEqual = a.objectValue != b.objectValue;
                    else if (a.type == ValueType::ARRAY)
                        notEqual = a.arrayValue != b.arrayValue;
                    // add any types we missed
                }
                registers[instruction.a] = (Value::boolean(notEqual));
                break;
            }
                
            case TurboOpCode::Decrement: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                int sum = a.numberValue - b.numberValue;
                registers[instruction.a] = Value(sum);
                break;
            }

            case TurboOpCode::Negate: {
                Value a = registers[instruction.a];
                registers[instruction.a] = Value(-a.numberValue);
                break;
            }

            case TurboOpCode::LogicalNot: {
                Value a = registers[instruction.a];
                registers[instruction.a] = Value::boolean(!isTruthy(a));
                break;
            }
                
            case TurboOpCode::Increment: {
                
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                
                int sum = a.numberValue + b.numberValue;
                
                registers[instruction.a] = Value(sum);

                break;
            }

            case TurboOpCode::Equal: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                registers[instruction.a] =  Value::boolean(equals(a,b));
                break;
            }

            case TurboOpCode::NotEqual: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                registers[instruction.a] = Value::boolean(!equals(a,b));
                break;
            }

            case TurboOpCode::LessThan: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                registers[instruction.a] = Value::boolean(a.numberValue < b.numberValue);
                break;
            }
                
            case TurboOpCode::LessThanOrEqual: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                registers[instruction.a] = Value::boolean(a.numberValue <= b.numberValue);
                break;
            }
                
            case TurboOpCode::GreaterThan: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                registers[instruction.a] = Value::boolean(a.numberValue > b.numberValue);
                break;
            }
                
            case TurboOpCode::GreaterThanOrEqual: {
                Value a = registers[instruction.a];
                Value b = registers[instruction.b];
                registers[instruction.a] = Value::boolean(a.numberValue >= b.numberValue);
                break;
            }
                                                
            case TurboOpCode::LoadLocalVar: {
                // LoadLocalVar, reg_slot, idx
                uint8_t reg = instruction.a;
                uint8_t idx = instruction.b;
                
                registers[reg] = frame->locals[idx];
                break;
            }
                
            case TurboOpCode::LoadGlobalVar: {
                
                uint8_t reg = instruction.a;
                uint8_t idx = instruction.b;
                string name = frame->chunk->constants[idx].stringValue;
                
                registers[reg] = toValue(env->get(name));

                break;
            }
                //TODO: we need to store via var, let, const
                //emit(TurboOpCode::StoreLocal, idx, reg_slot);
            case TurboOpCode::StoreLocalVar: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                
                frame->locals[idx] = registers[reg_slot];

                break;
            }
                
            case TurboOpCode::StoreLocalLet: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                
                frame->locals[idx] = registers[reg_slot];
                
                break;
            }

            case TurboOpCode::StoreGlobalVar: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                Value val = frame->chunk->constants[idx];
                string name = val.stringValue;
                
                env->set_var(name, registers[reg_slot]);

                break;
            }
                
            case TurboOpCode::StoreGlobalLet: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                Value val = frame->chunk->constants[idx];
                string name = val.stringValue;

                env->set_let(name, registers[reg_slot]);

                break;
            }
                
                // emit(TurboOpCode::NewArray, arr);
            case TurboOpCode::NewArray: {
                auto array = make_shared<JSArray>();
                registers[instruction.a] = Value::array(array);
                break;
            }
                
                // emit(TurboOpCode::ArrayPush, arr, val);
            case TurboOpCode::ArrayPush: {
                auto array = registers[instruction.a].arrayValue;
                array->push({registers[instruction.b]});
                break;
            }

                // emit(TurboOpCode::NewObject, obj);
            case TurboOpCode::NewObject: {
                auto object = make_shared<JSObject>();
                Value v = Value::object(object);

                registers[instruction.a] = v;
                break;
            }

                // emit(TurboOpCode::SetProperty, obj, emitConstant(prop.first.lexeme), val);
                // SetProperty: objReg, nameIdx, valueReg
            case TurboOpCode::SetProperty: {
                auto object = registers[instruction.a];
                Value val = frame->chunk->constants[instruction.b];
                string prop_name = val.stringValue;
                Value obj_val = registers[instruction.c];
                
                setProperty(object, prop_name, obj_val);
                registers[instruction.c] = object;

                break;
            }
                
                // SetPropertyDynamic: objReg, propReg, valueReg
                // emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, resultReg);
            case TurboOpCode::SetPropertyDynamic: {
                auto object = registers[instruction.a];
                string prop_name = registers[instruction.b].stringValue;
                setProperty(object, prop_name, registers[instruction.c]);
                
                registers[instruction.c] = object;

                break;
            }
                
                // TurboOpCode::GetPropertyDynamic, lhsReg, objReg, propReg
            case TurboOpCode::GetPropertyDynamic: {
                auto object = registers[instruction.b];
                string prop = registers[instruction.c].stringValue;
                Value val = getProperty(object, prop);
                registers[instruction.a] = val;
                break;
            }
                
                // TurboOpCode::GetProperty, lhsReg, objReg, nameIdx
            case TurboOpCode::GetProperty: {
                auto object = registers[instruction.b];
                string prop = frame->chunk->constants[instruction.c].stringValue;
                Value val = getProperty(object, prop);
                registers[instruction.a] = val;
                break;
            }
                
            case TurboOpCode::PushArg: {
                int argReg = instruction.a;
                argStack.push_back(registers[argReg]);
                break;
            }

                // emit(TurboOpCode::Call, result, funcReg, (int)argRegs.size());
//            Where:
//
//            funcReg → register containing the function (the callable object or closure)
//            argStart → the index of the first argument register
//            argCount → how many arguments are being passed
                
//            case TurboOpCode::Call: {
//                
//                uint32_t result_reg = instruction.a;
//                Value func = registers[instruction.b]; // R[a] = function
//                uint8_t argCount = instruction.c;     // number of arguments
//                int argStart = instruction.b + 1;
//                
//                vector<Value> args;
//                args.reserve(argCount);
//                for (int i = 0; i < argCount; ++i) {
//                    args.push_back(registers[argStart + i]);
//                }
//                const vector<Value> const_args = args;
//                Value result = callFunction(func, const_args);
//                registers[result_reg] = result;
//                
//                break;
//            }
                
            case TurboOpCode::Call: {
                
                int resultReg = instruction.a;
                int funcReg   = instruction.b;
                // int argc      = instruction.c;
                
                Value func = registers[funcReg];
                
                const vector<Value> const_args = {
                    argStack.begin(),
                    argStack.end() };
                Value result = callFunction(func, const_args);
                argStack.clear();
                registers[resultReg] = result;

                break;
            }
                
            case TurboOpCode::Return: {
                break;
            }

            case TurboOpCode::Halt:
                return Value::undefined();
                break;

            default:
                throw std::runtime_error("Unknown opcode in VM");
        }
    }
    
    return Value::undefined();
    
}

Value TurboVM::callFunction(Value callee, const vector<Value>& args) {
    
    if (callee.type == ValueType::FUNCTION) {
        // call host function (native or compiled wrapper)
        Value result = callee.functionValue(args);
        return result;
    }
    
    if (callee.type == ValueType::CLOSURE) {

        shared_ptr<TurboChunk> calleeChunk = module_->chunks[callee.closureValue->fn->chunkIndex];
        
        // Build new frame
        CallFrame new_frame;
        new_frame.chunk = calleeChunk;
        new_frame.ip = 0;
        // allocate locals sized to the chunk's max locals (some chunks use maxLocals)
        new_frame.locals.resize(calleeChunk->maxLocals, Value::undefined());
        new_frame.args = args;
        new_frame.closure = callee.closureValue;
        //new_frame.js_object = callee.closureValue->js_object;

        // Set frame.closure if calling a closure
        // Assuming callee has a closurePtr member for Closure shared_ptr
        // If you extended Value for closure type, set here:
        callStack.push_back(std::move(new_frame));
        // auto prev_stack = std::move(stack);
        // stack.clear();

        Value result = runFrame(callStack.back());
        
        callStack.pop_back();
        frame = &callStack.back();
        
        // stack = std::move(prev_stack);

        return result;
        
    }

    if (callee.type == ValueType::NATIVE_FUNCTION) {
        Value result = callee.nativeFunction(args);
        return result;
    }
    
    if (callee.type != ValueType::FUNCTION_REF) {
        throw runtime_error("Attempt to call non-function");
        return Value::undefined();
    }
    
    auto fn = callee.fnRef;
    
    if (!module_) {
        throw runtime_error("Module not set in VM");
        return Value::undefined();
    }
    
    if (fn->chunkIndex >= module_->chunks.size()) {
        throw runtime_error("Bad function index");
        return Value::undefined();
    }
    
    shared_ptr<TurboChunk> calleeChunk = module_->chunks[fn->chunkIndex];

    // Build new frame
    CallFrame new_frame;
    new_frame.chunk = calleeChunk;
    new_frame.ip = 0;
    // allocate locals sized to the chunk's max locals (some chunks use maxLocals)
    new_frame.locals.resize(calleeChunk->maxLocals, Value::undefined());
    new_frame.args = args;
    
    // Set frame.closure if calling a closure
    // Assuming callee has a closurePtr member for Closure shared_ptr
    // If you extended Value for closure type, set here:
    new_frame.closure = callee.closureValue; // may be nullptr if callee is plain functionRef

    // save current frame
    CallFrame prev_frame = callStack.back();
    
    //auto prev_stack = std::move(stack);
    //stack.clear();
    
    // copy args into frame.locals[0..]
    uint32_t ncopy = std::min<uint32_t>((uint32_t)args.size(), calleeChunk->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) new_frame.locals[i] = args[i];

    // push frame and execute it
    callStack.push_back(std::move(new_frame));

    Value result = runFrame(callStack.back());
    
    callStack.pop_back();
    frame = &prev_frame;

    //stack = prev_stack;

    return result;
    
}

void TurboVM::handleRethrow() {
//    // simplified: pop pending exception and re-run OP_THROW-like unwinding
//    if (stack.empty()) { running = false; return; }
//    Value pending = pop();
//    // Re-run throw loop: same as OP_THROW handling but without recursion here.
//    bool handled = false;
//    while (!tryStack.empty()) {
//        TryFrame f = tryStack.back();
//        tryStack.pop_back();
//        while ((int)stack.size() > f.stackDepth) stack.pop_back();
//        if (f.finallyIP != -1) {
//            stack.push_back(pending);
//            TryFrame resume;
//            resume.catchIP = f.catchIP;
//            resume.finallyIP = -1;
//            resume.stackDepth = f.stackDepth;
//            tryStack.push_back(resume);
//            ip = f.finallyIP;
//            handled = true;
//            break;
//        }
//        if (f.catchIP != -1) {
//            stack.push_back(pending);
//            ip = f.catchIP;
//            handled = true;
//            break;
//        }
//    }
//    if (!handled) {
//        printf("Uncaught exception after finally, halting VM\n");
//        running = false;
//    }
}

