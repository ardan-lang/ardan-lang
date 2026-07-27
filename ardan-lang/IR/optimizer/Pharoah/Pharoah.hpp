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
#include "Interpreter/Utils/Utils.h"

//Constant Folding
//Constant Propagation
//Copy Propagation
//Dead Code Elimination
//Common Subexpression Elimination (or Global Value Numbering)
//Type Propagation
//Loop Optimizations (LICM, strength reduction, induction variables)
//Function Inlining
//Peephole Optimization
//Register Allocation

using namespace std;

class OptimizationPass {
public:
    virtual void run(std::vector<IRInstruction>& instructions) = 0;
};

class ConstantFold : public OptimizationPass {
    void run(std::vector<IRInstruction>& instructions) override;
};

class DeadCode : public OptimizationPass {
    void run(std::vector<IRInstruction>& instructions) override;
};

/**
 * Pharoah is the optmizer for Ardan
 */
class Pharoah {
    
    vector<OptimizationPass*> passes = {
        // new ConstantFold(),
        new DeadCode(),
    };
    
public:
    void start(IRModule& irModule);
};

#endif /* Pharoah_hpp */
