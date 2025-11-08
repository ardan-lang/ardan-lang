//
//  TurboVM.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include <thread>
#include "PeregrineVM.hpp"

PeregrineVM::PeregrineVM() {
    env = new Env();
    init_builtins();

}

PeregrineVM::PeregrineVM(shared_ptr<TurboModule> module_) : module_(module_) {

    env = new Env();
    init_builtins();
    
}

PeregrineVM::~PeregrineVM() {
    
    thread stopper([this]() {
        sleep(0);
        cout << "[Stopper] Calling stop()" << endl;
        event_loop->stop();
    });
    
    event_loop->run();
    
    stopper.join();
    
    if (env != nullptr) {
        delete env;
    }
    
    if (executionCtx != nullptr) {
        delete executionCtx;
    }
    
//    if (event_loop != nullptr) {
//        delete event_loop;
//    }
    
}

void PeregrineVM::init_builtins() {
    
    event_loop = &EventLoop::getInstance();

    env->set_var("Math", make_shared<Math>());
    env->set_var("console", make_shared<Print>());
    env->set_var("fs", make_shared<File>());
    env->set_var("Server", make_shared<Server>(event_loop));
    env->set_var("Promise", make_shared<JSPromise>(this));

    env->set_var("String", make_shared<JSString>());
    env->set_var("Number", make_shared<JSNumber>());
    env->set_var("Boolean", make_shared<JSBoolean>());
    env->set_var("Array", make_shared<Array>());

    env->set_var("print", Value::function([this](vector<Value> args) mutable -> Value {
        Print::print(args);
        return Value::nullVal();
    }));
    
    env->set_var("Window", make_shared<Window>());
    env->set_var("Button", make_shared<Button>());
    env->set_var("View", make_shared<View>());
    env->set_var("Text", make_shared<Text>());
    env->set_var("VStack", make_shared<VStack>());
    env->set_var("HStack", make_shared<HStack>());
    
    ExecutionContext* ctx = new ExecutionContext();
    ctx->lexicalEnv = make_shared<Env>(env);
    ctx->variableEnv = make_shared<Env>(env);
    contextStack.push_back(ctx);

    executionCtx = contextStack.back();

}

Value PeregrineVM::getVariable(const string& key) const {
    R value = (executionCtx->lexicalEnv->getValueWithoutThrow(key));
    
    if (std::holds_alternative<std::nullptr_t>(value)) {
        value = (executionCtx->variableEnv->getValueWithoutThrow(key));
        
        if (std::holds_alternative<std::nullptr_t>(value)) {
            throw runtime_error("Undefined variable: " + key);
        }
    }
    
    return toValue(value);
    
}

void PeregrineVM::putVariable(const string& key, const Value& v) const {
    
    Env* target = executionCtx->lexicalEnv->resolveBinding(key, executionCtx->lexicalEnv.get());
    if (target) {
        target->set_let(key, v);
        return;
    }
    
    target = executionCtx->variableEnv->resolveBinding(key, executionCtx->variableEnv.get());
    
    if (target) {
        target->set_var(key, v);
        return;
    }

}

Instruction PeregrineVM::readInstruction() {
    if (frame->ip >= frame->chunk->code.size())
        throw runtime_error("Empty instruction.");
    return frame->chunk->code[frame->ip++];
}

Value PeregrineVM::CreateInstance(Value klass) {
    
    if (klass.classValue->is_native == true) {
        
        // add constructor
        // check if constructor exists
        if (!klass.classValue->is_constructor_available()) {
            klass.classValue->set_proto_vm_var("constructor", addCtor(), { "public" } );
        }

        shared_ptr<JSObject> native_object = klass.classValue->construct();

        Value obj_value = Value::object(native_object);
        // obj_value.objectValue->turboVM = this;

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

Value PeregrineVM::getProperty(const Value &objVal, const string &propName) {
    
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
    
    if (objVal.type == ValueType::PROMISE) {
        return objVal.promiseValue->get(propName);
    }
    
    // primitives -> string -> property? For now, undefined
    if (objVal.type == ValueType::STRING) {
        
        auto jsString = make_shared<JSString>();
        
        shared_ptr<JSObject> native_object = jsString->construct();

        Value obj_value = Value::object(native_object);
        //obj_value.objectValue->turboVM = this;
        
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

void PeregrineVM::set_js_object_closure(Value objVal) {
    if (objVal.type == ValueType::OBJECT) {

        //objVal.objectValue->turboVM = this;

        for(auto& prop : objVal.objectValue->get_all_properties()) {
            if (prop.second.type == ValueType::CLOSURE) {
                prop.second.closureValue->js_object = objVal.objectValue;
                
            }
        }
    }
}

shared_ptr<JSObject> PeregrineVM::createJSObject(shared_ptr<JSClass> klass) {
    
    shared_ptr<JSObject> object = make_shared<JSObject>();
    //object->turboVM = this;
    object->setClass(klass);

    makeObjectInstance(Value::klass(klass), object);
    
    // create a jsobject from superclass and assign to parent_object
    if (klass->superClass != nullptr) {
        
        object->parent_object = klass->superClass->is_native ? klass->superClass->construct() : createJSObject(klass->superClass);
        object->parent_class = klass->superClass;
        
    }

    return object;

}

void PeregrineVM::CreateObjectLiteralProperty(const Value& obj_val, const string& prop_name, const Value& object) {
    if (obj_val.type == ValueType::CLOSURE) {
        
        shared_ptr<Closure> new_closure = make_shared<Closure>();
        new_closure->fn = obj_val.closureValue->fn;
        new_closure->upvalues = obj_val.closureValue->upvalues;
        new_closure->js_object = object.objectValue;
        new_closure->ctx = obj_val.closureValue->ctx;

        object.objectValue->set(prop_name, Value::closure(new_closure), VAR, { PUBLIC });

    } else {
                    
        object.objectValue->set(prop_name, obj_val, VAR, { PUBLIC });

    }

}

void PeregrineVM::makeObjectInstance(Value klass, shared_ptr<JSObject> obj) {
    
    for (auto& protoProp : klass.classValue->var_proto_props) {
                
        if (protoProp.second.value.type == ValueType::CLOSURE) {
            
            shared_ptr<Closure> new_closure = make_shared<Closure>();
            new_closure->fn = protoProp.second.value.closureValue->fn;
            new_closure->upvalues = protoProp.second.value.closureValue->upvalues;
            new_closure->js_object = obj;
            new_closure->ctx = protoProp.second.value.closureValue->ctx;

            obj->set(protoProp.first, Value::closure(new_closure), VAR, protoProp.second.modifiers);

        } else {
            
            // evaluate fields
            int field_reg = protoProp.second.value.numberValue;
            Value fnValue = module_->constants[field_reg];
            Value val = callFunction(fnValue, {});
                        
            obj->set(protoProp.first, val, VAR, protoProp.second.modifiers);

        }

    }

    for (auto& constProtoProp : klass.classValue->const_proto_props) {
                
        if (constProtoProp.second.value.type == ValueType::CLOSURE) {
            
            shared_ptr<Closure> new_closure = make_shared<Closure>();
            new_closure->fn = constProtoProp.second.value.closureValue->fn;
            new_closure->upvalues = constProtoProp.second.value.closureValue->upvalues;
            new_closure->js_object = obj;
            new_closure->ctx = constProtoProp.second.value.closureValue->ctx;

            obj->set(constProtoProp.first, Value::closure(new_closure), CONST, {});

        } else {
            
            // evaluate fields
            int field_reg = constProtoProp.second.value.numberValue;
            Value fnValue = module_->constants[field_reg];
            Value val = callFunction(fnValue, {});

            obj->set(constProtoProp.first, constProtoProp.second.value, CONST, constProtoProp.second.modifiers);

        }

    }

}

void PeregrineVM::invokeMethod(const Value& obj_value, const string& name, const vector<Value>& args) {
    
    if (obj_value.type == ValueType::OBJECT) {
        Value constructor = obj_value.objectValue->get(name);
        // TODO: check whether to throw error
        if (constructor.type != ValueType::UNDEFINED) {
            callMethod(constructor, args, obj_value);
        }
    }
    
}

void PeregrineVM::InvokeConstructor(const Value& obj_value, const vector<Value>& args) {
    
    invokeMethod(obj_value, CONSTRUCTOR, args);
    
    // call super class constructor
    
    if (obj_value.objectValue->parent_object != nullptr) {
        InvokeConstructor(Value::object(obj_value.objectValue->parent_object), args);
    }
}

Value PeregrineVM::addCtor() {

    shared_ptr<TurboChunk> fnChunk = make_shared<TurboChunk>();
    fnChunk->arity = 0;

    fnChunk->code.push_back({TurboOpCode::Nop, 0,0,0});
    fnChunk->code.push_back({TurboOpCode::SuperCall, 0,0,0});

    int constant_index = fnChunk->addConstant(Value::undefined());
    
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
    shared_ptr<ExecutionContext> ctx = make_shared<ExecutionContext>();
    new_closure->ctx = ctx;

    return Value::closure(new_closure);

}

Value PeregrineVM::run(shared_ptr<TurboChunk> chunk_, const vector<Value>& args) {
    
    auto closure = make_shared<Closure>();

    // prepare a top-level frame that will be executed by runFrame()
    CallFrame new_frame;
    new_frame.chunk = chunk_;
    new_frame.ip = 0;
    new_frame.args = args;
    new_frame.closure = closure;
        
    callStack.push_back(std::move(new_frame));
    Value result = runFrame(callStack.back());
    callStack.pop_back();
    return result;
    
}

Value PeregrineVM::runFrame(CallFrame &current_frame) {
    
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
                                
            case TurboOpCode::CreateGlobalVar:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                executionCtx->variableEnv->set_var(name,
                                                         frame->registers[data_reg]);
                
                break;
            }
                
            case TurboOpCode::CreateGlobalLet:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                executionCtx
                    ->lexicalEnv->set_let(name, frame->registers[data_reg]);
                
                break;
            }
                
            case TurboOpCode::CreateGlobalConst:{
                uint8_t constant_index = instruction.a;
                uint8_t data_reg = instruction.b;
                
                Value name_val = frame->chunk->constants[constant_index];
                string name = name_val.stringValue;
                
                executionCtx->lexicalEnv->set_const(name,
                                               frame->registers[data_reg]);
                
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
                
                // TurboOpCode::TypeOf, reg
            case TurboOpCode::TypeOf: {
                Value value = frame->registers[instruction.a];
                frame->registers[instruction.a] = Value::str(type_of(value));
                break;
            }
                
                // TurboOpCode::Delete, reg, objReg, propertyReg
            case TurboOpCode::Delete: {
                
                Value obj = frame->registers[instruction.b];
                Value property = frame->registers[instruction.c];

                frame->registers[instruction.a] = Value::boolean(delete_op(obj, property));
                
                break;
            }

                // checks if an object is an instance of a specific class or constructor function,
                // or if its prototype chain includes the prototype of the specified constructor.
                // obj, class
            case TurboOpCode::InstanceOf: {

                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] =  Value::boolean(instance_of(a,b));

                break;
            }
                
            case TurboOpCode::In: {
                Value a = frame->registers[instruction.b];
                Value b = frame->registers[instruction.c];
                frame->registers[instruction.a] =  Value::boolean(in(a,b));
                break;
            }
                
            case TurboOpCode::Void: {
                frame->registers[instruction.a] = Value::undefined();
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
                
            case TurboOpCode::LoadGlobalVar: {
                
                int reg = instruction.a;
                int idx = instruction.b;
                string name = frame->chunk->constants[idx].stringValue;
                
                frame->registers[reg] = getVariable(name); //toValue(executionCtx->variableEnv->get(name)/*env->get(name)*/);

                break;
            }

            case TurboOpCode::StoreGlobalVar: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                Value val = frame->chunk->constants[idx];
                string name = val.stringValue;
                
                // env->set_var(name, frame->registers[reg_slot]);
                putVariable(name, frame->registers[reg_slot]);

                break;
            }
                
            case TurboOpCode::StoreGlobalLet: {
                
                uint8_t idx = instruction.a;
                uint8_t reg_slot = instruction.b;
                Value val = frame->chunk->constants[idx];
                string name = val.stringValue;

                // env->set_let(name, frame->registers[reg_slot]);
                putVariable(name, frame->registers[reg_slot]);

                break;
            }
                
                // emit(TurboOpCode::CreateArrayLiteral, arr);
            case TurboOpCode::CreateArrayLiteral: {
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
                
                // TurboOpCode::ArraySpread, arr, val
            case TurboOpCode::ArraySpread: {
                
                Value spreadArray = frame->registers[instruction.b];
                Value array = frame->registers[instruction.a];
                
                for( auto index : spreadArray.arrayValue->get_indexed_properties()) {
                    array.arrayValue->push({ index.second });
                }
                
                frame->registers[instruction.a] = array;

                break;
            }
                
            case TurboOpCode::ObjectSpread: {
                
                Value spreadObj = frame->registers[instruction.b];
                Value obj = frame->registers[instruction.a];
                
                for (auto index : spreadObj.objectValue->get_all_properties()) {
                    setProperty(obj, index.first, index.second);
                }
                
                frame->registers[instruction.a] = obj;
                
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
                klass.classValue->set_proto_vm_var(fieldNameValue.toString(), init, { "public" } );

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
                
                Value obj_value = CreateInstance(klass);

                frame->registers[instruction.a] = obj_value;

                break;
            }
                
                // TurboOpCode::InvokeConstructor, reg, argRegs[0], (int)argRegs.size());
            case TurboOpCode::InvokeConstructor: {
                
                const vector<Value> const_args = { argStack.begin(), argStack.end() };
                argStack.clear();

                Value obj_value = frame->registers[instruction.a];

                // call the constructor
                InvokeConstructor(obj_value, const_args);
                
                frame->registers[instruction.a] = obj_value;

                break;
            }
                
                // emit(TurboOpCode::CreateObjectLiteral, obj);                
            case TurboOpCode::CreateObjectLiteral: {
                auto object = make_shared<JSObject>();
                Value v = Value::object(object);
                set_js_object_closure(v);

                frame->registers[instruction.a] = v;

                break;
            }
             
                // CreateObjectLiteralProperty, obj, index, val
            case TurboOpCode::CreateObjectLiteralProperty: {
                
                auto object = frame->registers[instruction.a];
                Value val = frame->chunk->constants[instruction.b];
                string prop_name = val.toString();
                Value obj_val = frame->registers[instruction.c];

                CreateObjectLiteralProperty(obj_val, prop_name, object);

                frame->registers[instruction.a] = object;

                break;
            }
                
            case TurboOpCode::GetThis: {
                frame->registers[instruction.a] = Value::object(frame->closure->js_object);
                break;
            }
                
                // TurboOpCode::LoadThisProperty, reg_slot, nameIdx
            case TurboOpCode::LoadThisProperty: {
                
                // load constant from nameIdx
                Value property_value = frame->chunk->constants[instruction.b];
                string property_name = property_value.toString();
                                
                Value obj = getProperty(Value::object(frame->closure->js_object), property_name);
                
                frame->registers[instruction.a] = obj;

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
                frame->registers[instruction.a] = Value::object(frame->closure->js_object->parent_object);
                break;
            }

                // emit(TurboOpCode::SetProperty, obj, emitConstant(prop.first.lexeme), val);
                // SetProperty: objReg, nameIdx, valueReg
            case TurboOpCode::SetProperty: {
                auto object = frame->registers[instruction.a];
                Value val = frame->chunk->constants[instruction.b];
                string prop_name = val.toString();//stringValue;
                Value obj_val = frame->registers[instruction.c];
                
                setProperty(object, prop_name, obj_val);
                frame->registers[instruction.c] = object;

                break;
            }
                
                // SetPropertyDynamic: objReg, propReg, valueReg
                // emit(TurboOpCode::SetPropertyDynamic, objReg, propReg, resultReg);
            case TurboOpCode::SetPropertyDynamic: {
                auto object = frame->registers[instruction.a];
                string prop_name = frame->registers[instruction.b].toString();
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
                
                // TurboOpCode::CreateEnum, enumNameReg
            case TurboOpCode::CreateEnum: {
                
                auto enum_value_obj = createJSObject(make_shared<JSClass>());
                frame->registers[instruction.a] = Value::object(enum_value_obj);
                
                break;
            }
                
                // TurboOpCode::SetEnumProperty, enumNameReg, memberNameReg, valueReg
            case TurboOpCode::SetEnumProperty: {
                
                Value enum_obj = frame->registers[instruction.a];
                Value prop_value = frame->registers[instruction.b];
                Value value = frame->registers[instruction.c];
                
                setProperty(enum_obj, prop_value.toString(), value);
                
                frame->registers[instruction.a] = enum_obj;
                
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
                // ipAfterTry can be filled later by codegen if you want; keep -1 if unused
                f.ipAfterTry = -1;
                f.regCatch = instruction.c;
                tryStack.push_back(f);
                break;
            }

            case TurboOpCode::EndTry: {
                if (tryStack.empty()) {
                    // runtime error: unmatched EndTry
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
                                        
                    // If there is a finally, run it first.
                    if (f.catchIP != -1) {
                        
                        // Record a special marker frame to indicate we are resuming a throw after finally
                        // We'll push a synthetic TryFrame with catchIP = original catchIP, finallyIP = -1
                        TryFrame resume;
                        resume.catchIP = -1;
                        resume.finallyIP = f.finallyIP;
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
                    printf("Uncaught exception, halting VM\n");
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
                            // emulate throwing again: pop pending exception and re-run Throw logic
                            // Value pending = pop();
                            // continue throw loop by re-inserting pending on stack and handling next frame
                            // simplest approach: directly re-run THROW handling by pushing pending and continuing
                            // For clarity, we will set a special behaviour: re-insert pending and emulate OP_THROW
                            // stack.push_back(pending);
                            // simulate Throw logic by jumping back one step: decrement ip so we re-execute OP_THROW
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
                Value v = frame->chunk->constants[exception_value_index];
                
                executionCtx->lexicalEnv->set_let(v.toString(), throw_value);
                
                break;
            }

            case TurboOpCode::PushArg: {
                int argReg = instruction.a;
                argStack.push_back(frame->registers[argReg]);
                break;
            }
                
            case TurboOpCode::PushSpreadArg: {
                int argReg = instruction.a;
                Value array = frame->registers[argReg];
                
                for (auto index : array.arrayValue->get_indexed_properties()) {
                    argStack.push_back(index.second);
                }
                
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
                // loop thorugh frame->args and don't count null and undefined.
                
                int size = 0;
                
                for (auto arg : frame->args) {
                    if (arg.type == ValueType::UNDEFINED) {
                        continue;
                    }
                    size++;
                }
                
                frame->registers[instruction.a] = Value((double)size/*frame->args.size()*/);
                
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
                
                // TurboOpCode::SuperCall, resultReg, funcReg, static_cast<int>(argRegs.size())
            case TurboOpCode::SuperCall: {
                
                const vector<Value> const_args = { argStack.begin(), argStack.end() };
                argStack.clear();

                Value obj_value = frame->registers[instruction.b];

                // call the constructor
                invokeMethod(obj_value, CONSTRUCTOR, const_args);

                frame->registers[instruction.a] = obj_value;

                break;
            }

            case TurboOpCode::Return: {

                Value v = frame->registers[instruction.a];
                                
                return v;
            }

            case TurboOpCode::Halt:
                return Value::undefined();
                break;
                
            case TurboOpCode::CopyIterationBinding: {
                Value name_value = frame->chunk->constants[instruction.a];
                R prev = executionCtx->lexicalEnv->getParentValue(name_value.toString());
                executionCtx->lexicalEnv->set_let(name_value.toString(), toValue(prev));
                break;
            }
                
            case TurboOpCode::PushLexicalEnv: {
                
                ExecutionContext* ctx = new ExecutionContext();
                ctx->lexicalEnv = make_shared<Env>();
                ctx->lexicalEnv->setParentEnv(executionCtx->lexicalEnv);
                ctx->variableEnv = executionCtx->variableEnv;
                                
                contextStack.push_back(ctx);
                
                executionCtx = contextStack.back();
                
                break;
            }
                
            case TurboOpCode::PopLexicalEnv: {
                
                contextStack.pop_back();
                executionCtx = contextStack.back();
                break;
            }
            case TurboOpCode::SetExecutionContext: {
                Value closure = frame->registers[instruction.a];
                closure.closureValue->ctx = make_shared<ExecutionContext>(*executionCtx);
                break;
            }
                
                // CreatePromise, reg, promise_reg
            case TurboOpCode::CreatePromise: {
                
                auto promise = make_shared<Promise>(this);
                promise->resolve(frame->registers[instruction.a]);
                //promise->then_callbacks.push_back([]()->Value {});
                frame->registers[instruction.b] = Value::promise(promise);
                
                break;
                
            }
                
            case TurboOpCode::Await: {
                //                auto& promise = registers[instr.a];
                //                auto* currentTask = this->currentCoroutine();
                //
                //                // Suspend coroutine
                //                currentTask->suspend();
                //
                //                // Register resume callback when promise resolves
                //                promise->then(
                //                    [currentTask](Value result) {
                //                        currentTask->resumeWith(result);
                //                    },
                //                    [currentTask](Value error) {
                //                        currentTask->resumeWithException(error);
                //                    });
                
                Value result = frame->registers[instruction.a];
                auto index = frame->closure->fn->chunkIndex;
                auto closure = frame->closure;
                auto this_frame = frame;
                auto i = instruction;
                auto executionContext = executionCtx;

                auto value = result.promiseValue->then([this, this_frame, index, closure, i, executionContext](vector<Value> args)->Value {
                    
                    shared_ptr<TurboChunk> calleeChunk = module_->chunks[index];

                    callStack.push_back(*this_frame);
                    callStack.back().registers[i.b] = args[0];
                    Value result = runFrameContext(callStack.back(), executionContext);
                    
                    return result;
                    
                });

                return Value();

            }
                
            default:
                throw std::runtime_error("Unknown opcode in VM");
        }
    }
    
    return Value::undefined();
    
}

Value PeregrineVM::callMethod(const Value& callee, const vector<Value>& args, const Value& js_object) {

    return callFunction(callee, args);
    
}

ExecutionContext* PeregrineVM::createNewExecutionContext(const Value& callee) const  {
    
    ExecutionContext* funcCtx = new ExecutionContext();
    funcCtx->lexicalEnv = make_shared<Env>();
    funcCtx->lexicalEnv->setParentEnv(callee.closureValue->ctx->lexicalEnv);
    
    funcCtx->variableEnv = make_shared<Env>();
    funcCtx->variableEnv->setParentEnv(callee.closureValue->ctx->variableEnv);
    
    return funcCtx;

}

Value PeregrineVM::runFrameContext(CallFrame& frame, ExecutionContext* ctx) {
    
    contextStack.push_back(ctx);
    executionCtx = ctx;
    
    return runFrame(frame);
    
}

Value PeregrineVM::callFunction(const Value& callee, const vector<Value>& args) {
    
    if (callee.type == ValueType::FUNCTION) {
        Value result = callee.functionValue(args);
        return result;
    }
    
    if (callee.type == ValueType::CLOSURE) {

        shared_ptr<TurboChunk> calleeChunk = module_->chunks[callee.closureValue->fn->chunkIndex];
        
        // Build new frame
        CallFrame new_frame;
        new_frame.chunk = calleeChunk;
        new_frame.ip = 0;
        new_frame.args = args;
        new_frame.closure = callee.closureValue;

        callStack.push_back(std::move(new_frame));

        ExecutionContext* funcCtx = createNewExecutionContext(callee);
        
        contextStack.push_back(funcCtx);
        executionCtx = contextStack.back();

        Value result = runFrame(callStack.back());
        
        callStack.pop_back();
        if (callStack.size() > 0) {
            frame = &callStack.back();
        }
        
        contextStack.pop_back();
        executionCtx = contextStack.back();
        
        if (callee.closureValue->fn->isAsync) {
            auto promise = make_shared<Promise>(this);
            promise->resolve(result);
            return Value::promise(promise);

        }
        
        return result;
        
    }

    if (callee.type == ValueType::NATIVE_FUNCTION) {
        Value result = callee.nativeFunction(args);
        return result;
    }
    
    if (callee.type == ValueType::CLASS && callee.classValue->is_native) {
        // handle native call
        // Array(), Boolean(), String(), etc
        return callee.classValue->call(args);
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
    new_frame.args = args;
    
    new_frame.closure = callee.closureValue; // may be nullptr if callee is plain functionRef

    // save current frame
    CallFrame prev_frame = callStack.back();
        
    // push frame and execute it
    callStack.push_back(std::move(new_frame));

    Value result = runFrame(callStack.back());
    
    callStack.pop_back();
    frame = &prev_frame;

    return result;
    
}

void PeregrineVM::handleRethrow() {
    // simplified: pop pending exception and re-run OP_THROW-like unwinding
    // Re-run throw loop: same as OP_THROW handling but without recursion here.
    bool handled = false;
    while (!tryStack.empty()) {
        TryFrame f = tryStack.back();
        tryStack.pop_back();
        
        if (f.catchIP != -1) {
                        
            TryFrame resume;
            resume.catchIP = -1;
            resume.finallyIP = f.finallyIP;
            tryStack.push_back(resume);
            
            frame->ip = f.catchIP;
            
            handled = true;
            break;
        }
        
        if (f.finallyIP != -1) {
            frame->ip = f.finallyIP;
            handled = true;
            break;
        }
        
    }
    if (!handled) {
        printf("Uncaught exception after finally, halting VM\n");
    }
}
