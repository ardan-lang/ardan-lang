//
//  FunctionInvoker.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#include "FunctionInvoker.hpp"

FunctionInvoker::FunctionInvoker(std::shared_ptr<TurboModule> m,
                                 CallStackManager* c,
                                 EnvironmentManager* e)
    : module(std::move(m)), callStack(c), envManager(e) {}

ExecutionContext* FunctionInvoker::createExecutionContext(const Value& callee) const {
    auto* ctx = new ExecutionContext();
    ctx->lexicalEnv = std::make_shared<Env>();
    ctx->lexicalEnv->setParentEnv(callee.closureValue->ctx->lexicalEnv);
    ctx->variableEnv = std::make_shared<Env>();
    ctx->variableEnv->setParentEnv(callee.closureValue->ctx->variableEnv);
    return ctx;
}

Value FunctionInvoker::call(const Value& callee, const std::vector<Value>& args) {
    if (callee.type == ValueType::FUNCTION)
        return callee.functionValue(args);

    if (callee.type == ValueType::NATIVE_FUNCTION)
        return callee.nativeFunction(args);

    if (callee.type == ValueType::CLASS && callee.classValue->is_native)
        return callee.classValue->call(args);

    if (callee.type != ValueType::CLOSURE)
        throw std::runtime_error("Attempt to call non-function");

    auto calleeChunk = module->chunks[callee.closureValue->fn->chunkIndex];
    CallFrame frame;
    frame.chunk = calleeChunk;
    frame.locals.resize(calleeChunk->maxLocals, Value::undefined());
    frame.args = args;
    frame.closure = callee.closureValue;

    callStack->pushFrame(std::move(frame));
    envManager->pushLexicalEnv(callee.closureValue->ctx->lexicalEnv,
                               callee.closureValue->ctx->variableEnv);

    // (In VM, this would call runFrame)
    Value result = Value::undefined();

    envManager->popLexicalEnv();
    callStack->popFrame();
    return result;
}
