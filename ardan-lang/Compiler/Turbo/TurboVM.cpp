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
    
    event_loop = new EventLoop();
    
    env->set_var("Math", make_shared<Math>());
    env->set_var("console", make_shared<Print>());
    env->set_var("fs", make_shared<File>());
    env->set_var("Server", make_shared<Server>(event_loop));
    
    env->set_var("print", Value::function([this](vector<Value> args) mutable -> Value {
        Print::print(args);
        return Value::nullVal();
    }));
    
    env->set_var("Window", make_shared<Window>());
    env->set_var("Button", make_shared<Button>());
    
}

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
        // perform privacy check
        // if we have do not have js_object in closuure.
        
        // get the prop modifiers.
        // if its private, check if the js_object in closure is not nullptr
        // if closure.js_object is not nullptr
        
        vector<string> modifiers = objVal.objectValue->get_modifiers(propName);
        
        bool isPrivate = false;
        bool isProtected = false;
        
        for (auto modifier : modifiers) {
            if (modifier == "private") {
                isPrivate = true;
            }
            
            if (modifier == "protected") {
                isProtected = true;
            }
        }
        
        if (isPrivate) {
            // Disallow if we are not inside a closure of the owning object
            if (frame->closure->js_object == nullptr || frame->closure->js_object.get() != objVal.objectValue.get()) {
                throw runtime_error("Can't access '" + propName + "' a private property outside its class.");
            }
        }
        
        if (isProtected) {
            if (frame->closure->js_object == nullptr) {
                throw runtime_error("Can't access '" + propName + "' a protected property outside its class or subclass.");
            }
            auto accessor = frame->closure->js_object;
            auto owner = objVal.objectValue;
            // Traverse up the class hierarchy of accessor to see if it matches owner's class
            auto accessorClass = accessor->getKlass();
            auto ownerClass = owner->getKlass();
            bool allowed = false;
            while (accessorClass) {
                if (accessorClass.get() == ownerClass.get()) {
                    allowed = true;
                    break;
                }
                accessorClass = accessorClass->superClass;
            }
            if (!allowed) {
                throw runtime_error("Can't access '" + propName + "' a protected property outside its class or subclass.");
            }
        }
        
        return objVal.objectValue->get(propName);
    }
    
    if (objVal.type == ValueType::ARRAY) {
        return objVal.arrayValue->get(propName);
    }
    
    if (objVal.type == ValueType::CLASS) {
        
        // perform privacy check
        // if we have do not have js_object in closuure.
        
        // get the prop modifiers.
        // if its private, check if the js_object in closure is not nullptr
        
        vector<string> modifiers = objVal.classValue->get_static_modifiers(propName);
        bool isPrivate = false;
        bool isProtected = false;

        for (auto modifier : modifiers) {
            
            if (modifier == "private") {
                isPrivate = true;
            }
            
            if (modifier == "protected") {
                isProtected = true;
            }

        }

        if (isPrivate) {
            if (!frame->closure->js_object ||
                frame->closure->js_object->getKlass().get() != objVal.classValue.get()) {
                throw runtime_error("Can't access a private static property outside its class.");
            }
        }
        
        if (isProtected) {
            if (!frame->closure->js_object) {
                throw runtime_error("Can't access a protected static property outside its class or subclass.");
            }
            auto accessorClass = frame->closure->js_object->getKlass();
            auto targetClass = objVal.classValue;
            bool allowed = false;
            while (accessorClass) {
                if (accessorClass.get() == targetClass.get()) {
                    allowed = true;
                    break;
                }
                accessorClass = accessorClass->superClass;
            }
            if (!allowed) {
                throw runtime_error("Can't access a protected static property outside its class or subclass.");
            }
        }
        
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
//    if (objVal.type == ValueType::CLASS) {
//        objVal.classValue->set(propName, val, false);
//        return;
//    }
    throw std::runtime_error("Cannot set property on non-object");
}

void TurboVM::setStaticProperty(const Value &objVal, const string &propName, const Value &val) {
    if (objVal.type == ValueType::CLASS) {
        //objVal.classValue->set_static_vm(propName, val);
        return;
    }
    throw std::runtime_error("Cannot set static property on non-class");
}

const unordered_map<string, Value> TurboVM::enumerateKeys(Value obj) {
    
    if (obj.type == ValueType::OBJECT) {
        return obj.objectValue->get_all_properties();
    }
    
    if (obj.type == ValueType::ARRAY) {
        return obj.arrayValue->get_indexed_properties();
    }
    
    return {};
    
}

void TurboVM::set_js_object_closure(Value objVal) {
    if (objVal.type == ValueType::OBJECT) {
        for(auto& prop : objVal.objectValue->get_all_properties()) {
            if (prop.second.type == ValueType::CLOSURE) {
                prop.second.closureValue->js_object = objVal.objectValue;
                
            }
        }
    }
}

shared_ptr<JSObject> TurboVM::createJSObject(shared_ptr<JSClass> klass) {
    
    shared_ptr<JSObject> object = make_shared<JSObject>();
    object->turboVM = this;
    object->setClass(klass);

    makeObjectInstance(Value::klass(klass), object);
    
    // create a jsobject from superclass and assign to parent_object
    if (klass->superClass != nullptr) {
        
        object->parent_object = createJSObject(klass->superClass);
        object->parent_class = klass->superClass;
        
    }

    return object;

}

void TurboVM::makeObjectInstance(Value klass, shared_ptr<JSObject> obj) {
    
    for (auto& protoProp : klass.classValue->var_proto_props) {
                
        if (protoProp.second.value.type == ValueType::CLOSURE) {
            
            shared_ptr<Closure> new_closure = make_shared<Closure>();
            new_closure->fn = protoProp.second.value.closureValue->fn;
            new_closure->upvalues = protoProp.second.value.closureValue->upvalues;
            new_closure->js_object = obj;

            obj->set(protoProp.first,
                     Value::closure(new_closure),
                     "VAR",
                     protoProp.second.modifiers);

        } else {
            
            // evaluate fields
            int field_reg = protoProp.second.value.numberValue;
            int chunk_index = frame->registers[field_reg].numberValue;
            Value fnValue = module_->constants[chunk_index];
            Value val = callFunction(fnValue, {});
            
            //     Value fnValue = Value::functionRef(fnObj);
            
            obj->set(protoProp.first,
                     val,
                     "VAR",
                     protoProp.second.modifiers);

        }

    }

    for (auto& constProtoProp : klass.classValue->const_proto_props) {
                
        if (constProtoProp.second.value.type == ValueType::CLOSURE) {
            
            shared_ptr<Closure> new_closure = make_shared<Closure>();
            new_closure->fn = constProtoProp.second.value.closureValue->fn;
            new_closure->upvalues = constProtoProp.second.value.closureValue->upvalues;
            new_closure->js_object = obj;

            obj->set(constProtoProp.first, Value::closure(new_closure), "CONST", {});

        } else {
            
            // evaluate fields
            int field_reg = constProtoProp.second.value.numberValue;
            int chunk_index = frame->registers[field_reg].numberValue;
            Value fnValue = module_->constants[chunk_index];
            Value val = callFunction(fnValue, {});

            obj->set(constProtoProp.first,
                     constProtoProp.second.value,
                     "CONST",
                     constProtoProp.second.modifiers);

        }

    }

}

void TurboVM::invokeMethod(Value obj_value, string name, vector<Value> args) {
    
    if (obj_value.type == ValueType::OBJECT) {
        Value constructor = obj_value.objectValue->get(name);
        // TODO: check whether to throw error
        if (constructor.type != ValueType::UNDEFINED) {
            callMethod(constructor, args, obj_value);
        }
    }
    
}

Value TurboVM::addCtor() {

    shared_ptr<TurboChunk> fnChunk = make_shared<TurboChunk>();
    fnChunk->arity = 0;

    // fnChunk->writeByte(static_cast<uint8_t>(OpCode::Nop));
    fnChunk->code.push_back({TurboOpCode::Nop, 0,0,0});
    // fnChunk->writeByte(static_cast<uint8_t>((OpCode::SuperCall)));
    // fnChunk->writeUint8((uint8_t)0);
    fnChunk->code.push_back({TurboOpCode::SuperCall, 0,0,0});

    int constant_index = fnChunk->addConstant(Value::undefined());
    
    // fnChunk->writeByte(static_cast<uint8_t>(OpCode::LoadConstant));
    // fnChunk->writeUint32(constant_index);
    // fnChunk->writeByte(static_cast<uint8_t>(OpCode::Return));
    fnChunk->code
        .push_back({TurboOpCode::LoadConst, 0, (uint8_t)constant_index});
    fnChunk->code.push_back({TurboOpCode::Return, 0});
    
    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = std::make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = "ctor";
    fnObj->upvalues_size = 0;

    Value fnValue = Value::functionRef(fnObj);

    shared_ptr<Closure> new_closure = make_shared<Closure>();
    new_closure->fn = fnObj;
    new_closure->upvalues = {};
    
    return Value::closure(new_closure);

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
                frame->registers[dest] = val;
                break;
            }
                
            case TurboOpCode::Move: {
                uint8_t src = instruction.b;
                uint8_t dest = instruction.a;
                frame->registers[dest] = frame->registers[src];
                break;
            }
                
            case TurboOpCode::CreateLocalVar: {
                uint8_t local_index = instruction.a;
                uint8_t data_reg = instruction.b;
                frame->locals[local_index] = frame->registers[data_reg];
                break;
            }
                
            case TurboOpCode::CreateLocalLet:{
                uint8_t local_index = instruction.a;
                uint8_t data_reg = instruction.b;
                frame->locals[local_index] = frame->registers[data_reg];
                break;
            }
                
            case TurboOpCode::CreateLocalConst:{
                uint8_t local_index = instruction.a;
                uint8_t data_reg = instruction.b;
                frame->locals[local_index] = frame->registers[data_reg];
                break;
            }
                
            case TurboOpCode::CreateGlobalVar:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                env->set_var(name, frame->registers[data_reg]);
                
                break;
            }
                
            case TurboOpCode::CreateGlobalLet:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                env->set_let(name, frame->registers[data_reg]);
                
                break;
            }
                
            case TurboOpCode::CreateGlobalConst:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                env->set_const(name, frame->registers[data_reg]);
                
                break;
            }
                
                // TurboOpCode::Add, opResultReg, lhsReg, rhsReg
            case TurboOpCode::Add: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                Value result = binaryAdd(lhs, rhs);
                frame->registers[instruction.a] = result;
                break;
            }
                
            case TurboOpCode::Subtract: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(lhs.numberValue - rhs.numberValue);
                break;
            }
                
            case TurboOpCode::Multiply: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(lhs.numberValue * rhs.numberValue);
                break;
            }
                
            case TurboOpCode::Divide: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(lhs.numberValue / rhs.numberValue);
                break;
            }
                
            case TurboOpCode::Modulo: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(fmod(lhs.numberValue, rhs.numberValue));
                break;
            }
                
            case TurboOpCode::Power: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(pow(lhs.numberValue, rhs.numberValue));
                break;
            }
                
            case TurboOpCode::ShiftLeft: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value((int)lhs.numberValue << (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::ShiftRight: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value((int)lhs.numberValue >> (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::UnsignedShiftRight: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value((unsigned int)lhs.numberValue >> (unsigned int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::BitAnd: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value((int)lhs.numberValue & (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::BitOr: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value((int)lhs.numberValue | (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::BitXor: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value((int)lhs.numberValue ^ (int)rhs.numberValue);
                break;
            }
                
            case TurboOpCode::LogicalAnd: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(isTruthy(lhs) && isTruthy(rhs));
                break;
            }
                
            case TurboOpCode::LogicalOr: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value(isTruthy(lhs) || isTruthy(rhs));
                break;
            }
                
            case TurboOpCode::NullishCoalescing: {
                Value lhs = frame->registers[instruction.b];
                Value rhs = frame->registers[instruction.c];
                frame->registers[instruction.a] = isNullish(lhs) ? rhs : lhs;
                break;
            }
                
            case TurboOpCode::StrictEqual: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                bool isEqual = false;
                // For objects/arrays: we compare pointers
                if (a.type == b.type) {
                    if (a.type == ValueType::OBJECT)
                        isEqual = (a.objectValue == b.objectValue);
                    else if (a.type == ValueType::ARRAY)
                        isEqual = (a.arrayValue == b.arrayValue);
                    // For other types, we could default to pointer or identity check
                }
                frame->registers[instruction.a] = Value::boolean(isEqual);
                break;
            }
                
            case TurboOpCode::StrictNotEqual: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
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
                frame->registers[instruction.a] = (Value::boolean(notEqual));
                break;
            }
                
            case TurboOpCode::Decrement: {
                Value a = frame->registers[instruction.a];
                Value b = frame->registers[instruction.b];
                int sum = a.numberValue - b.numberValue;
                frame->registers[instruction.a] = Value(sum);
                break;
            }

            case TurboOpCode::Negate: {
                Value a = frame->registers[instruction.a];
                frame->registers[instruction.a] = Value(-a.numberValue);
                break;
            }

            case TurboOpCode::LogicalNot: {
                Value a = frame->registers[instruction.a];
                frame->registers[instruction.a] = Value::boolean(!isTruthy(a));
                break;
            }
                
            case TurboOpCode::Increment: {
                
                Value a = frame->registers[instruction.a];
                Value b = frame->registers[instruction.b];
                
                int sum = a.numberValue + b.numberValue;
                
                frame->registers[instruction.a] = Value(sum);

                break;
            }

            case TurboOpCode::Equal: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] =  Value::boolean(equals(a,b));
                break;
            }

            case TurboOpCode::NotEqual: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value::boolean(!equals(a,b));
                break;
            }

                // b < c
            case TurboOpCode::LessThan: {
                // result register is a.
                // left reg is b
                // right register is c
                Value b = frame->registers[instruction.b];
                Value c = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value::boolean(b.numberValue < c.numberValue);
                break;
            }
                
            case TurboOpCode::LessThanOrEqual: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value::boolean(a.numberValue <= b.numberValue);
                break;
            }
                
            case TurboOpCode::GreaterThan: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value::boolean(a.numberValue > b.numberValue);
                break;
            }
                
            case TurboOpCode::GreaterThanOrEqual: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] = Value::boolean(a.numberValue >= b.numberValue);
                break;
            }
                
                // jumps
            case TurboOpCode::Jump: {
                
                uint32_t offset = instruction.a;
                
                frame->ip += offset;
                break;
            }

            case TurboOpCode::JumpIfFalse: {
                uint32_t offset = instruction.b;
                Value cond = frame->registers[instruction.a];
                if (!isTruthy(cond)) frame->ip += offset;
                break;
            }
                
            case TurboOpCode::Loop: {
                
                uint32_t offset = instruction.a;
                
                frame->ip -= offset;

                break;
            }

            case TurboOpCode::LoadLocalVar: {
                // LoadLocalVar, reg_slot, idx
                int reg = instruction.a;
                int idx = instruction.b;
                
                frame->registers[reg] = frame->locals[idx];
                break;
            }
                
            case TurboOpCode::LoadGlobalVar: {
                
                int reg = instruction.a;
                int idx = instruction.b;
                string name = frame->chunk->constants[idx].stringValue;
                
                frame->registers[reg] = toValue(env->get(name));

                break;
            }
                
                // TODO: we need to store via var, let, const
                // emit(TurboOpCode::StoreLocal, idx, reg_slot);
            case TurboOpCode::StoreLocalVar: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                
                frame->locals[idx] = frame->registers[reg_slot];

                break;
            }
                
            case TurboOpCode::StoreLocalLet: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                
                frame->locals[idx] = frame->registers[reg_slot];
                
                break;
            }

            case TurboOpCode::StoreGlobalVar: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                Value val = frame->chunk->constants[idx];
                string name = val.stringValue;
                
                env->set_var(name, frame->registers[reg_slot]);

                break;
            }
                
            case TurboOpCode::StoreGlobalLet: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                Value val = frame->chunk->constants[idx];
                string name = val.stringValue;

                env->set_let(name, frame->registers[reg_slot]);

                break;
            }
                
                // emit(TurboOpCode::NewArray, arr);
            case TurboOpCode::NewArray: {
                auto array = make_shared<JSArray>();
                frame->registers[instruction.a] = Value::array(array);
                break;
            }
                
                // emit(TurboOpCode::ArrayPush, arr, val);
            case TurboOpCode::ArrayPush: {
                auto array = frame->registers[instruction.a].arrayValue;
                array->push({frame->registers[instruction.b]});
                break;
            }
                
                // TurboOpCode::NewClass, super_class_reg, nameconstindex
            case TurboOpCode::NewClass: {
                                
                auto superclass = frame->registers[instruction.a];
                
                string name = frame->chunk->constants[instruction.c].toString();
                
                auto js_class = make_shared<JSClass>();
                
                if (superclass.type == ValueType::CLASS) {
                    js_class->superClass = superclass.classValue;
                }
                
                Value klass = Value::klass(js_class);
                klass.classValue->name = name;
                
                // add constructor
                klass.classValue->set_proto_vm_var("constructor", addCtor(), { "public" } );
                                
                frame->registers[instruction.a] = (klass);
                break;
            }
                
                // op, super_class_reg, initReg, fieldNameReg
                
                // property var
            case TurboOpCode::CreateClassPrivatePropertyVar: {

                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_proto_vm_var(fieldNameValue.stringValue, init, { "private" } );
                
                break;
            }
                
            case TurboOpCode::CreateClassPublicPropertyVar: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                klass.classValue->set_proto_vm_var(fieldNameValue.stringValue, init, { "public" } );

                break;
                
            }
                
            case TurboOpCode::CreateClassProtectedPropertyVar: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                klass.classValue->set_proto_vm_var(fieldNameValue.stringValue, init, { "protected" } );

                break;
                
            }
                
                // property const
            case TurboOpCode::CreateClassPrivatePropertyConst: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_proto_vm_const(fieldNameValue.stringValue, init, { "private" } );

                break;
            }
                
            case TurboOpCode::CreateClassPublicPropertyConst: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_proto_vm_const(fieldNameValue.stringValue, init, { "public" } );

                break;
            }
                
            case TurboOpCode::CreateClassProtectedPropertyConst: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];

                klass.classValue->set_proto_vm_const(fieldNameValue.stringValue, init, { "protected" } );

                break;
            }
                
                // static var
            case TurboOpCode::CreateClassPrivateStaticPropertyVar: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_var(fieldNameValue.stringValue, init, { "private" });
                break;
                
            }
                
            case TurboOpCode::CreateClassPublicStaticPropertyVar: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_var(fieldNameValue.stringValue, init, { "public" });
                break;
            }
                
            case TurboOpCode::CreateClassProtectedStaticPropertyVar: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_var(fieldNameValue.stringValue, init, { "protected" });
                break;
            }
                
                // static const
            case TurboOpCode::CreateClassPrivateStaticPropertyConst: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_const(fieldNameValue.stringValue, init, { "private" });
                break;
            }
                
            case TurboOpCode::CreateClassPublicStaticPropertyConst: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_const(fieldNameValue.stringValue, init, { "public" });

                break;
            }
                
            case TurboOpCode::CreateClassProtectedStaticPropertyConst: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_const(fieldNameValue.stringValue, init, { "protected" });
                break;
            }
                
                // op, super_class_reg, method_reg, methodNameReg);
                
            case TurboOpCode::CreateClassProtectedStaticMethod: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_var(fieldNameValue.stringValue, init, { "protected" });

                break;
            }
                
            case TurboOpCode::CreateClassPrivateStaticMethod: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_var(fieldNameValue.stringValue, init, { "private" });

                break;
            }
                
            case TurboOpCode::CreateClassPublicStaticMethod: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_var(fieldNameValue.stringValue, init, { "public" });
                break;
            }
                
            case TurboOpCode::CreateClassProtectedMethod: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_proto_vm_var(fieldNameValue.stringValue, init, { "protected" });
                break;
            }
                
            case TurboOpCode::CreateClassPrivateMethod: {
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_proto_vm_var(fieldNameValue.stringValue, init, { "private" });
                break;
            }
                
            case TurboOpCode::CreateClassPublicMethod: {
                
                Value klass = frame->registers[instruction.a];
                Value init = frame->registers[instruction.b];
                Value fieldNameValue = frame->registers[instruction.c];
                
                klass.classValue->set_proto_vm_var(fieldNameValue.stringValue, init, { "public" });
                break;
            }
                
                // TurboOpCode::CreateInstance, reg
            case TurboOpCode::CreateInstance: {
                
                Value klass = frame->registers[instruction.a];
                
                if (klass.classValue->is_native == true) {
                    
                    // add constructor
                    // check if constructor exists
                    if (!klass.classValue->is_constructor_available()) {
                        klass.classValue->set_proto_vm_var("constructor", addCtor(), { "public" } );
                    }

                    shared_ptr<JSObject> native_object = klass.classValue->construct();

                    Value obj_value = Value::object(native_object);
                    obj_value.objectValue->turboVM = this;

                    set_js_object_closure(obj_value);

                    frame->registers[instruction.a] = obj_value;

                    break;
                }

                // auto obj = make_shared<JSObject>();
                // obj->setClass(klass.classValue);
                
                // TODO: we need to invoke parent constructor

                shared_ptr<JSObject> obj = createJSObject(klass.classValue);
                
                Value obj_value = Value::object(obj);

                frame->registers[instruction.a] = obj_value;

                break;
            }
                
                // TurboOpCode::InvokeConstructor, reg, argRegs[0], (int)argRegs.size());
            case TurboOpCode::InvokeConstructor: {
                
                // vector<Value> args;
                
                int start = instruction.b;
                int count = instruction.c;
                
//                for (int i = 0; i < count; i++) {
//                    int reg = start + i;
//                    args.push_back(frame->registers[reg]);
//                }

                const vector<Value> const_args = { argStack.begin(), argStack.end() };

//                for (auto arg : argStack) {
//                    args.push_back(frame->registers[arg.toString()]);
//                }
                
                argStack.clear();

                Value obj_value = frame->registers[instruction.a];

                // call the constructor
                invokeMethod(obj_value, "constructor", const_args);

                frame->registers[instruction.a] = obj_value;

                break;
            }
                
                // emit(TurboOpCode::NewObject, obj);
            case TurboOpCode::NewObject: {
                auto object = make_shared<JSObject>();
                Value v = Value::object(object);

                frame->registers[instruction.a] = v;
                break;
            }
                
            case TurboOpCode::GetThis: {
//                push(Value::object(frame->closure->js_object));
                frame->registers[instruction.a] = Value::object(frame->closure->js_object);
                break;
            }
                
                // TurboOpCode::LoadThisProperty, reg_slot, nameIdx
            case TurboOpCode::LoadThisProperty: {
                
                // load constant from nameIdx
                Value property_value = frame->chunk->constants[instruction.b];
                string property_name = property_value.toString();
                                
                Value obj = getProperty(Value::object(frame->closure->js_object), property_name);
                
                frame->registers[instruction.b] = obj;

                break;
            }
                
                // StoreThisProperty, nameIdx, reg_slot
            case TurboOpCode::StoreThisProperty: {
                
                // load constant from nameIdx
                Value property_value = frame->chunk->constants[instruction.a];
                string property_name = property_value.toString();
                
                Value value = frame->registers[instruction.b];
                
                setProperty(Value::object(frame->closure->js_object), property_name, value);
                
                // this update the object the current object
//                int index = readUint32();
//                Value v = pop();
//                string prop = frame->chunk->constants[index].toString();
//                setProperty(Value::object(frame->closure->js_object), prop, v);

                
                break;
            }
                
                // Now: StoreThisProperty
            case TurboOpCode::SetThisProperty: {
                // this update the object the current object
//                int index = readUint32();
//                Value v = pop();
//                string prop = frame->chunk->constants[index].toString();
//                setProperty(Value::object(frame->closure->js_object), prop, v);

                break;
            }
                
                // Now: LoadThisProperty
            case TurboOpCode::GetThisProperty: {
                
//                int index = readUint32();
//                string prop = frame->chunk->constants[index].toString();

                // push(getProperty(Value::object(frame->closure->js_object), prop));
                
                break;
            }
                
            case TurboOpCode::GetParentObject: {
                // push(Value::object(frame->closure->js_object->parent_object));
                frame->registers[instruction.a] = Value::object(frame->closure->js_object->parent_object);
                break;
            }

                // emit(TurboOpCode::SetProperty, obj, emitConstant(prop.first.lexeme), val);
                // SetProperty: objReg, nameIdx, valueReg
            case TurboOpCode::SetProperty: {
                auto object = frame->registers[instruction.a];
                Value val = frame->chunk->constants[instruction.b];
                string prop_name = val.stringValue;
                Value obj_val = frame->registers[instruction.c];
                
                setProperty(object, prop_name, obj_val);
                frame->registers[instruction.c] = object;

                break;
            }
                
                // SetPropertyDynamic: objReg, propReg, valueReg
                // emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, resultReg);
            case TurboOpCode::SetPropertyDynamic: {
                auto object = frame->registers[instruction.a];
                string prop_name = frame->registers[instruction.b].stringValue;
                setProperty(object, prop_name, frame->registers[instruction.c]);
                
                frame->registers[instruction.c] = object;

                break;
            }
                
                // TurboOpCode::GetPropertyDynamic, lhsReg, objReg, propReg
            case TurboOpCode::GetPropertyDynamic: {
                auto object = frame->registers[instruction.b];
                string prop = frame->registers[instruction.c].toString();
                Value val = getProperty(object, prop);
                frame->registers[instruction.a] = val;
                break;
            }
                
                // TurboOpCode::GetProperty, lhsReg, objReg, nameIdx
            case TurboOpCode::GetProperty: {
                Value object = frame->registers[instruction.b];
                string prop = frame->chunk->constants[instruction.c].stringValue;
                Value val = getProperty(object, prop);
                frame->registers[instruction.a] = val;
                break;
            }
                
                // TurboOpCode::GetObjectLength, lenReg, arrReg
            case TurboOpCode::GetObjectLength: {
                auto array = frame->registers[instruction.b];
                frame->registers[instruction.a] = getValueLength(array);
                break;
            }
                
                // keysReg, objReg
            case TurboOpCode::EnumKeys: {
                // object is in stack.
                Value objVal = frame->registers[instruction.b];
                
                auto obj = make_shared<JSObject>();
                
                int index = 0;
                // get the properties
                auto props = enumerateKeys(objVal);
                for (auto key : props) {
                    obj->set(to_string(index), key.first, "", {});
                    index++;
                }
                // pop obj
                frame->registers[instruction.a] = (Value::object(obj));

                break;
            }
                
            case TurboOpCode::Try: {
                uint32_t catchOffset = instruction.a;
                uint32_t finallyOffset = instruction.b;
                // compute absolute IPs
                int base = (int)frame->ip; // ip now points after the offsets
                TryFrame f;
                f.catchIP = (catchOffset == 0) ? -1 : (base + (int)catchOffset);
                f.finallyIP = (finallyOffset == 0) ? -1 : (base + (int)finallyOffset);
                // f.stackDepth = (int)stack.size();
                // ipAfterTry can be filled later by codegen if you want; keep -1 if unused
                f.ipAfterTry = -1;
                f.regCatch = instruction.c;
                tryStack.push_back(f);
                break;
            }

            case TurboOpCode::EndTry: {
                if (tryStack.empty()) {
                    // runtime error: unmatched END_TRY
                    // running = false;
                    break;
                }
                tryStack.pop_back();
                break;
            }
                
            case TurboOpCode::Throw: {
                // exception value on top of stack
                Value exc = frame->registers[instruction.a].toString();
                // unwind frames until we find a handler (catch or finally)
                bool handled = false;
                // Store a 'pending exception' to know we are unwinding due to throw
                Value pending = exc;
                
                while (!tryStack.empty()) {
                    TryFrame f = tryStack.back();
                    tryStack.pop_back();
                    
                    // first, unwind the value stack to the depth at try entry
                    // while ((int)stack.size() > f.stackDepth) stack.pop_back();
                    
                    // If there is a finally, run it first.
                    if (f.catchIP != -1) {
                        // push pending exception so finalizer can see it if needed
                        // stack.push_back(pending);
                        
                        // Record a special marker frame to indicate we are resuming a throw after finally
                        // We'll push a synthetic TryFrame with catchIP = original catchIP, finallyIP = -1
                        TryFrame resume;
                        resume.catchIP = -1;
                        resume.finallyIP = f.finallyIP;
                        // resume.stackDepth = f.stackDepth; // after finally resumes, stack depth should be this
                        resume.ipAfterTry = -1;
                        
                        frame->registers[f.regCatch] = exc;
                        
                        tryStack.push_back(resume);
                        
                        // jump into finalizer
                        frame->ip = f.catchIP;
                        
                        handled = true; // we will handle after finalizer/resume
                        break;
                    }
                    
                    // If no finally, but there is a catch, jump to catch and push exception
                    if (f.finallyIP != -1) {
                        // stack.push_back(pending);
                        frame->ip = f.finallyIP;
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
                    // running = false;
                }
                break;
            }
                
            case TurboOpCode::EndFinally: {
                // When a finally finishes, we must check whether we have a resume frame that carries a pending throw
                // Approach: if there is a TryFrame on tryStack whose catchIP != -1 and which we pushed as resume frame,
                // then either jump into catch or rethrow.
                if (!tryStack.empty()) {
                    TryFrame resume = tryStack.back();
                    // If resume.finallyIP == -1, we treat this as the resume frame we pushed earlier
                    if (resume.finallyIP == -1) {
                        tryStack.pop_back();
                        if (resume.finallyIP != -1) {
                            // pending exception should be on stack top
                            // jump into catch with exception on stack
                            frame->ip = resume.finallyIP;
                            break;
                        } else {
                            // no catch for the pending exception, continue unwinding:
                            // emulate throwing again: pop pending exception and re-run OP_THROW logic
                            // Value pending = pop();
                            // continue throw loop by re-inserting pending on stack and handling next frame
                            // simplest approach: directly re-run THROW handling by pushing pending and continuing
                            // For clarity, we will set a special behaviour: re-insert pending and emulate OP_THROW
                            // stack.push_back(pending);
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
                
                // TurboOpCode::LoadExceptionValue, ex_val_reg, idx
            case TurboOpCode::LoadExceptionValue: {
                
                int exception_value_register = instruction.a;
                int exception_value_index = instruction.b;
                
                Value throw_value = frame->registers[exception_value_register];
                
                // load above into local index
                frame->locals[exception_value_index] = throw_value;
                
                break;
            }

            case TurboOpCode::PushArg: {
                int argReg = instruction.a;
                argStack.push_back(frame->registers[argReg]);
                break;
            }
                
                // TurboOpCode::LoadArgument, reg
            case TurboOpCode::LoadArgument: {
                int argIndex = frame->registers[instruction.a].numberValue;
                
                Value result = Value::undefined();
                if (argIndex < frame->args.size()) {
                    result = frame->args[argIndex];
                }
                frame->registers[instruction.a] = (result);
                break;
            }
                
                // LoadArguments, arg_array_reg
            case TurboOpCode::LoadArguments: {
                // Pushes the full arguments array as a JSArray object (or equivalent)

                auto arr = make_shared<JSArray>();
                for (const Value& v : frame->args) {
                    arr->push({v});
                }
                frame->registers[instruction.a] = Value::array(arr);
                break;
            }

                // TurboOpCode::Slice, arg_array_reg, i_reg
            case TurboOpCode::Slice: {
                // Expects: [array, start] on stack; pops both and pushes array.slice(start)
                Value startVal = frame->registers[instruction.b];
                Value arrayVal = frame->registers[instruction.a];

                int start = (int)startVal.numberValue;
                shared_ptr<JSArray> inputArr = arrayVal.arrayValue;
                auto arr = make_shared<JSArray>();

                if (inputArr && start < (int)inputArr->length()) {
                    for (int i = start; i < (int)inputArr->length(); ++i) {
                        arr->push({ inputArr->get(to_string(i)) });
                    }
                }
                
                // push(Value::array(arr));
                frame->registers[instruction.a] = Value::array(arr);

                break;
                
            }

            case TurboOpCode::LoadArgumentsLength: {
                // Pushes the count of arguments passed to the current frame
                frame->registers[instruction.a] = Value((double)frame->args.size());
                break;
            }

            case TurboOpCode::CreateClosure: {
                
                int ci = frame->registers[instruction.a].numberValue;
                
                Value fnVal = module_->constants[ci];
                auto fnRef = fnVal.fnRef; // FunctionObject*
                auto closure = make_shared<Closure>();
                closure->fn = fnRef;

                frame->registers[instruction.a] = Value::closure(closure);

                break;
            }
                
                // --- Upvalue access ---
                //case TurboOpCode::GetUpvalue: {
//                    uint32_t idx = readUint32();
//                    push(*frame->closure->upvalues[idx]->location);
                //    break;
                //}
                //case TurboOpCode::SetUpvalue: {
//                    uint32_t idx = readUint32();
//                    *frame->closure->upvalues[idx]->location = pop();
                //    break;
                //}
                    
            case TurboOpCode::CloseUpvalue: {
                //                    closeUpvalues(stack.empty() ? nullptr : &stack.back());
                //                    pop();
                // TODO: check this works
                closeUpvalues(nullptr);
                break;
            }
                
                // LoadUpvalue, reg_slot, upvalue
            case TurboOpCode::LoadUpvalue: {
                uint32_t idx = instruction.b;
                frame->registers[instruction.a] = *frame->closure->upvalues[idx]->location;

                break;
            }
                
                // StoreUpvalueVar, upvalue, reg_slot
            case TurboOpCode::StoreUpvalueVar: {
                *frame->closure->upvalues[instruction.a]->location = frame->registers[instruction.b];
                break;
            }
            case TurboOpCode::StoreUpvalueLet: {
                *frame->closure->upvalues[instruction.a]->location = frame->registers[instruction.b];
                break;
            }
            case TurboOpCode::StoreUpvalueConst: {
                *frame->closure->upvalues[instruction.a]->location = frame->registers[instruction.b];
                break;
            }
                
                // TurboOpCode::SetClosureIsLocal, isLocalReg, closureChunkIndexReg);
            case TurboOpCode::SetClosureIsLocal: {
                int idx = frame->registers[instruction.a].numberValue;
                frame->registers[instruction.b].closureValue->upvalues.push_back(captureUpvalue(&frame->locals[idx]));
                break;
            }
                
                // TurboOpCode::SetClosureIndex, indexReg, closureChunkIndexReg
            case TurboOpCode::SetClosureIndex: {
                int idx = frame->registers[instruction.a].numberValue;
                frame->registers[instruction.b].closureValue->upvalues.push_back(frame->closure->upvalues[idx]);

                break;
            }
                
                // emit(TurboOpCode::Call, result, funcReg, (int)argRegs.size());
//            Where:
//
//            funcReg → register containing the function (the callable object or closure)
//            argStart → the index of the first argument register
//            argCount → how many arguments are being passed
                
                
            case TurboOpCode::Call: {
                
                int resultReg = instruction.a;
                int funcReg   = instruction.b;
                // int argc      = instruction.c;
                
                Value func = frame->registers[funcReg];
                
                const vector<Value> const_args = { argStack.begin(), argStack.end() };
                argStack.clear();

                Value result = callFunction(func, const_args);
                frame->registers[resultReg] = result;

                break;
            }
                
            case TurboOpCode::SuperCall: {
                break;
            }
                
            case TurboOpCode::Return: {
                Value v = frame->registers[instruction.a];
                                
                return v;
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

Value TurboVM::callMethod(Value callee, vector<Value>& args, Value js_object) {

    return callFunction(callee, args);
    
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

        // Set frame.closure if calling a closure
        // Assuming callee has a closurePtr member for Closure shared_ptr
        // If you extended Value for closure type, set here:
        callStack.push_back(std::move(new_frame));

        Value result = runFrame(callStack.back());
        
        callStack.pop_back();
        frame = &callStack.back();
        
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
        
    // copy args into frame.locals[0..]
    uint32_t ncopy = std::min<uint32_t>((uint32_t)args.size(), calleeChunk->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) new_frame.locals[i] = args[i];

    // push frame and execute it
    callStack.push_back(std::move(new_frame));

    Value result = runFrame(callStack.back());
    
    callStack.pop_back();
    frame = &prev_frame;

    return result;
    
}

void TurboVM::handleRethrow() {
    // simplified: pop pending exception and re-run OP_THROW-like unwinding
//    if (stack.empty()) { /*running = false;*/ return; }
//    Value pending = pop();
    // Re-run throw loop: same as OP_THROW handling but without recursion here.
    bool handled = false;
    while (!tryStack.empty()) {
        TryFrame f = tryStack.back();
        tryStack.pop_back();
        //while ((int)stack.size() > f.stackDepth) stack.pop_back();
        
        if (f.catchIP != -1) {
            
            //stack.push_back(pending);
            
            TryFrame resume;
            resume.catchIP = -1;
            resume.finallyIP = f.finallyIP;
            // resume.stackDepth = f.stackDepth;
            tryStack.push_back(resume);
            
            frame->ip = f.catchIP;
            
            handled = true;
            break;
        }
        
        if (f.finallyIP != -1) {
            //stack.push_back(pending);
            frame->ip = f.finallyIP;
            handled = true;
            break;
        }
        
    }
    if (!handled) {
        printf("Uncaught exception after finally, halting VM\n");
        //running = false;
    }
}
