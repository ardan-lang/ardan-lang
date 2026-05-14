////
////  Optimizer.hpp
////  ardan-lang
////
////  Created by Chidume Nnamdi on 10/05/2026.
////
//
//#ifndef Optimizer_hpp
//#define Optimizer_hpp
//
//#include <stdio.h>
//#include <memory>
//#include <string>
//#include "IRNode.hpp"
//
//class OptimizationPass {
//public:
//    virtual ~OptimizationPass() = default;
//    virtual std::string name() const = 0;
//    virtual bool run(IRGraph& graph) = 0;
//    
//protected:
//    bool changed = false;
//    
//    void markChanged() { changed = true; }
//    bool hasChanged() const { return changed; }
//    void resetChanged() { changed = false; }
//};
//
//class ConstantFoldingPass : public OptimizationPass {
//public:
//    std::string name() const override { return "ConstantFolding"; }
//    bool run(IRGraph& graph) override;
//};
//
//class DeadCodeEliminationPass : public OptimizationPass {
//public:
//    std::string name() const override { return "DeadCodeElimination"; }
//    bool run(IRGraph& graph) override;
//};
//
//class CommonSubexpressionEliminationPass : public OptimizationPass {
//public:
//    std::string name() const override { return "CommonSubexpressionElimination"; }
//    bool run(IRGraph& graph) override;
//};
//
//class InliningPass : public OptimizationPass {
//public:
//    std::string name() const override { return "Inlining"; }
//    bool run(IRGraph& graph) override;
//};
//
//class LoopOptimizationPass : public OptimizationPass {
//public:
//    std::string name() const override { return "LoopOptimization"; }
//    bool run(IRGraph& graph) override;
//};
//
//class TypePropagationPass : public OptimizationPass {
//public:
//    std::string name() const override { return "TypePropagation"; }
//    bool run(IRGraph& graph) override;
//};
//
//#endif /* OptimizationPass_hpp */
//
//#include "OptimizationPass.hpp"
//#include <unordered_map>
//#include <unordered_set>
//
//bool ConstantFoldingPass::run(IRGraph& graph) {
//    resetChanged();
//    
//    for (auto& node : graph.nodes) {
//        if (node->type == IRNodeType::Add &&
//            node->inputs.size() >= 2 &&
//            node->inputs[0]->from->type == IRNodeType::Constant &&
//            node->inputs[1]->from->type == IRNodeType::Constant) {
//            
//            double left = node->inputs[0]->from->data.floatValue;
//            double right = node->inputs[1]->from->data.floatValue;
//            double result = left + right;
//            
//            auto constNode = graph.newConstant(Value::number(result));
//            graph.replaceNode(node, constNode);
//            markChanged();
//        }
//    }
//    
//    return hasChanged();
//}
//
//bool DeadCodeEliminationPass::run(IRGraph& graph) {
//    resetChanged();
//    
//    std::unordered_set<IRNodePtr> liveNodes;
//    std::vector<IRNodePtr> worklist;
//    
//    worklist.push_back(graph.end);
//    liveNodes.insert(graph.end);
//    
//    while (!worklist.empty()) {
//        auto node = worklist.back();
//        worklist.pop_back();
//        
//        for (auto& edge : node->inputs) {
//            if (liveNodes.insert(edge->from).second) {
//                worklist.push_back(edge->from);
//            }
//        }
//        for (auto& control : node->controlInputs) {
//            if (liveNodes.insert(control).second) {
//                worklist.push_back(control);
//            }
//        }
//        for (auto& effect : node->effectInputs) {
//            if (liveNodes.insert(effect).second) {
//                worklist.push_back(effect);
//            }
//        }
//    }
//    
//    std::vector<IRNodePtr> deadNodes;
//    for (auto& node : graph.nodes) {
//        if (liveNodes.find(node) == liveNodes.end()) {
//            deadNodes.push_back(node);
//        }
//    }
//    
//    for (auto& node : deadNodes) {
//        graph.removeNode(node);
//        markChanged();
//    }
//    
//    return hasChanged();
//}
//
//bool CommonSubexpressionEliminationPass::run(IRGraph& graph) {
//    resetChanged();
//    
//    std::unordered_map<std::string, IRNodePtr> exprMap;
//    
//    for (auto& node : graph.getReversePostOrder()) {
//        if (node->type == IRNodeType::Add || node->type == IRNodeType::Sub ||
//            node->type == IRNodeType::Mul || node->type == IRNodeType::Div) {
//            
//            std::string sig = std::to_string(static_cast<int>(node->type)) + "_";
//            for (auto& input : node->inputs) {
//                sig += std::to_string(input->from->id) + "_";
//            }
//            
//            if (exprMap.count(sig)) {
//                
//                graph.replaceNode(node, exprMap[sig]);
//                markChanged();
//            } else {
//                exprMap[sig] = node;
//            }
//        }
//    }
//    
//    return hasChanged();
//}
//
//bool InliningPass::run(IRGraph& graph) {
//    resetChanged();
//    
//    for (auto& node : graph.nodes) {
//        if (node->type == IRNodeType::Call) {
//            markChanged();
//        }
//    }
//    
//    return hasChanged();
//}
//
//bool LoopOptimizationPass::run(IRGraph& graph) {
//    resetChanged();
//        
//    return hasChanged();
//}
//
//bool TypePropagationPass::run(IRGraph& graph) {
//    resetChanged();
//    
//    for (auto& node : graph.nodes) {
//        if (node->type == IRNodeType::Constant) {
//            markChanged();
//        }
//    }
//    
//    return hasChanged();
//}
//
//#ifndef TurbofanOptimizer_hpp
//#define TurbofanOptimizer_hpp
//
//#include <vector>
//#include <memory>
//#include <string>
//#include "IRNode.hpp"
//#include "OptimizationPass.hpp"
//
//enum class OptimizationPhase {
//    Early,
//    Loop,
//    Late,
//    Last
//};
//
//#endif /* Optimizer_hpp */
