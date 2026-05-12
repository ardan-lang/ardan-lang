//
//  IRNode.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#ifndef IRNode_hpp
#define IRNode_hpp

#include <stdio.h>
#include <vector>
#include <memory>
#include <unordered_set>
#include <string>
#include <iostream>
#include <sstream>

enum class IRNodeType {

    Start,
    End,
    Region,
    If,
    Loop,
    Return,
    
    Constant,
    Parameter,
    Phi,
    
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Neg,
    Not,
    
    Equal,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
    
    Load,
    Store,
    LoadProperty,
    StoreProperty,
    
    Call,
    CallBuiltin,
    
    NewObject,
    NewArray,
    
    EffectPhi,
    StateValues,
    
    Projection
};

enum class IRValueType {
    Int32,
    Float64,
    Boolean,
    String,
    Object,
    Array,
    Undefined,
    Null,
    Any
};

class IRNode;
class IREdge;

using IRNodePtr = std::shared_ptr<IRNode>;
using IREdgePtr = std::shared_ptr<IREdge>;

class IREdge {
public:
    IRNodePtr from;
    IRNodePtr to;
    int index; // Input index on 'to' node
    
    IREdge(IRNodePtr f, IRNodePtr t, int idx) : from(f), to(t), index(idx) {}
};

class IRNode : public std::enable_shared_from_this<IRNode> {
public:
    int id;
    IRNodeType type;
    IRValueType valueType;
    std::vector<IREdgePtr> inputs; // These are DATAFLOW edges INTO this node.
    std::vector<IREdgePtr> outputs; // Tracks nodes USING this node.
    
    std::vector<IRNodePtr> controlInputs; // Tracks execution order.
    std::vector<IRNodePtr> controlOutputs; // Reverse control edges.
    std::vector<IRNodePtr> effectInputs; // Tracks SIDE EFFECT dependencies.
    std::vector<IRNodePtr> effectOutputs; // Reverse effect dependencies.
    
    union {
        int32_t intValue;
        double floatValue;
        bool boolValue;
        const char* stringValue;
    } data;
    
    std::string debugName;
    
    IRNode(int id, IRNodeType t, IRValueType vt = IRValueType::Any)
        : id(id), type(t), valueType(vt) {}
    
    void addInput(IRNodePtr input, int index = -1);
    void addControlInput(IRNodePtr control);
    void addEffectInput(IRNodePtr effect);
    
    void replaceInput(int index, IRNodePtr newInput);
    void removeInput(int index);
    
    bool isDead() const { return outputs.empty() && controlOutputs.empty() && effectOutputs.empty(); }
    
    virtual std::string toString() const;
};

#endif /* IRNode_hpp */
