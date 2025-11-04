//
//  FunctionInvoker.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#include "FunctionInvoker.hpp"
#include <stdexcept>

FunctionInvoker::FunctionInvoker(std::shared_ptr<TurboModule> module_,
                                 CallStackManager* callStack,
                                 EnvironmentManager* envMgr,
                                 Runner runner)
    : module_(std::move(module_)), callStack(callStack), envManager(envMgr), runner_(std::move(runner)) {}

unique_ptr<ExecutionContext> FunctionInvoker::createExecutionContextForClosure(const Value& callee) const {
    
    auto ctx = make_unique<ExecutionContext>();
    
    ctx->lexicalEnv = make_shared<Env>();
    ctx->lexicalEnv->setParentEnv(callee.closureValue->ctx->lexicalEnv);
    
    ctx->variableEnv = make_shared<Env>();
    ctx->variableEnv->setParentEnv(callee.closureValue->ctx->variableEnv);
    
    return ctx;
    
}

Value FunctionInvoker::call(const Value& callee, const std::vector<Value>& args) {
    if (callee.type == ValueType::FUNCTION) {
        return callee.functionValue(args);
    }

    if (callee.type == ValueType::NATIVE_FUNCTION) {
        return callee.nativeFunction(args);
    }

    if (callee.type == ValueType::CLASS && callee.classValue->is_native) {
        return callee.classValue->call(args);
    }

    if (callee.type != ValueType::CLOSURE && callee.type != ValueType::FUNCTION_REF) {
        throw std::runtime_error("Attempt to call non-function");
    }
    
        if (!module_) {
            throw runtime_error("Module not set in VM");
            return Value::undefined();
        }
    
    // Determine the function chunk & closure
    shared_ptr<TurboChunk> calleeChunk;
    shared_ptr<Closure> closurePtr;
    if (callee.type == ValueType::CLOSURE) {
        closurePtr = callee.closureValue;
        calleeChunk = module_->chunks[closurePtr->fn->chunkIndex];
    } else { // FUNCTION_REF
        
        auto fn = callee.fnRef;
        
        if (!module_) throw runtime_error("Module not set");
        
        if (fn->chunkIndex >= module_->chunks.size()) {
            throw runtime_error("Bad function index");
            return Value::undefined();
        }

        calleeChunk = module_->chunks[fn->chunkIndex];
        closurePtr = callee.closureValue; // may be nullptr in your model; adapt as necessary
    }

    TurboCallFrame newFrame;
    newFrame.chunk = calleeChunk;
    newFrame.ip = 0;
    newFrame.args = args;
    newFrame.closure = closurePtr;

    // push frame
    callStack->pushFrame(std::move(newFrame));

    // push execution context for closures
    bool pushedCtx = false;
    unique_ptr<ExecutionContext> createdCtx;
    if (closurePtr && closurePtr->ctx) {
        createdCtx = createExecutionContextForClosure(Value::closure(closurePtr));
        envManager->pushContext(std::move(createdCtx));
        pushedCtx = true;
    }

    // Run by calling back to VM's runFrame through runner_
    // The top frame on the callStack_ is the one we just pushed.
    TurboCallFrame* frameRef = callStack->top();
    Value result = runner_(frameRef);

    // cleanup
    if (pushedCtx) {
        envManager->popLexicalEnv();
    }

    callStack->popFrame();
    return result;
}

//FunctionInvoker::FunctionInvoker(std::shared_ptr<TurboModule> m,
//                                 CallStackManager* c,
//                                 EnvironmentManager* e)
//    : module_(std::move(m)), callStack(c), envManager(e) {}
//
//ExecutionContext* FunctionInvoker::createExecutionContext(const Value& callee) const {
//    auto* ctx = new ExecutionContext();
//    ctx->lexicalEnv = std::make_shared<Env>();
//    ctx->lexicalEnv->setParentEnv(callee.closureValue->ctx->lexicalEnv);
//    ctx->variableEnv = std::make_shared<Env>();
//    ctx->variableEnv->setParentEnv(callee.closureValue->ctx->variableEnv);
//    return ctx;
//}

//Value FunctionInvoker::call(const Value& callee, const std::vector<Value>& args) {
//    if (callee.type == ValueType::FUNCTION)
//        return callee.functionValue(args);
//
//    if (callee.type == ValueType::NATIVE_FUNCTION)
//        return callee.nativeFunction(args);
//
//    if (callee.type == ValueType::CLASS && callee.classValue->is_native)
//        return callee.classValue->call(args);
//
//    if (callee.type != ValueType::CLOSURE)
//        throw std::runtime_error("Attempt to call non-function");
//
//    auto calleeChunk = module->chunks[callee.closureValue->fn->chunkIndex];
//    CallFrame frame;
//    frame.chunk = calleeChunk;
//    frame.locals.resize(calleeChunk->maxLocals, Value::undefined());
//    frame.args = args;
//    frame.closure = callee.closureValue;
//
//    callStack->pushFrame(std::move(frame));
//    envManager->pushLexicalEnv(callee.closureValue->ctx->lexicalEnv,
//                               callee.closureValue->ctx->variableEnv);
//
//    // (In VM, this would call runFrame)
//    Value result = Value::undefined();
//
//    envManager->popLexicalEnv();
//    callStack->popFrame();
//    return result;
//}

//Value FunctionInvoker::call(const Value& callee, const vector<Value>& args) {
//    
//    if (callee.type == ValueType::FUNCTION) {
//        Value result = callee.functionValue(args);
//        return result;
//    }
//    
//    if (callee.type == ValueType::CLOSURE) {
//
//        shared_ptr<TurboChunk> calleeChunk = module_->chunks[callee.closureValue->fn->chunkIndex];
//        
//        // Build new frame
//        CallFrame* new_frame = new CallFrame;
//        new_frame->chunk = calleeChunk;
//        new_frame->ip = 0;
//        new_frame->args = args;
//        new_frame->closure = callee.closureValue;
//
//        callStack->pushFrame(new_frame);
//
//        ExecutionContext* funcCtx = createExecutionContext(callee);
//        
//        contextStack.push_back(funcCtx);
//        executionCtx = contextStack.back();
//
//        Value result = runFrame(callStack.back());
//        
//        callStack.pop_back();
//        frame = &callStack.back();
//        
//        contextStack.pop_back();
//        executionCtx = contextStack.back();
//        
//        return result;
//        
//    }
//
//    if (callee.type == ValueType::NATIVE_FUNCTION) {
//        Value result = callee.nativeFunction(args);
//        return result;
//    }
//    
//    if (callee.type == ValueType::CLASS && callee.classValue->is_native) {
//        // handle native call
//        // Array(), Boolean(), String(), etc
//        return callee.classValue->call(args);
//    }
//    
//    if (callee.type != ValueType::FUNCTION_REF) {
//        throw runtime_error("Attempt to call non-function");
//        return Value::undefined();
//    }
//    
//    auto fn = callee.fnRef;
//    
//    if (!module_) {
//        throw runtime_error("Module not set in VM");
//        return Value::undefined();
//    }
//    
//    if (fn->chunkIndex >= module_->chunks.size()) {
//        throw runtime_error("Bad function index");
//        return Value::undefined();
//    }
//    
//    shared_ptr<TurboChunk> calleeChunk = module_->chunks[fn->chunkIndex];
//
//    // Build new frame
//    CallFrame new_frame;
//    new_frame.chunk = calleeChunk;
//    new_frame.ip = 0;
//    new_frame.args = args;
//    
//    new_frame.closure = callee.closureValue; // may be nullptr if callee is plain functionRef
//
//    // save current frame
//    CallFrame prev_frame = callStack.back();
//        
//    // push frame and execute it
//    callStack.push_back(std::move(new_frame));
//
//    Value result = runFrame(callStack.back());
//    
//    callStack.pop_back();
//    frame = &prev_frame;
//
//    return result;
//    
//}
