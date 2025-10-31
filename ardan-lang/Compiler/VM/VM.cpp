//
//  VM.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "VM.hpp"

VM::VM() {

    env = new Env();
    init_builtins();

}

VM::VM(shared_ptr<Module> module_) : module_(module_) {

    env = new Env();
    init_builtins();
    
}

VM::~VM() {
    if (env != nullptr) {
        delete env;
    }
}

void VM::init_builtins() {
    
    event_loop = new EventLoop();
    
    env->set_var("Math", make_shared<Math>());
    env->set_var("console", make_shared<Print>());
    env->set_var("fs", make_shared<File>());
    env->set_var("Server", make_shared<Server>(event_loop));
    
    env->set_var("print", Value::function([this](vector<Value> args) mutable -> Value {
        Print::print(args);
        return Value::nullVal();
    }));
    
}

std::shared_ptr<Upvalue> VM::captureUpvalue(Value* local) {
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

void VM::closeUpvalues(Value* last) {
    while (openUpvalues && openUpvalues->location >= last) {
        openUpvalues->closed = *openUpvalues->location;
        openUpvalues->location = &openUpvalues->closed;
        openUpvalues = openUpvalues->next;
    }
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
    if (frame->ip >= frame->chunk->code.size()) return 0;
    return frame->chunk->code[frame->ip++];
}

uint32_t VM::readUint32() {
    if (frame->ip + 4 > frame->chunk->code.size()) throw std::runtime_error("read past end");
    uint32_t v = 0;
    v |= (uint32_t)frame->chunk->code[frame->ip++];
    v |= (uint32_t)frame->chunk->code[frame->ip++] << 8;
    v |= (uint32_t)frame->chunk->code[frame->ip++] << 16;
    v |= (uint32_t)frame->chunk->code[frame->ip++] << 24;
    return v;
}

uint8_t VM::readUint8() {
    return readByte();
}

void VM::CreateObjectLiteralProperty(Value obj_val, string prop_name, Value object) {
    if (obj_val.type == ValueType::CLOSURE) {
        
        shared_ptr<Closure> new_closure = make_shared<Closure>();
        new_closure->fn = obj_val.closureValue->fn;
        new_closure->upvalues = obj_val.closureValue->upvalues;
        new_closure->js_object = object.objectValue;

        object.objectValue->set(prop_name, Value::closure(new_closure), "VAR", { "public" });

    } else {
                    
        object.objectValue->set(prop_name, obj_val, "VAR", { "public" });

    }

}

int VM::getValueLength(Value& v) {
    
    if (v.type == ValueType::OBJECT) {
        return (int)v.objectValue->get_all_properties().size();
    }
    
    if (v.type == ValueType::ARRAY) {
        return v.arrayValue->get("length").numberValue;
    }
    
    return v.numberValue;

}

Value VM::getProperty(const Value &objVal, const string &propName) {
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
                continue;
            }
            
            if (modifier == "protected") {
                isProtected = true;
                continue;
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
                continue;
            }
            
            if (modifier == "protected") {
                isProtected = true;
                continue;
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
    if (objVal.type == ValueType::STRING) {
        
        auto jsString = make_shared<JSString>();
        
        shared_ptr<JSObject> native_object = jsString->construct();

        Value obj_value = Value::object(native_object);
        obj_value.objectValue->vm = this;
        
        vector<Value> args = { objVal.toString() };

        invokeMethod(obj_value, "constructor", args);

        return native_object->get(propName);
        
    }
    
    if (objVal.type == ValueType::NUMBER) {
    }
    
    if (objVal.type == ValueType::BOOLEAN) {
    }

    return Value::undefined();
}

void VM::setStaticProperty(const Value &objVal, const string &propName, const Value &val) {
    if (objVal.type == ValueType::CLASS) {
        // objVal.classValue->set_static_vm(propName, val);
        return;
    }
    throw std::runtime_error("Cannot set static property on non-class");
}

const unordered_map<string, Value> VM::enumerateKeys(Value obj) {
    
    if (obj.type == ValueType::OBJECT) {
        return obj.objectValue->get_all_properties();
    }
    
    if (obj.type == ValueType::ARRAY) {
        return obj.arrayValue->get_indexed_properties();
    }
    
    return {};
    
}

void VM::set_js_object_closure(Value objVal) {
    if (objVal.type == ValueType::OBJECT) {
        
        objVal.objectValue->vm = this;

        for(auto& prop : objVal.objectValue->get_all_properties()) {
            if (prop.second.type == ValueType::CLOSURE) {
                prop.second.closureValue->js_object = objVal.objectValue;
                
            }
        }
    }
}

shared_ptr<JSObject> VM::createJSObject(shared_ptr<JSClass> klass) {
    
    shared_ptr<JSObject> object = make_shared<JSObject>();
    object->vm = this;
    object->setClass(klass);

    makeObjectInstance(Value::klass(klass), object);
    
    // create a jsobject from superclass and assign to parent_object
    if (klass->superClass != nullptr) {
        
        object->parent_object = klass->superClass->is_native ? klass->superClass->construct() : createJSObject(klass->superClass);
        object->parent_class = klass->superClass;
        
    }

    return object;

}

void VM::makeObjectInstance(Value klass, shared_ptr<JSObject> obj) {
    
    // TODO: check out where var_proto_props and const_proto_props are set.
    for (auto& protoProp : klass.classValue->var_proto_props) {
                
        if (protoProp.second.value.type == ValueType::CLOSURE) {
            
            shared_ptr<Closure> new_closure = make_shared<Closure>();
            new_closure->fn = protoProp.second.value.closureValue->fn;
            new_closure->upvalues = protoProp.second.value.closureValue->upvalues;
            new_closure->js_object = obj;

            obj->set(protoProp.first, Value::closure(new_closure), "VAR", protoProp.second.modifiers);

        } else {
            
            // evaluate fields
            int field_reg = protoProp.second.value.numberValue;
            Value fnValue = module_->constants[field_reg];
            Value val = callFunction(fnValue, {});
                        
            obj->set(protoProp.first, val, "VAR", protoProp.second.modifiers);

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
            Value fnValue = module_->constants[field_reg];
            Value val = callFunction(fnValue, {});

            obj->set(constProtoProp.first, constProtoProp.second.value, "CONST", constProtoProp.second.modifiers);

        }

    }

}

void VM::invokeMethod(Value obj_value, string name, vector<Value> args) {
    
    if (obj_value.type == ValueType::OBJECT) {
        Value constructor = obj_value.objectValue->get(name);
        // TODO: check whether to throw error
        if (constructor.type != ValueType::UNDEFINED) {
            callMethod(constructor, args, obj_value);
        }
    }
    
}

Value VM::CreateInstance(Value klass) {
    
    if (klass.classValue->is_native == true) {
        
        // add constructor
        // check if constructor exists
        if (!klass.classValue->is_constructor_available()) {
            klass.classValue->set_proto_vm_var("constructor", addCtor(), { "public" } );
        }

        shared_ptr<JSObject> native_object = klass.classValue->construct();

        Value obj_value = Value::object(native_object);
        obj_value.objectValue->vm = this;

        set_js_object_closure(obj_value);

        return obj_value;

    }

    // auto obj = make_shared<JSObject>();
    // obj->setClass(klass.classValue);
    
    // TODO: we need to invoke parent constructor

    shared_ptr<JSObject> obj = createJSObject(klass.classValue);
    
    Value obj_value = Value::object(obj);
    
    return obj_value;

}

void VM::InvokeConstructor(Value obj_value, vector<Value> args) {
    
    invokeMethod(obj_value, "constructor", args);
    
    // call super class constructor
    
    if (obj_value.objectValue->parent_object != nullptr) {
        InvokeConstructor(Value::object(obj_value.objectValue->parent_object), args);
    }

}

Value VM::addCtor() {

    shared_ptr<Chunk> fnChunk = make_shared<Chunk>();
    fnChunk->arity = 0;

    fnChunk->writeByte(static_cast<uint8_t>(OpCode::Nop));
    fnChunk->writeByte(static_cast<uint8_t>((OpCode::SuperCall)));
    fnChunk->writeUint8((uint8_t)0);

    int constant_index = fnChunk->addConstant(Value::undefined());
    
    fnChunk->writeByte(static_cast<uint8_t>(OpCode::LoadConstant));
    fnChunk->writeUint32(constant_index);
    fnChunk->writeByte(static_cast<uint8_t>(OpCode::Return));
    
    uint32_t chunkIndex = module_->addChunk(fnChunk);

    auto fnObj = std::make_shared<FunctionObject>();
    fnObj->chunkIndex = chunkIndex;
    fnObj->arity = fnChunk->arity;
    fnObj->name = "ctor";
    fnObj->upvalues_size = 0;

    Value fnValue = Value::functionRef(fnObj);
    // int ci = module_->addConstant(fnValue);

    shared_ptr<Closure> new_closure = make_shared<Closure>();
    new_closure->fn = fnObj;
    new_closure->upvalues = {};
    
    return Value::closure(new_closure);

}

Value VM::run(shared_ptr<Chunk> chunk_, const vector<Value>& args) {
    
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

Value VM::runFrame(CallFrame &current_frame) {
    
    if (callStack.empty()) return Value::undefined();

    frame = &current_frame;

    while (true) {
        OpCode op = static_cast<OpCode>(readByte());
        switch (op) {
            case OpCode::Nop:
                break;

                // pushes the constant to stack
            case OpCode::LoadConstant: {
                uint32_t ci = readUint32();
                push(frame->chunk->constants[ci]);
                break;
            }

            case OpCode::Pop: {
                pop();
                break;
            }

            case OpCode::Dup: {
                Value v = peek(0);
                push(v);
                break;
            }
                
            case OpCode::Dup2: {
                Value b = stack[stack.size() - 1];
                Value a = stack[stack.size() - 2];
                stack.push_back(a);
                stack.push_back(b);
                break;
            }
                
                // TODO: marked for removal
            case OpCode::LoadLocal: {
                uint32_t idx = readUint32();
                if (idx >= frame->locals.size()) push(Value::undefined());
                else push(frame->locals[idx]);
                break;
            }

                // TODO: marked for removal
            case OpCode::StoreLocal: {
                uint32_t idx = readUint32();
                Value val = pop();
                if (idx >= frame->locals.size()) {
                    // grow if needed
                    frame->locals.resize(idx + 1, Value::undefined());
                }
                frame->locals[idx] = val;
                //push(val);
                break;
            }

                // pushes the value of the global to stack
                // TODO: marked for removal
            case OpCode::LoadGlobal: {
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                                
                try {
                    
                    R env_value = env->get(name);
                    
                    Value env_value_conv = toValue(env_value);
                    
                    push(env_value_conv);
                    
                } catch(...) {
                    push(Value::undefined());
                }
                
                break;
            }

                // leaves nothing on stack
                // TODO: marked for removal
            case OpCode::StoreGlobal: {
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_var(name, v);
                
                break;
            }

                // TODO: marked for removal
            case OpCode::CreateGlobal: {
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_var(name, v);
                
                break;
            }

                // moves data from local into stack
            case OpCode::LoadLocalVar: {
                // local index
                uint32_t idx = readUint32();
                
                push(frame->locals[idx]);

                break;
            }
                
            case OpCode::LoadGlobalVar: {
                
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                
                R env_value = env->get(name);
                
                Value env_value_conv = toValue(env_value);
                
                push(env_value_conv);
                
                break;
            }
                
            case OpCode::StoreLocalVar: {
                
                uint32_t idx = readUint32();
                Value val = pop();
                
                frame->locals[idx] = val;
                
                break;
            }
                
            case OpCode::StoreGlobalVar: {
                
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_var(name, v);
                
                break;
            }
                
            case OpCode::StoreLocalLet: {
                
                uint32_t idx = readUint32();
                Value val = pop();
                frame->locals[idx] = val;
                
                break;
            }
                
            case OpCode::StoreGlobalLet: {
                
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_let(name, v);
                
                break;
            }

            case OpCode::CreateLocalVar: {
                
                uint32_t idx = readUint32();
                Value val = pop();
                
                frame->locals[idx] = val;
                
                break;
            }
                
            case OpCode::CreateLocalLet: {
                
                uint32_t idx = readUint32();
                Value val = pop();
                frame->locals[idx] = val;
                
                break;
            }
                
            case OpCode::CreateLocalConst: {
                
                uint32_t idx = readUint32();
                Value val = pop();
                frame->locals[idx] = val;
                
                break;
            }
                
            case OpCode::CreateGlobalVar: {
                
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_var(name, v);
                
                break;
            }
                
            case OpCode::CreateGlobalLet: {
                
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_let(name, v);
                
                break;
            }
                
            case OpCode::CreateGlobalConst: {
                
                uint32_t ci = readUint32();
                Value nameVal = frame->chunk->constants[ci];
                string name = nameVal.toString();
                Value v = pop();
                
                env->set_const(name, v);
                
                break;
            }
                
            case OpCode::StoreThisProperty: {
                
                uint32_t idx = readUint32();
                
                // load constant from nameIdx
                Value property_value = frame->chunk->constants[idx];
                string property_name = property_value.toString();
                
                Value value = pop();
                
                setProperty(Value::object(frame->closure->js_object), property_name, value);

                break;
            }
                
            case OpCode::LoadUpvalue: {
                
                uint32_t idx = readUint32();
                push(*frame->closure->upvalues[idx]->location);

                break;
            }
                
            case OpCode::StoreUpvalueVar: {
                
                Value v = pop();
                uint32_t idx = readUint32();

                *frame->closure->upvalues[idx]->location = v;

                break;
            }
                
            case OpCode::StoreUpvalueLet: {
                
                Value v = pop();
                uint32_t idx = readUint32();

                *frame->closure->upvalues[idx]->location = v;

                break;
            }
                
            case OpCode::StoreUpvalueConst: {
                
                Value v = pop();
                uint32_t idx = readUint32();

                *frame->closure->upvalues[idx]->location = v;

                break;
            }
                
            case OpCode::LoadThisProperty: {
                
                uint32_t idx = readUint32();
                
                // load constant from nameIdx
                Value property_value = frame->chunk->constants[idx];
                
                string property_name = property_value.toString();
                                
                Value obj = getProperty(Value::object(frame->closure->js_object), property_name);
                
                push(obj);

                break;
            }
                
            case OpCode::TypeOf: {
                
                Value value = pop();
                push(Value::str(type_of(value)));

                break;
            }
                
                // Delete: objReg, propertyReg
            case OpCode::Delete: {
                
                Value property = pop();
                Value object = pop();
                
                push(Value::boolean(delete_op(object, property)));
                
                break;
            }
                
            case OpCode::InstanceOf: {
                
                Value a = pop();
                Value b = pop();
                
                push(Value::boolean(instance_of(a,b)));

                break;
                
            }
                
            case OpCode::In: {
                Value a = pop();
                Value b = pop();
                
                push(Value::boolean(in(a,b)));
                break;
            }
                
            case OpCode::Void: {
                pop();
                push(Value::undefined());
            }

                // stack: nameIdx
            case OpCode::CreateEnum: {
                
                Value enum_name_index = pop();
                
                auto enum_value_obj = createJSObject(make_shared<JSClass>());
                
                push(Value::object(enum_value_obj));

                break;
            }
                
            case OpCode::SetEnumProperty: {
                // stack: obj, property, value

                Value value = pop();
                Value prop_value = pop();
                Value enum_obj = pop();
                
                setProperty(enum_obj, prop_value.toString(), value);
                
                push(enum_obj);
                
                break;
            }

            case OpCode::NewObject: {
                auto obj = std::make_shared<JSObject>();
                Value v;
                v.type = ValueType::OBJECT;
                v.objectValue = obj;
                push(v);
                break;
            }
                
            case OpCode::CreateObjectLiteral: {
                Value objVal = pop();
                set_js_object_closure(objVal);
                push(objVal);
                
                break;
            }
                
            case OpCode::CreateObjectLiteralProperty: {
                
                uint32_t idx = readUint32();
                Value constant = frame->chunk->constants[idx];
                string prop_name = constant.toString();
                
                Value val = pop();
                
                Value object = pop();

                CreateObjectLiteralProperty(val, prop_name, object);

                push(object);
                
                break;
                
            }
                
            case OpCode::NewClass: {
                
                auto superclass = pop();
                
                auto js_class = make_shared<JSClass>();
                
                if (superclass.type == ValueType::CLASS) {
                    js_class->superClass = superclass.classValue;
                }
                
                Value klass = Value::klass(js_class);
                
                // add constructor
                klass.classValue->set_proto_vm_var("constructor", addCtor(), { "public" } );
                                
                push(klass);
                break;
            }
                
                // property var
            case OpCode::CreateClassPrivatePropertyVar: {

                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_var(prop, valueToSet, { "private" } );

                // push object back
                push(klassVal);
                
                break;
            }
                
            case OpCode::CreateClassPublicPropertyVar: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_var(prop, valueToSet, { "public" } );

                // push object back
                push(klassVal);

                break;
                
            }
                
            case OpCode::CreateClassProtectedPropertyVar: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_var(prop, valueToSet, { "protected" } );

                // push object back
                push(klassVal);

                break;
                
            }
                
                // property const
            case OpCode::CreateClassPrivatePropertyConst: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_const(prop, valueToSet, { "private" } );

                // push object back
                push(klassVal);

                break;
            }
                
            case OpCode::CreateClassPublicPropertyConst: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_const(prop, valueToSet, { "public" } );

                // push object back
                push(klassVal);

                break;
            }
                
            case OpCode::CreateClassProtectedPropertyConst: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_const(prop, valueToSet, { "protected" } );

                // push object back
                push(klassVal);

                break;
            }
                
                // static var
            case OpCode::CreateClassPrivateStaticPropertyVar: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_var(prop, valueToSet, { "private" });
                
                push(objVal);

                break;
                
            }
                
            case OpCode::CreateClassPublicStaticPropertyVar: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_var(prop, valueToSet, { "public" });
                
                push(objVal);
                
                break;
                
            }
                
            case OpCode::CreateClassProtectedStaticPropertyVar: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_var(prop, valueToSet, { "protected" });
                
                push(objVal);
                
                break;
                
            }
                
                // static const
            case OpCode::CreateClassPrivateStaticPropertyConst: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_const(prop, valueToSet, { "private" });
                
                push(objVal);
                
                break;
            }
                
            case OpCode::CreateClassPublicStaticPropertyConst: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_const(prop, valueToSet, { "public" });
                
                push(objVal);
                

                break;
            }
                
            case OpCode::CreateClassProtectedStaticPropertyConst: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_const(prop, valueToSet, { "protected" });
                
                push(objVal);
                
                break;
                
            }
                
                // op, super_class_reg, method_reg, methodNameReg);
                
            case OpCode::CreateClassProtectedStaticMethod: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_var(prop, valueToSet, { "protected" });
                
                push(objVal);
                
                break;
            }
                
            case OpCode::CreateClassPrivateStaticMethod: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_var(prop, valueToSet, { "private" });
                
                push(objVal);
                break;
            }
                
            case OpCode::CreateClassPublicStaticMethod: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                
                Value valueToSet = pop();
                Value objVal = pop();
                
                objVal.classValue->set_var(prop, valueToSet, { "public" });
                
                push(objVal);
                break;
            }
                
            case OpCode::CreateClassProtectedMethod: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_var(prop, valueToSet, { "protected" } );

                // push object back
                push(klassVal);
                
                break;
            }
                
            case OpCode::CreateClassPrivateMethod: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_var(prop, valueToSet, { "private" } );

                // push object back
                push(klassVal);
                break;
            }
                
            case OpCode::CreateClassPublicMethod: {
                
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();

                Value valueToSet = pop();
                Value klassVal = pop();
                
                klassVal.classValue->set_proto_vm_var(prop, valueToSet, { "public" } );

                // push object back
                push(klassVal);
                
                break;
            }
                
            case OpCode::SuperCall: {
                
                vector<Value> args = popArgs(readUint8());

                // there must be an object in the frame.closure
                // This is because super() must be run inside a constructor.
                
                // if the parent_object is null, do not call
                // TODO: we need to know if we need to throw error and also if the parent must have a constructor
                // TODO: learn about constructor calling chain.
                if (frame->closure->js_object->parent_object != nullptr) {
                    
                    Value obj_value = Value::object(frame->closure->js_object->parent_object);
                    
                    invokeMethod(obj_value, "constructor", args);
                    
                }

                break;
            }
                
            case OpCode::GetThis: {
                push(Value::object(frame->closure->js_object));
                break;
            }
                
            case OpCode::SetThisProperty: {
                // this update the object the current object
                int index = readUint32();
                Value v = pop();
                string prop = frame->chunk->constants[index].toString();
                setProperty(Value::object(frame->closure->js_object), prop, v);

                break;
            }
                
            case OpCode::GetThisProperty: {
                
                int index = readUint32();
                string prop = frame->chunk->constants[index].toString();

                push(getProperty(Value::object(frame->closure->js_object), prop));
                
                break;
            }
                
            case OpCode::GetParentObject: {
                push(Value::object(frame->closure->js_object->parent_object));
                break;
            }
                
            case OpCode::CreateInstance: {
                                
                Value klass = pop();

//                if (klass.classValue->is_native == true) {
//                    
//                    // add constructor
//                    // check if constructor exists
//                    if (!klass.classValue->is_constructor_available()) {
//                        klass.classValue->set_proto_vm_var("constructor", addCtor(), { "public" } );
//                    }
//
//                    shared_ptr<JSObject> native_object = klass.classValue->construct();
//
//                    Value obj_value = Value::object(native_object);
//                    obj_value.objectValue->vm = this;
//
//                    set_js_object_closure(obj_value);
//
//                    // obj_value.type = ValueType::OBJECT;
//                    // obj_value.objectValue = native_klass;
//
//                    push(obj_value);
//
//                    break;
//                }
//
//                // auto obj = make_shared<JSObject>();
//                // obj->setClass(klass.classValue);
//                
//                // TODO: we need to invoke parent constructor
//
//                shared_ptr<JSObject> obj = createJSObject(klass.classValue);
//                
//                Value obj_value;
//                obj_value.type = ValueType::OBJECT;
//                obj_value.objectValue = obj;
                                
                Value obj_value = CreateInstance(klass);

                push(obj_value);

                break;
            }
                
            case OpCode::InvokeConstructor: {

                vector<Value> args = popArgs(readUint8());

                Value obj_value = pop();

                // call the constructor
                InvokeConstructor(obj_value, args);
                // invokeMethod(obj_value, "constructor", args);

                push(obj_value);
                
                break;
            }
                
            case OpCode::SetStaticProperty: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                Value valueToSet = pop();
                Value objVal = pop();
                setStaticProperty(objVal, prop, valueToSet);
                // push object back
                push(objVal);
                break;
            }
                
            case OpCode::SetProperty: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                Value valueToSet = pop();
                Value objVal = pop();
                setProperty(objVal, prop, valueToSet);
                // push object back
                push(objVal);
                break;
            }

            case OpCode::GetProperty: {
                uint32_t ci = readUint32();
                string prop = frame->chunk->constants[ci].toString();
                Value objVal = pop();
                Value v = getProperty(objVal, prop);
                push(v);
                break;
            }
                
            case OpCode::GetPropertyDynamic: {
                // object is on stack
                // property is on stack
                Value val = pop();
                Value objVal = pop();
                Value v = getProperty(objVal, val.toString());
                push(v);
                break;
            }
                
            case OpCode::GetIndexPropertyDynamic: {
                Value val = pop();
                Value objVal = pop();
                Value v = getProperty(objVal, val.toString());
                push(v);
                break;
            }
                
            case OpCode::GetObjectLength: {
                
                // object is in stack
                Value objVal = pop();
                
                int size = getValueLength(objVal);
                
                push(Value(size));
                
                break;
                
            }
                
            case OpCode::SetPropertyDynamic: {
                // Stack: ... obj, key, value
                Value value = pop(); //.back(); stack.pop_back();
                Value key = pop();//stack.back(); stack.pop_back();
                Value obj = pop();//stack.back();

                // Set property;
                setProperty(obj, key.toString(), value);

                // Remove obj from stack, push result if needed (often value or obj)
                // Here, we keep 'value' on top as result of assignment:
                //stack.back() = value;
                push(value);
                break;
            }
                
            case OpCode::EnumKeys: {
                // object is in stack.
                Value objVal = pop();
                
                auto obj = make_shared<JSObject>();
                
                int index = 0;
                // get the properties
                auto props = enumerateKeys(objVal);
                for (auto key : props) {
                    obj->set(to_string(index), key.first, "", {});
                    index++;
                }
                // pop obj
                push(Value::object(obj));
                break;
            }
                
            case OpCode::NewArray: {
                auto arr = std::make_shared<JSArray>();
                arr->vm = this;
                
                Value v;
                v.type = ValueType::ARRAY;
                v.arrayValue = arr;
                push(v);
                break;
            }

            case OpCode::ArrayPush: {
                Value val = pop();
                Value arrVal = pop();
                if (arrVal.type != ValueType::ARRAY)
                    throw std::runtime_error("ArrayPush: target not array");
                arrVal.arrayValue->push({val});
                push(arrVal);
                break;
            }

            case OpCode::Add: {
                Value b = pop();
                Value a = pop();
                push(binaryAdd(a,b));
                break;
            }
            case OpCode::Subtract: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numberValue - b.numberValue));
                break;
            }
            case OpCode::Multiply: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numberValue * b.numberValue));
                break;
            }
            case OpCode::Divide: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numberValue / b.numberValue));
                break;
            }
            case OpCode::Modulo: {
                Value b = pop();
                Value a = pop();
                push(Value(fmod(a.numberValue, b.numberValue)));
                break;
            }
            case OpCode::Power: {
                Value b = pop();
                Value a = pop();
                push(Value(pow(a.numberValue, b.numberValue)));
                break;
            }

            case OpCode::Negate: {
                Value a = pop();
                push(Value(-a.numberValue));
                break;
            }

            case OpCode::LogicalNot: {
                Value a = pop();
                push(Value::boolean(!isTruthy(a)));
                break;
            }
                
            case OpCode::Increment: {
                
                Value a = pop();
                Value b = pop();
                
                int sum = a.numberValue + b.numberValue;
                
                push(Value(sum));

                break;
            }

            case OpCode::Equal: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(equals(a,b)));
                break;
            }

            case OpCode::NotEqual: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(!equals(a,b)));
                break;
            }

            case OpCode::LessThan: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue < b.numberValue));
                break;
            }
                
            case OpCode::LessThanOrEqual: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue <= b.numberValue));
                break;
            }
                
            case OpCode::GreaterThan: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue > b.numberValue));
                break;
            }
                
            case OpCode::GreaterThanOrEqual: {
                Value b = pop();
                Value a = pop();
                push(Value::boolean(a.numberValue >= b.numberValue));
                break;
            }
                                
            case OpCode::LogicalAnd: {
                Value b = pop();
                Value a = pop();
                bool result = isTruthy(a) && isTruthy(b);
                push(Value::boolean(result));
                break;
            }

            case OpCode::LogicalOr: {
                Value b = pop();
                Value a = pop();
                bool result = isTruthy(a) || isTruthy(b);
                push(Value::boolean(result));
                break;
            }
                
            case OpCode::NullishCoalescing: {
                
                Value right = pop();  // fallback value
                Value left = pop();   // main value
                
                bool isNullish = left.isNull() || left.isUndefined();
                
                if (isNullish) {
                    push(right);
                } else {
                    push(left);
                }
                
                break;
            }
                
            case OpCode::StrictEqual: {
                Value b = pop();
                Value a = pop();
                bool isEqual = false;
                // For objects/arrays: we compare pointers
                if (a.type == b.type) {
                    if (a.type == ValueType::OBJECT)
                        isEqual = (a.objectValue == b.objectValue);
                    else if (a.type == ValueType::ARRAY)
                        isEqual = (a.arrayValue == b.arrayValue);
                    // For other types, we could default to pointer or identity check
                }
                push(Value::boolean(isEqual));
                break;
            }
                
            case OpCode::StrictNotEqual: {
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
                    // add any types we missed
                }
                push(Value::boolean(notEqual));
                break;
            }
                
            case OpCode::Decrement: {
                Value a = pop();
                Value b = pop();
                int sum = a.numberValue - b.numberValue;
                push(Value(sum));
                break;
            }
                
                // bitwise
            case OpCode::BitAnd: {
                Value b = pop();
                Value a = pop();
                int result = (int)a.numberValue & (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::BitOr: {
                Value b = pop();
                Value a = pop();
                int result = (int)a.numberValue | (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::BitXor: {
                Value b = pop();
                Value a = pop();
                int result = (int)a.numberValue ^ (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::ShiftLeft: {
                Value b = pop(); // shift amount
                Value a = pop(); // value to shift
                int result = (int)a.numberValue << (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::ShiftRight: {
                Value b = pop(); // shift amount
                Value a = pop(); // value to shift
                int result = (int)a.numberValue >> (int)b.numberValue;
                push(Value(result));
                break;
            }

            case OpCode::UnsignedShiftRight: {
                Value b = pop(); // shift amount
                Value a = pop(); // value to shift
                unsigned int result = (unsigned int)a.numberValue >> (unsigned int)b.numberValue;
                push(Value(result));
                break;
            }
                
            case OpCode::Positive: {
                Value a = pop();
                Value b = pop();
                int result = (int)a.numberValue & (int)b.numberValue;
                push(Value(result));
                break;
            }

            // jumps
            case OpCode::Jump: {
                uint32_t offset = readUint32();
                frame->ip += offset;
                break;
            }

            case OpCode::JumpIfFalse: {
                uint32_t offset = readUint32();
                Value cond = pop();
                if (!isTruthy(cond)) frame->ip += offset;
                break;
            }

            case OpCode::Loop: {
                uint32_t offset = readUint32();
                // jump backwards
                frame->ip -= offset;
                break;
            }
                
            case OpCode::LoadChunkIndex: {
                
                uint32_t chunkIndex = readUint32();
                Value ci = module_->constants[chunkIndex];
                push(ci);

                break;
                
            }
                
            case OpCode::Call: {
                uint32_t argCount = readUint8();
                vector<Value> args = popArgs(argCount);
                Value callee = pop();
                Value result = callFunction(callee, args);
                
                push(result);
                break;
            }
                
            case OpCode::LoadArgument: {
                // Expects next 4 bytes: uint32_t index of argument to load
                uint32_t argIndex = readUint32();
                
                Value result = Value::undefined();
                if (argIndex < frame->args.size()) {
                    result = frame->args[argIndex];
                }
                push(result);
                break;
            }

            case OpCode::LoadArguments: {
                // Pushes the full arguments array as a JSArray object (or equivalent)

                auto arr = make_shared<JSArray>();
                for (const Value& v : frame->args) {
                    arr->push({v});
                }
                push(Value::array(arr));
                break;
            }

            case OpCode::Slice: {
                // Expects: [array, start] on stack; pops both and pushes array.slice(start)
                Value startVal = pop();
                Value arrayVal = pop();

                int start = (int)startVal.numberValue;
                shared_ptr<JSArray> inputArr = arrayVal.arrayValue;
                auto arr = make_shared<JSArray>();

                if (inputArr && start < (int)inputArr->length()) {
                    for (int i = start; i < (int)inputArr->length(); ++i) {
                        arr->push({ inputArr->get(to_string(i)) });
                    }
                }
                push(Value::array(arr));
                break;
            }

            case OpCode::LoadArgumentsLength: {
                // Pushes the count of arguments passed to the current frame
                
                int size = 0;
                
                for (auto arg : frame->args) {
                    if (arg.type == ValueType::UNDEFINED) {
                        continue;
                    }
                    size++;
                }

                push(Value((double)size));
                break;
            }
                
            case OpCode::Try: {
                uint32_t catchOffset = readUint32();   // relative offset from after the two offsets
                uint32_t finallyOffset = readUint32();
                // compute absolute IPs
                int base = (int)frame->ip; // ip now points after the offsets
                TryFrame f;
                f.catchIP = (catchOffset == 0) ? -1 : (base + (int)catchOffset);
                f.finallyIP = (finallyOffset == 0) ? -1 : (base + (int)finallyOffset);
                f.stackDepth = (int)stack.size();
                // ipAfterTry can be filled later by codegen if you want; keep -1 if unused
                f.ipAfterTry = -1;
                tryStack.push_back(f);
                break;
            }

            case OpCode::EndTry: {
                if (tryStack.empty()) {
                    // runtime error: unmatched END_TRY
                    //running = false;
                    break;
                }
                tryStack.pop_back();
                break;
            }
                
            case OpCode::Throw: {
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
                        frame->ip = f.finallyIP;
                        handled = true; // we will handle after finalizer/resume
                        break;
                    }
                    
                    // If no finally, but there is a catch, jump to catch and push exception
                    if (f.catchIP != -1) {
                        stack.push_back(pending);
                        frame->ip = f.catchIP;
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
                    //running = false;
                }
                break;
            }
                
            case OpCode::EndFinally: {
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
                            frame->ip = resume.catchIP;
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
                
            case OpCode::Return: {
                // Close any open upvalues pointing to locals being popped
                closeUpvalues(frame->locals.size() > 0 ? &frame->locals[0] : nullptr);
                Value v = pop();
                                
                return v;
            }

            case OpCode::Halt:
                return Value::undefined();

            case OpCode::CreateClosure: {
                uint32_t ci = readUint8();
                Value fnVal = module_->constants[ci]; // chunk->constants[ci];
                auto fnRef = fnVal.fnRef; // FunctionObject*
                auto closure = make_shared<Closure>();
                closure->fn = fnRef;

                // Push a Value wrapping this closure
                push(Value::closure(closure)); // Adjust to push closure Value if implemented

                for (size_t i = 0; i < fnRef->upvalues_size; ++i) {
                    // Read upvalue info (isLocal, index)
                    uint32_t isLocal = readUint8();
                    uint32_t idx = readUint8();
                    if (isLocal) {
                        closure->upvalues.push_back(captureUpvalue(&frame->locals[idx]));
                    } else {
                        closure->upvalues.push_back(frame->closure->upvalues[idx]);
                    }
                }
                
                break;
            }

            // --- Upvalue access ---
            case OpCode::GetUpvalue: {
                uint32_t idx = readUint32();
                push(*frame->closure->upvalues[idx]->location);
                break;
            }
            case OpCode::SetUpvalue: {
                uint32_t idx = readUint32();
                *frame->closure->upvalues[idx]->location = pop();
                break;
            }
                
            case OpCode::CloseUpvalue: {
                closeUpvalues(stack.empty() ? nullptr : &stack.back());
                pop();
                break;
            }
                
            case OpCode::ClearLocals: {
                frame->locals.clear();
                break;
            }
                
            case OpCode::ClearStack: {
                stack.clear();
                break;
            }

            default:
                throw std::runtime_error("Unhandled opcode");
        }
    }
    return Value::undefined();
}

Value VM::callMethod(Value callee, vector<Value>& args, Value js_object) {

    return callFunction(callee, args);

}

Value VM::callFunction(Value callee, const vector<Value>& args) {
    
    if (callee.type == ValueType::FUNCTION) {
        Value result = callee.functionValue(args);
        return result;
    }
    
    if (callee.type == ValueType::CLOSURE) {

        shared_ptr<Chunk> calleeChunk = module_->chunks[callee.closureValue->fn->chunkIndex];
        
        // Build new frame
        CallFrame new_frame;
        new_frame.chunk = calleeChunk;
        new_frame.ip = 0;
        // allocate locals sized to the chunk's max locals (some chunks use maxLocals)
        new_frame.locals.resize(calleeChunk->maxLocals, Value::undefined());
        new_frame.args = args;
        new_frame.closure = callee.closureValue;

        callStack.push_back(std::move(new_frame));
        auto prev_stack = std::move(stack);
        stack.clear();

        Value result = runFrame(callStack.back());
        
        callStack.pop_back();
        frame = &callStack.back();
        
        stack = std::move(prev_stack);

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
    
    shared_ptr<Chunk> calleeChunk = module_->chunks[fn->chunkIndex];

    // Build new frame
    CallFrame new_frame;
    new_frame.chunk = calleeChunk;
    new_frame.ip = 0;
    // allocate locals sized to the chunk's max locals (some chunks use maxLocals)
    new_frame.locals.resize(calleeChunk->maxLocals, Value::undefined());
    new_frame.args = args;
    
    new_frame.closure = callee.closureValue;
    
    auto prev_stack = std::move(stack);
    stack.clear();
    
    // copy args into frame.locals[0..]
    uint32_t ncopy = std::min<uint32_t>((uint32_t)args.size(), calleeChunk->maxLocals);
    for (uint32_t i = 0; i < ncopy; ++i) new_frame.locals[i] = args[i];

    // push frame and execute it
    callStack.push_back(std::move(new_frame));

    Value result = runFrame(callStack.back());
    callStack.pop_back();

    frame = &callStack.back();

    stack = prev_stack;

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
    if (stack.empty()) { /*running = false;*/ return; }
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
            frame->ip = f.finallyIP;
            handled = true;
            break;
        }
        if (f.catchIP != -1) {
            stack.push_back(pending);
            frame->ip = f.catchIP;
            handled = true;
            break;
        }
    }
    if (!handled) {
        printf("Uncaught exception after finally, halting VM\n");
        //running = false;
    }
}

