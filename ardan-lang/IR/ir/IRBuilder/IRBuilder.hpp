//
//  IRBuilder.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#ifndef IRBuilder_hpp
#define IRBuilder_hpp

#include <memory>
#include <vector>
#include <stack>
#include "../TurboChunk.hpp"
#include "IRNode.hpp"

class IRBuilder {
public:
    IRGraph graph;
    
    IRBuilder();
    
    IRNodePtr buildFromChunk(std::shared_ptr<TurboChunk> chunk);
    
private:
    std::shared_ptr<TurboChunk> currentChunk;
    std::vector<IRNodePtr> registerNodes;
    std::stack<IRNodePtr> controlStack;
    std::stack<IRNodePtr> effectStack;
    
    IRNodePtr getRegisterNode(uint8_t reg);
    void setRegisterNode(uint8_t reg, IRNodePtr node);
    
    IRNodePtr processInstruction(const Instruction& inst, size_t& ip);
    IRNodePtr buildBinaryOp(TurboOpCode op, IRNodePtr left, IRNodePtr right);
    IRNodePtr buildUnaryOp(TurboOpCode op, IRNodePtr operand);
    IRNodePtr buildLoadConst(uint32_t index);
    IRNodePtr buildLoad(uint8_t reg);
    IRNodePtr buildStore(uint8_t reg, IRNodePtr value);
    IRNodePtr buildCall(IRNodePtr callee, const std::vector<IRNodePtr>& args);
    IRNodePtr buildReturn(IRNodePtr value);
    IRNodePtr buildJump(size_t target);
    IRNodePtr buildBranch(IRNodePtr condition, size_t trueTarget, size_t falseTarget);
    
    IRValueType opcodeToValueType(TurboOpCode op);
};

#endif /* IRBuilder_hpp */
