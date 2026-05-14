//
//  IRModule.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 13/05/2026.
//

#ifndef IRModule_hpp
#define IRModule_hpp

#include <stdio.h>
#include <vector>
#include "IR/ir/IRFunction/IRFunction.hpp"

class IRModule {
    std::vector<std::unique_ptr<IRFunction>> functions;
};

#endif /* IRModule_hpp */
