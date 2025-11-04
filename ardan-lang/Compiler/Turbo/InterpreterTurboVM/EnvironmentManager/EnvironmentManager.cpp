//
//  EnvironmentManager.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#include "EnvironmentManager.hpp"

EnvironmentManager::~EnvironmentManager() = default;

ExecutionContext* EnvironmentManager::current() const {
    if (contextStack.empty()) return nullptr;
    return contextStack.back().get();
}

//void EnvironmentManager::pushLexicalEnv(std::shared_ptr<Env> parentLexical, std::shared_ptr<Env> variableEnv) {
//    auto* ctx = new ExecutionContext();
//    ctx->lexicalEnv = std::make_shared<Env>();
//    ctx->lexicalEnv->setParentEnv(parentLexical);
//    ctx->variableEnv = variableEnv;
//    contextStack.push_back(ctx);
//}

void EnvironmentManager::pushContext(unique_ptr<ExecutionContext> ctx) {
    contextStack.push_back(std::move(ctx));
}

void EnvironmentManager::pushLexicalEnv() {
    
    if (contextStack.empty()) throw runtime_error("EnvironmentManager: no root context");
    
    auto currentCtx = current();
    
    auto ctx = make_unique<ExecutionContext>();
    ctx->lexicalEnv = make_shared<Env>();
    ctx->lexicalEnv->setParentEnv(currentCtx->lexicalEnv);
    ctx->variableEnv = currentCtx->variableEnv;
    
    contextStack.push_back(std::move(ctx));
    
}

void EnvironmentManager::popLexicalEnv() {
    if (!contextStack.empty()) {
        contextStack.pop_back();
    }
}

void EnvironmentManager::initRoot(std::shared_ptr<Env> rootEnv) {
    contextStack.clear();
    auto ctx = std::make_unique<ExecutionContext>();
    // root lexical and variable envs both point to the same top-level Env wrapper
    ctx->lexicalEnv = std::make_shared<Env>(rootEnv.get()); // If Env has copy ctor or constructor from pointer
    ctx->variableEnv = std::make_shared<Env>(rootEnv.get());
    // NOTE: In your original code you passed raw Env*; adapt as needed to your Env constructors.
    contextStack.push_back(std::move(ctx));
}

Value EnvironmentManager::getVariable(const std::string& key) const {
    auto ctx = current();
    if (!ctx) throw std::runtime_error("No current execution context");
    R value = ctx->lexicalEnv->getValueWithoutThrow(key);
    if (std::holds_alternative<std::nullptr_t>(value)) {
        value = ctx->variableEnv->getValueWithoutThrow(key);
        if (std::holds_alternative<std::nullptr_t>(value))
            throw std::runtime_error("Undefined variable: " + key);
    }
    return toValue(value);
}

void EnvironmentManager::putVariable(const std::string& key, const Value& v) const {
    auto ctx = current();
    if (!ctx) throw std::runtime_error("No current execution context");
    Env* target = ctx->lexicalEnv->resolveBinding(key, ctx->lexicalEnv.get());
    if (target) {
        target->set_let(key, v);
        return;
    }
    target = ctx->variableEnv->resolveBinding(key, ctx->variableEnv.get());
    if (target) {
        target->set_var(key, v);
        return;
    }
    // if not found, you may consider creating in variableEnv (behavior depends on language semantics)
}
