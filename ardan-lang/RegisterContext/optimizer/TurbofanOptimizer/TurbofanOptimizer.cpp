//
//  TurbofanOptimizer.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#include "TurbofanOptimizer.hpp"
#include <iostream>

TurbofanOptimizer::TurbofanOptimizer() {

    addPass(std::make_unique<ConstantFoldingPass>(), OptimizationPhase::Early);
    addPass(std::make_unique<DeadCodeEliminationPass>(), OptimizationPhase::Early);
    addPass(std::make_unique<TypePropagationPass>(), OptimizationPhase::Early);
    
    addPass(std::make_unique<CommonSubexpressionEliminationPass>(), OptimizationPhase::Loop);
    addPass(std::make_unique<LoopOptimizationPass>(), OptimizationPhase::Loop);
    
    addPass(std::make_unique<InliningPass>(), OptimizationPhase::Late);
    addPass(std::make_unique<ConstantFoldingPass>(), OptimizationPhase::Late);
    
    addPass(std::make_unique<DeadCodeEliminationPass>(), OptimizationPhase::Last);
}

IRGraph TurbofanOptimizer::optimize(IRGraph& inputGraph) {
    IRGraph graph = inputGraph;
    
    std::cout << "Starting Turbofan optimization pipeline..." << std::endl;
    
    runPhase(OptimizationPhase::Early, graph);
    runPhase(OptimizationPhase::Loop, graph);
    runPhase(OptimizationPhase::Late, graph);
    runPhase(OptimizationPhase::Last, graph);
    
    std::cout << "Optimization complete." << std::endl;
    return graph;
}

void TurbofanOptimizer::addPass(std::unique_ptr<OptimizationPass> pass, OptimizationPhase phase) {
    switch (phase) {
        case OptimizationPhase::Early:
            earlyPasses.push_back(std::move(pass));
            break;
        case OptimizationPhase::Loop:
            loopPasses.push_back(std::move(pass));
            break;
        case OptimizationPhase::Late:
            latePasses.push_back(std::move(pass));
            break;
        case OptimizationPhase::Last:
            lastPasses.push_back(std::move(pass));
            break;
    }
}

void TurbofanOptimizer::runPhase(OptimizationPhase phase, IRGraph& graph) {
    std::vector<std::unique_ptr<OptimizationPass>>* passes = nullptr;
    std::string phaseName;
    
    switch (phase) {
        case OptimizationPhase::Early:
            passes = &earlyPasses;
            phaseName = "Early";
            break;
        case OptimizationPhase::Loop:
            passes = &loopPasses;
            phaseName = "Loop";
            break;
        case OptimizationPhase::Late:
            passes = &latePasses;
            phaseName = "Late";
            break;
        case OptimizationPhase::Last:
            passes = &lastPasses;
            phaseName = "Last";
            break;
    }
    
    if (!passes) return;
    
    std::cout << "Running " << phaseName << " optimization phase..." << std::endl;
    
    bool changed = true;
    int iterations = 0;
    const int maxIterations = 10;
    
    while (changed && iterations < maxIterations) {
        changed = runPasses(*passes, graph);
        iterations++;
    }
    
    std::cout << phaseName << " phase completed after " << iterations << " iterations." << std::endl;
}

bool TurbofanOptimizer::runPasses(std::vector<std::unique_ptr<OptimizationPass>>& passes, IRGraph& graph) {
    bool anyChanged = false;
    
    for (auto& pass : passes) {
        std::cout << "  Running " << pass->name() << "..." << std::endl;
        bool changed = pass->run(graph);
        if (changed) {
            std::cout << "    " << pass->name() << " made changes." << std::endl;
            anyChanged = true;
        }
    }
    
    return anyChanged;
}

#endif /* TurbofanOptimizer_hpp */
