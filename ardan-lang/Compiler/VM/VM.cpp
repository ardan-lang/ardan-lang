//
//  VM.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "VM.hpp"

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
        objVal.objectValue->set(propName, val);
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
    chunk = chunk_;
    ip = 0;
    stack.clear();

    // initialize locals
    locals.clear();
    locals.resize(chunk->maxLocals, Value::undefined());
    // copy args into first slots
    uint32_t ncopy = std::min((uint32_t)args.size(), chunk->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) locals[i] = args[i];

    while (true) {
        OpCode op = static_cast<OpCode>(readByte());
        switch (op) {
            case OpCode::OP_NOP:
                break;

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

            case OpCode::OP_GET_GLOBAL: {
                uint32_t ci = readUint32();
                Value nameVal = chunk->constants[ci];
                string name = nameVal.toString();
                if (globals.find(name) != globals.end()) push(globals[name]);
                else push(Value::undefined());
                break;
            }

            case OpCode::OP_SET_GLOBAL: {
                uint32_t ci = readUint32();
                Value nameVal = chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                globals[name] = v;
                push(v);
                break;
            }

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

            case OpCode::OP_CALL: {
                uint8_t argc = readUint8();
                vector<Value> args(argc);
                // args were pushed left-to-right; pop in reverse to place in array correctly
                for (int i = argc - 1; i >= 0; --i) {
                    args[i] = pop();
                }
                Value callee = pop();

                if (callee.type == ValueType::FUNCTION) {
                    // call host function (native or compiled wrapper)
                    Value result = callee.functionValue(args);
                    push(result);
                } else {
                    throw std::runtime_error("Attempted to call a non-function value.");
                }
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
