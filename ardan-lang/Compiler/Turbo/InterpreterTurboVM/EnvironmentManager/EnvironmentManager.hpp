//
//  EnvironmentManager.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#ifndef EnvironmentManager_hpp
#define EnvironmentManager_hpp

#include <stdio.h>

#pragma once
#include <memory>
#include <vector>
#include "../../../../Interpreter/Env.h"
#include "../../../../Interpreter/Utils/Utils.h"
#include "../ExecutionContext/ExecutionContext.hpp"

class EnvironmentManager {
public:
    EnvironmentManager() = default;
    ~EnvironmentManager();
    
    ExecutionContext* current() const;
//    void pushLexicalEnv(std::shared_ptr<Env> parentLexical, std::shared_ptr<Env> variableEnv);
    void pushLexicalEnv();
    void pushContext(unique_ptr<ExecutionContext> ctx);
    void popLexicalEnv();
    
    void initRoot(std::shared_ptr<Env> rootEnv);
    Value getVariable(const std::string& key) const;
    void putVariable(const std::string& key, const Value& v) const;
    
private:
    std::vector<unique_ptr<ExecutionContext>> contextStack;
};

#endif /* EnvironmentManager_hpp */
