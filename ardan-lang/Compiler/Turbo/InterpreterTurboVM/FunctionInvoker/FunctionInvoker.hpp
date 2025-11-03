//
//  FunctionInvoker.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#ifndef FunctionInvoker_hpp
#define FunctionInvoker_hpp

#pragma once
#include <stdio.h>
#include <vector>
#include <memory>
#include "../../../../Interpreter/ExecutionContext/Value/Value.h"
#include "../CallStackManager/CallStackManager.hpp"
#include "../EnvironmentManager/EnvironmentManager.hpp"
#include "../../TurboModule.hpp"

using namespace std;

using std::shared_ptr;

class FunctionInvoker {
public:
    FunctionInvoker(std::shared_ptr<TurboModule> module,
                    CallStackManager* callStack,
                    EnvironmentManager* envManager);

    Value call(const Value& callee, const std::vector<Value>& args);

private:
    std::shared_ptr<TurboModule> module;
    CallStackManager* callStack;
    EnvironmentManager* envManager;

    ExecutionContext* createExecutionContext(const Value& callee) const;
};

#endif /* FunctionInvoker_hpp */
