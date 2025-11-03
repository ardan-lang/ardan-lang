//
//  EnvironmentManager.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#include "EnvironmentManager.hpp"

EnvironmentManager::EnvironmentManager() {}

ExecutionContext* EnvironmentManager::current() const {
    return contextStack.empty() ? nullptr : contextStack.back();
}

void EnvironmentManager::pushLexicalEnv(std::shared_ptr<Env> parentLexical, std::shared_ptr<Env> variableEnv) {
    auto* ctx = new ExecutionContext();
    ctx->lexicalEnv = std::make_shared<Env>();
    ctx->lexicalEnv->setParentEnv(parentLexical);
    ctx->variableEnv = variableEnv;
    contextStack.push_back(ctx);
}

void EnvironmentManager::popLexicalEnv() {
    if (!contextStack.empty()) {
        contextStack.pop_back();
    }
}
