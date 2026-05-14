////
////  TurbofanOptimizer.hpp
////  ardan-lang
////
////  Created by Chidume Nnamdi on 10/05/2026.
////
//
//#ifndef TurbofanOptimizer_hpp
//#define TurbofanOptimizer_hpp
//
//#include <stdio.h>
//
//class TurbofanOptimizer {
//public:
//    TurbofanOptimizer();
//    
//    IRGraph optimize(IRGraph& inputGraph);
//    
//    void addPass(std::unique_ptr<OptimizationPass> pass, OptimizationPhase phase);
//    void runPhase(OptimizationPhase phase, IRGraph& graph);
//    
//private:
//    std::vector<std::unique_ptr<OptimizationPass>> earlyPasses;
//    std::vector<std::unique_ptr<OptimizationPass>> loopPasses;
//    std::vector<std::unique_ptr<OptimizationPass>> latePasses;
//    std::vector<std::unique_ptr<OptimizationPass>> lastPasses;
//    
//    bool runPasses(std::vector<std::unique_ptr<OptimizationPass>>& passes, IRGraph& graph);
//};
//
//#endif /* TurbofanOptimizer_hpp */
