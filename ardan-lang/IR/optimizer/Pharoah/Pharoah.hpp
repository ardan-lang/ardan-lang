//
//  Pharoah.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 20/07/2026.
//

#ifndef Pharoah_hpp
#define Pharoah_hpp

#include <stdio.h>
#include <vector>

#include "ir/IRFunction/IRFunction.hpp"
#include "ir/IRModule/IRModule.hpp"

using namespace std;

class OptimizationPass {
public:
    virtual void run(std::vector<IRInstruction>& instructions) = 0;
};

class ConstantFold : public OptimizationPass {
    void run(std::vector<IRInstruction>& instructions) override;
};

/**
 *
 */
class Pharoah {
    
    vector<OptimizationPass*> passes = {
        new ConstantFold()
    };
    
public:
    void start(IRModule& irModule);
};

#endif /* Pharoah_hpp */
