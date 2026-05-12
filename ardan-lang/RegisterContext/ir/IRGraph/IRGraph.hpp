//
//  IRGraph.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#ifndef IRGraph_hpp
#define IRGraph_hpp

#include <stdio.h>
#include "RegisterContext/ir/IRNode/IRNode.hpp"

class IRGraph {
public:
    std::vector<IRNodePtr> nodes;
    IRNodePtr start;
    IRNodePtr end;
    
    int nextId = 0;
    
    IRNodePtr newNode(IRNodeType type, IRValueType vt = IRValueType::Any);
    IRNodePtr newConstant(Value value);
    IRNodePtr newParameter(int index);
    
    void addEdge(IRNodePtr from, IRNodePtr to, int inputIndex = -1);
    void addControlEdge(IRNodePtr from, IRNodePtr to);
    void addEffectEdge(IRNodePtr from, IRNodePtr to);
    
    void removeNode(IRNodePtr node);
    void replaceNode(IRNodePtr oldNode, IRNodePtr newNode);
    
    std::vector<IRNodePtr> getReversePostOrder();
    std::vector<IRNodePtr> getDominators();
};

#endif /* IRGraph_hpp */
