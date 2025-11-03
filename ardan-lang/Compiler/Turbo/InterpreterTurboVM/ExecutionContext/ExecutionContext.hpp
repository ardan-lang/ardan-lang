//
//  ExecutionContext.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#ifndef ExecutionContext_hpp
#define ExecutionContext_hpp

#pragma once
#include <stdio.h>
#include <memory>
#include <vector>

#include "../../../../Interpreter/Env.h"

class ExecutionContext {
public:
    std::shared_ptr<Env> lexicalEnv;
    std::shared_ptr<Env> variableEnv;
};

#endif /* ExecutionContext_hpp */
