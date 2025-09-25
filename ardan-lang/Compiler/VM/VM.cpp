//
//  VM.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "VM.hpp"

VM::VM() {
    globals["print"] = Value::function([](vector<Value> args) -> Value {
        Print::print(args);
        return Value::undefined();
    });
}

VM::VM(shared_ptr<Module> module_) : module_(module_) {
    
    globals["print"] = Value::function([](vector<Value> args) -> Value {
        Print::print(args);
        return Value::undefined();
    });
    
}

Value VM::pop() {
    if (stack.empty()) return Value::undefined();
    Value v = stack.back();
    stack.pop_back();
    return v;
}

Value VM::peek(int distance) {
    if (distance >= (int)stack.size()) return Value::undefined();
    return stack[stack.size() - 1 - distance];
}

uint8_t VM::readByte() {
    if (ip >= chunk->code.size()) return 0;
    return chunk->code[ip++];
}

uint32_t VM::readUint32() {
    if (ip + 4 > chunk->code.size()) throw std::runtime_error("read past end");
    uint32_t v = 0;
    v |= (uint32_t)chunk->code[ip++];
    v |= (uint32_t)chunk->code[ip++] << 8;
    v |= (uint32_t)chunk->code[ip++] << 16;
    v |= (uint32_t)chunk->code[ip++] << 24;
    return v;
}

uint8_t VM::readUint8() {
    return readByte();
}

Value VM::binaryAdd(const Value &a, const Value &b) {
    if (a.type == ValueType::STRING || b.type == ValueType::STRING) {
        return Value::str(a.toString() + b.toString());
    }
    return Value(a.numberValue + b.numberValue);
}

bool VM::isTruthy(const Value &v) {
    if (v.type == ValueType::NULLTYPE) return false;
    if (v.type == ValueType::UNDEFINED) return false;
    if (v.type == ValueType::BOOLEAN) return v.boolValue;
    if (v.type == ValueType::NUMBER) return v.numberValue != 0;
    if (v.type == ValueType::STRING) return !v.stringValue.empty();
    // objects/arrays considered truthy
    return true;
}

bool VM::equals(const Value &a, const Value &b) {
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

Value VM::getProperty(const Value &objVal, const string &propName) {
    if (objVal.type == ValueType::OBJECT) {
        return objVal.objectValue->get(propName);
    }
    if (objVal.type == ValueType::ARRAY) {
        return objVal.arrayValue->get(propName);
    }
//    if (objVal.type == ValueType::CLASS) {
//        return objVal.classValue->get(propName, false);
//    }
    // primitives -> string -> property? For now, undefined
    return Value::undefined();
}

void VM::setProperty(const Value &objVal, const string &propName, const Value &val) {
    if (objVal.type == ValueType::OBJECT) {
        objVal.objectValue->set(propName, val, "VAR", {});
        return;
    }
    if (objVal.type == ValueType::ARRAY) {
        objVal.arrayValue->set(propName, val);
        return;
    }
//    if (objVal.type == ValueType::CLASS) {
//        objVal.classValue->set(propName, val, false);
//        return;
//    }
    throw std::runtime_error("Cannot set property on non-object");
}

Value VM::run(shared_ptr<Chunk> chunk_, const vector<Value>& args) {
    // prepare a top-level frame that will be executed by runFrame()
    CallFrame frame;
    frame.chunk = chunk_;
    frame.ip = 0;
    frame.locals.resize(chunk_->maxLocals, Value::undefined());
    uint32_t ncopy = std::min((uint32_t)args.size(), chunk_->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) frame.locals[i] = args[i];

    callStack.push_back(std::move(frame));
    Value result = runFrame();
    callStack.pop_back();
    return result;
}

Value VM::runFrame() {
    
    if (callStack.empty()) return Value::undefined();
    CallFrame &frame = callStack.back();

    // Save current VM-level chunk/ip/locals to restore after this frame (if they are used elsewhere)
    auto prevChunk = chunk;
    // size_t prevIp = ip;
    auto prevLocals = std::move(locals); // move out current locals (if you rely on them elsewhere)
    // We'll restore them at the end.

    // Point VM at this frame's chunk/locals
    chunk = frame.chunk;
    ip = frame.ip;
    locals = frame.locals; // copy frame locals into VM locals for existing opcode handlers
        
//    chunk = chunk_;
//    ip = 0;
//    stack.clear();
//
//    // initialize locals
//    locals.clear();
//    locals.resize(chunk->maxLocals, Value::undefined());
//    // copy args into first slots
//    uint32_t ncopy = std::min((uint32_t)args.size(), chunk->maxLocals);
//    for (uint32_t i = 0; i < ncopy; ++i) locals[i] = args[i];

    while (running) {
        OpCode op = static_cast<OpCode>(readByte());
        switch (op) {
            case OpCode::OP_NOP:
                break;

                // pushes the constant to stack
            case OpCode::OP_CONSTANT: {
                uint32_t ci = readUint32();
                push(chunk->constants[ci]);
                break;
            }

            case OpCode::OP_POP: {
                pop();
                break;
            }

            case OpCode::OP_DUP: {
                Value v = peek(0);
                push(v);
                break;
            }
                
            case OpCode::OP_DUP2: {
                Value b = stack[stack.size() - 1];
                Value a = stack[stack.size() - 2];
                stack.push_back(a);
                stack.push_back(b);
                break;
            }
                
            case OpCode::OP_GET_LOCAL: {
                uint32_t idx = readUint32();
                if (idx >= locals.size()) push(Value::undefined());
                else push(locals[idx]);
                break;
            }

            case OpCode::OP_SET_LOCAL: {
                uint32_t idx = readUint32();
                Value val = pop();
                if (idx >= locals.size()) {
                    // grow if needed
                    locals.resize(idx + 1, Value::undefined());
                }
                locals[idx] = val;
                push(val);
                break;
            }

                // pushes the value of the global to stack
            case OpCode::OP_GET_GLOBAL: {
                uint32_t ci = readUint32();
                Value nameVal = chunk->constants[ci];
                string name = nameVal.toString();
                if (globals.find(name) != globals.end()) push(globals[name]);
                else push(Value::undefined());
                break;
            }

                // leaves nothing on stack
            case OpCode::OP_SET_GLOBAL: {
                uint32_t ci = readUint32();
                Value nameVal = chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                globals[name] = v;
                push(v);
                break;
            }

                // leaves nothing on stack
            case OpCode::OP_DEFINE_GLOBAL: {
                uint32_t ci = readUint32();
                Value nameVal = chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                globals[name] = v;
                push(v);
                break;
            }

            case OpCode::OP_NEW_OBJECT: {
                auto obj = std::make_shared<JSObject>();
                Value v;
                v.type = ValueType::OBJECT;
                v.objectValue = obj;
                push(v);
                break;
            }

            case OpCode::OP_SET_PROPERTY: {
                uint32_t ci = readUint32();
                string prop = chunk->constants[ci].toString();
                Value valueToSet = pop();
                Value objVal = pop();
                setProperty(objVal, prop, valueToSet);
                // push object back
                push(objVal);
                break;
            }

            case OpCode::OP_GET_PROPERTY: {
                uint32_t ci = readUint32();
                string prop = chunk->constants[ci].toString();
                Value objVal = pop();
                Value v = getProperty(objVal, prop);
                push(v);
                break;
            }
                
            case OpCode::OP_GET_PROPERTY_DYNAMIC: {
                // object is on stack
                // property is on stack
                Value val = pop();
                Value objVal = pop();
                Value v = getProperty(objVal, val.toString());
                push(v);
                break;
            }
                
            case OpCode::OP_SET_PROPERTY_DYNAMIC: {
                // Stack: ... obj, key, value
                Value value = stack.back(); stack.pop_back();
                Value key = stack.back(); stack.pop_back();
                Value& obj = stack.back();

                // Set property; assuming obj is some kind of associative object
                // obj.setProperty(key, value);
                setProperty(obj, key.toString(), value);

                // Remove obj from stack, push result if needed (often value or obj)
                // Here, we keep 'value' on top as result of assignment:
                stack.back() = value;
                break;
            }
                
            case OpCode::OP_NEW_ARRAY: {
                auto arr = std::make_shared<JSArray>();
                Value v;
                v.type = ValueType::ARRAY;
                v.arrayValue = arr;
                push(v);
                break;
            }

            case OpCode::OP_ARRAY_PUSH: {
                Value val = pop();
                Value arrVal = pop();
                if (arrVal.type != ValueType::ARRAY)
                    throw std::runtime_error("OP_ARRAY_PUSH: target not array");
                //arrVal.arrayValue->push(val);
                push(arrVal);
                break;
            }

            case OpCode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                push(binaryAdd(a,b));
                break;
            }
            case OpCode::OP_SUB: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numberValue - b.numberValue));
                break;
            }
            case OpCode::OP_MUL: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numberValue * b.numberValue));
                break;
            }
            case OpCode::OP_DIV: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numberValue / b.numberValue));
                break;
            }
            case OpCode::OP_MOD: {
                Value b = pop();
                Value a = pop();
                push(Value(fmod(a.numberValue, b.numberValue)));
                break;
            }
            case OpCode::OP_POW: {
                Value b = pop();
                Value a = pop();
                push(Value(pow(a.numberValue, b.numberValue)));
                break;
            }

            case OpCode::OP_NEGATE: {
                Value a = pop();
                push(Value(-a.numberValue));
                break;
            }

            case OpCode::OP_NOT: {
                Value a = pop();
                push(Value::boolean(!isTruthy(a)));
                break;
            }
                
            case OpCode::OP_INCREMENT: {
                
                Value a = pop();
                Value b = pop();
                
                int sum = a.numberValue + b.numberValue;
                
                push(Value(sum));

                break;
            }

            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(equals(a,b)));
                break;
            }

            case OpCode::OP_NOTEQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(!equals(a,b)));
                break;
            }

            case OpCode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue < b.numberValue));
                break;
            }
                
            case OpCode::OP_LESSEQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue <= b.numberValue));
                break;
            }
                
            case OpCode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue > b.numberValue));
                break;
            }
                
            case OpCode::OP_GREATEREQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue >= b.numberValue));
                break;
            }
                                
            case OpCode::LOGICAL_AND: {
                Value b = pop();
                Value a = pop();
                bool result = isTruthy(a) && isTruthy(b);
                push(Value::boolean(result));
                break;
            }

            case OpCode::LOGICAL_OR: {
                Value b = pop();
                Value a = pop();
                bool result = isTruthy(a) || isTruthy(b);
                push(Value::boolean(result));
                break;
            }
                
            case OpCode::NULLISH_COALESCING: {
                break;
            }
                
            case OpCode::REFERENCE_EQUAL: {
                Value b = pop();
                Value a = pop();
                bool isEqual = false;
                // For objects/arrays: compare pointers
                if (a.type == b.type) {
                    if (a.type == ValueType::OBJECT)
                        isEqual = (a.objectValue == b.objectValue);
                    else if (a.type == ValueType::ARRAY)
                        isEqual = (a.arrayValue == b.arrayValue);
                    // For other types, could default to pointer or identity check
                }
                push(Value::boolean(isEqual));
                break;
            }
                
            case OpCode::STRICT_INEQUALITY: {
                Value b = pop();
                Value a = pop();
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
                    // etc.
                }
                push(Value::boolean(notEqual));
                break;
            }
                
            case OpCode::OP_DECREMENT: {
                Value a = pop();
                Value b = pop();
                int sum = a.numberValue - b.numberValue;
                push(Value(sum));
                break;
            }
                
                // bitwise
            case OpCode::OP_BIT_AND: {
                Value b = pop();
                Value a = pop();
                int result = (int)a.numberValue & (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::OP_BIT_OR: {
                Value b = pop();
                Value a = pop();
                int result = (int)a.numberValue | (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::OP_BIT_XOR: {
                Value b = pop();
                Value a = pop();
                int result = (int)a.numberValue ^ (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::OP_SHL: {
                Value b = pop(); // shift amount
                Value a = pop(); // value to shift
                int result = (int)a.numberValue << (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::OP_SHR: {
                Value b = pop(); // shift amount
                Value a = pop(); // value to shift
                int result = (int)a.numberValue >> (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::OP_USHR: {
                Value b = pop(); // shift amount
                Value a = pop(); // value to shift
                unsigned int result = (unsigned int)a.numberValue >> (unsigned int)b.numberValue;
                push(Value(result));
                break;
            }
                
            case OpCode::OP_POSITIVE: {
                Value a = pop();
                Value b = pop();
                int result = (int)a.numberValue & (int)b.numberValue;
                push(Value(result));
                break;
            }

            // jumps
            case OpCode::OP_JUMP: {
                uint32_t offset = readUint32();
                ip += offset;
                break;
            }

            case OpCode::OP_JUMP_IF_FALSE: {
                uint32_t offset = readUint32();
                Value cond = pop();
                if (!isTruthy(cond)) ip += offset;
                break;
            }

            case OpCode::OP_LOOP: {
                uint32_t offset = readUint32();
                // jump backwards
                ip -= offset;
                break;
            }

//            case OpCode::OP_CALL: {
//                uint8_t argc = readUint8();
//                vector<Value> args(argc);
//                // args were pushed left-to-right; pop in reverse to place in array correctly
//                for (int i = argc - 1; i >= 0; --i) {
//                    args[i] = pop();
//                }
//                Value callee = pop();
//
//                if (callee.type == ValueType::FUNCTION) {
//                    // call host function (native or compiled wrapper)
//                    Value result = callee.functionValue(args);
//                } else {
//                    throw std::runtime_error("Attempted to call a non-function value.");
//                }
//                break;
//            }
            
            case OpCode::OP_CALL: {
                uint32_t argCount = readUint8();
                // top-of-stack should be the callee value (function ref)
                Value callee = peek(argCount); // or adjust as per your calling convention
                vector<Value> args = popArgs(argCount);
                Value result = callFunction(callee, args);
                // pop callee and args and push result
                push(result);
                break;
            }
                
            case OpCode::OP_TRY: {
                uint32_t catchOffset = readUint32();   // relative offset from after the two offsets
                uint32_t finallyOffset = readUint32();
                // compute absolute IPs
                int base = ip; // ip now points after the offsets
                TryFrame f;
                f.catchIP = (catchOffset == 0) ? -1 : (base + (int)catchOffset);
                f.finallyIP = (finallyOffset == 0) ? -1 : (base + (int)finallyOffset);
                f.stackDepth = (int)stack.size();
                // ipAfterTry can be filled later by codegen if you want; keep -1 if unused
                f.ipAfterTry = -1;
                tryStack.push_back(f);
                break;
            }

            case OpCode::OP_END_TRY: {
                if (tryStack.empty()) {
                    // runtime error: unmatched END_TRY
                    running = false;
                    break;
                }
                tryStack.pop_back();
                break;
            }
                
            case OpCode::OP_THROW: {
                // exception value on top of stack
                Value exc = pop();
                // unwind frames until we find a handler (catch or finally)
                bool handled = false;
                // Store a 'pending exception' to know we are unwinding due to throw
                Value pending = exc;
                
                while (!tryStack.empty()) {
                    TryFrame f = tryStack.back();
                    tryStack.pop_back();
                    
                    // first, unwind the value stack to the depth at try entry
                    while ((int)stack.size() > f.stackDepth) stack.pop_back();
                    
                    // If there is a finally, run it first.
                    if (f.finallyIP != -1) {
                        // push pending exception so finalizer can see it if needed
                        stack.push_back(pending);
                        
                        // Record a special marker frame to indicate we are resuming a throw after finally
                        // We'll push a synthetic TryFrame with catchIP = original catchIP, finallyIP = -1
                        TryFrame resume;
                        resume.catchIP = f.catchIP;
                        resume.finallyIP = -1; // don't re-run finalizer
                        resume.stackDepth = f.stackDepth; // after finally resumes, stack depth should be this
                        resume.ipAfterTry = -1;
                        tryStack.push_back(resume);
                        // jump into finalizer
                        ip = f.finallyIP;
                        handled = true; // we will handle after finalizer/resume
                        break;
                    }
                    
                    // If no finally, but there is a catch, jump to catch and push exception
                    if (f.catchIP != -1) {
                        stack.push_back(pending);
                        ip = f.catchIP;
                        handled = true;
                        break;
                    }
                    
                    // else continue unwinding to outer try frame
                }
                
                if (!handled) {
                    // uncaught
                    // Here: runtime uncaught exception -> abort or print error
                    // For demo, we stop the VM
                    printf("Uncaught exception, halting VM\n");
                    running = false;
                }
                break;
            }
                
            case OpCode::OP_END_FINALLY: {
                // When a finally finishes, we must check whether we have a resume frame that carries a pending throw
                // Approach: if there is a TryFrame on tryStack whose catchIP != -1 and which we pushed as resume frame,
                // then either jump into catch or rethrow.
                if (!tryStack.empty()) {
                    TryFrame resume = tryStack.back();
                    // If resume.finallyIP == -1, we treat this as the resume frame we pushed earlier
                    if (resume.finallyIP == -1) {
                        tryStack.pop_back();
                        if (resume.catchIP != -1) {
                            // pending exception should be on stack top
                            // jump into catch with exception on stack
                            ip = resume.catchIP;
                            break;
                        } else {
                            // no catch for the pending exception, continue unwinding:
                            // emulate throwing again: pop pending exception and re-run OP_THROW logic
                            Value pending = pop();
                            // continue throw loop by re-inserting pending on stack and handling next frame
                            // simplest approach: directly re-run THROW handling by pushing pending and continuing
                            // For clarity, we will set a special behaviour: re-insert pending and emulate OP_THROW
                            stack.push_back(pending);
                            // simulate OP_THROW logic by jumping back one step: decrement ip so we re-execute OP_THROW
                            // But cleaner: call a helper to handle rethrow. For brevity we perform a manual loop here.
                            // (In production you would share the throw-handling code.)
                            // For now: we'll call a helper:
                            handleRethrow();
                            break;
                        }
                    }
                }
                // Normal end finally with no pending throw resume -> continue execution
                break;
            }
                
            case OpCode::OP_RETURN: {
                Value v = pop();
                return v;
            }

            case OpCode::OP_HALT:
                return Value::undefined();

            default:
                throw std::runtime_error("Unhandled opcode");
        }
    }
    return Value::undefined();
}

Value VM::callFunction(Value callee, vector<Value>& args) {
    
    if (callee.type == ValueType::FUNCTION) {
        // call host function (native or compiled wrapper)
        Value result = callee.functionValue(args);
        return result;
    }
    
    if (callee.type != ValueType::FUNCTION_REF) {
        // runtimeError("Attempt to call non-function");
        return Value::undefined();
    }
    
    auto fn = callee.fnRef;
    
    if (!module_) {
        // runtimeError("Module not set in VM");
        return Value::undefined();
    }
    
    if (fn->chunkIndex >= module_->chunks.size()) {
        // runtimeError("Bad function index");
        return Value::undefined();
    }
    
    shared_ptr<Chunk> calleeChunk = module_->chunks[fn->chunkIndex];

    // Build new frame
    CallFrame frame;
    frame.chunk = calleeChunk;
    frame.ip = 0;
    // allocate locals sized to the chunk's max locals (some chunks use maxLocals)
    frame.locals.resize(calleeChunk->maxLocals, Value::undefined());

    // copy args into frame.locals[0..]
    uint32_t ncopy = std::min<uint32_t>((uint32_t)args.size(), calleeChunk->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) frame.locals[i] = args[i];

    // push frame and execute it
    callStack.push_back(std::move(frame));
    Value result = runFrame();
    callStack.pop_back();

    return result;
    
}

vector<Value> VM::popArgs(size_t count) {
    
    vector<Value> args;
    args.resize(count, Value::undefined());

    // args were pushed left-to-right, so top of stack is last arg.
    // pop in reverse to restore original order into args[0..count-1].
    for (size_t i = 0; i < count; ++i) {
        if (stack.empty()) {
            args[count - 1 - i] = Value::undefined();
        } else {
            args[count - 1 - i] = stack.back();
            stack.pop_back();
        }
    }
    return args;
}

void VM::handleRethrow() {
    // simplified: pop pending exception and re-run OP_THROW-like unwinding
    if (stack.empty()) { running = false; return; }
    Value pending = pop();
    // Re-run throw loop: same as OP_THROW handling but without recursion here.
    bool handled = false;
    while (!tryStack.empty()) {
        TryFrame f = tryStack.back();
        tryStack.pop_back();
        while ((int)stack.size() > f.stackDepth) stack.pop_back();
        if (f.finallyIP != -1) {
            stack.push_back(pending);
            TryFrame resume;
            resume.catchIP = f.catchIP;
            resume.finallyIP = -1;
            resume.stackDepth = f.stackDepth;
            tryStack.push_back(resume);
            ip = f.finallyIP;
            handled = true;
            break;
        }
        if (f.catchIP != -1) {
            stack.push_back(pending);
            ip = f.catchIP;
            handled = true;
            break;
        }
    }
    if (!handled) {
        printf("Uncaught exception after finally, halting VM\n");
        running = false;
    }
}
