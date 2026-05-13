//
//  IRGenerator.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#ifndef IRGenerator_hpp
#define IRGenerator_hpp

#include <stdio.h>
#include <memory>
#include <vector>
#include <unordered_map>

#include "RegisterContext/ir/IRNode/IRNode.hpp"
#include "RegisterContext/ir/IRGraph/IRGraph.hpp"
#include "RegisterClosure/TurboChunk.hpp"

//class IRGenerator {
//public:
//
//    IRBuilder builder;
//
//    IRFunction generate(
//        std::shared_ptr<Program> program
//    ) {
//
//        for (auto& stmt : program->statements) {
//            generateStmt(stmt);
//        }
//
//        return builder.function;
//    }
//};
//

class IRCodeGenerator {
public:
    std::shared_ptr<TurboChunk> generate(IRGraph& graph);
    
private:
    std::shared_ptr<TurboChunk> chunk;
    std::unordered_map<IRNodePtr, uint8_t> nodeToRegister;
    uint8_t nextRegister = 0;
    
    uint8_t allocateRegister(IRNodePtr node);
    uint8_t getRegister(IRNodePtr node);
    
    void generateNode(IRNodePtr node);
    void generateBinaryOp(IRNodePtr node);
    void generateUnaryOp(IRNodePtr node);
    void generateConstant(IRNodePtr node);
    void generateReturn(IRNodePtr node);
    void generateCall(IRNodePtr node);
    void generateLoad(IRNodePtr node);
    void generateStore(IRNodePtr node);
    
    TurboOpCode nodeTypeToOpcode(IRNodeType type);
};

#endif /* IRGenerator_hpp */
