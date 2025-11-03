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
#include "../ExecutionContext/ExecutionContext.hpp"

class EnvironmentManager {
public:
    EnvironmentManager();
    
    ExecutionContext* current() const;
    void pushLexicalEnv(std::shared_ptr<Env> parentLexical, std::shared_ptr<Env> variableEnv);
    void popLexicalEnv();

private:
    std::vector<ExecutionContext*> contextStack;
};

#endif /* EnvironmentManager_hpp */
