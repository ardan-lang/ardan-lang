//
//  IRGenerator.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#include "IRGenerator.hpp"

std::shared_ptr<TurboChunk> IRCodeGenerator::generate(IRGraph& graph) {
    chunk = std::make_shared<TurboChunk>();
    nodeToRegister.clear();
    nextRegister = 0;
    
    auto nodes = graph.getReversePostOrder();
    for (auto& node : nodes) {
        generateNode(node);
    }
    
    return chunk;
}

uint8_t IRCodeGenerator::allocateRegister(IRNodePtr node) {
    if (nodeToRegister.count(node)) {
        return nodeToRegister[node];
    }
    
    uint8_t reg = nextRegister++;
    nodeToRegister[node] = reg;
    return reg;
}

uint8_t IRCodeGenerator::getRegister(IRNodePtr node) {
    if (nodeToRegister.count(node)) {
        return nodeToRegister[node];
    }
    return allocateRegister(node);
}

void IRCodeGenerator::generateNode(IRNodePtr node) {
    switch (node->type) {
        case IRNodeType::Constant:
            generateConstant(node);
            break;
        case IRNodeType::Add:
        case IRNodeType::Sub:
        case IRNodeType::Mul:
        case IRNodeType::Div:
            generateBinaryOp(node);
            break;
        case IRNodeType::Neg:
        case IRNodeType::Not:
            generateUnaryOp(node);
            break;
        case IRNodeType::Return:
            generateReturn(node);
            break;
        case IRNodeType::Call:
            generateCall(node);
            break;
        case IRNodeType::Load:
            generateLoad(node);
            break;
        case IRNodeType::Store:
            generateStore(node);
            break;
        default:
            break;
    }
}

void IRCodeGenerator::generateBinaryOp(IRNodePtr node) {
    if (node->inputs.size() < 2) return;
    
    uint8_t dest = allocateRegister(node);
    uint8_t left = getRegister(node->inputs[0]->from);
    uint8_t right = getRegister(node->inputs[1]->from);
    
    TurboOpCode op = nodeTypeToOpcode(node->type);
    Instruction inst(op, dest, left, right);
    chunk->code.push_back(inst);
}

void IRCodeGenerator::generateUnaryOp(IRNodePtr node) {
    if (node->inputs.empty()) return;
    
    uint8_t dest = allocateRegister(node);
    uint8_t src = getRegister(node->inputs[0]->from);
    
    TurboOpCode op = nodeTypeToOpcode(node->type);
    Instruction inst(op, dest, src, 0);
    chunk->code.push_back(inst);
}

void IRCodeGenerator::generateConstant(IRNodePtr node) {
    uint8_t dest = allocateRegister(node);
    
    Value constVal; 
    uint32_t constIndex = chunk->addConstant(constVal);
    
    Instruction inst(TurboOpCode::LoadConst, dest, constIndex, 0);
    chunk->code.push_back(inst);
}

void IRCodeGenerator::generateReturn(IRNodePtr node) {
    uint8_t src = 0;
    if (!node->inputs.empty()) {
        src = getRegister(node->inputs[0]->from);
    }
    
    Instruction inst(TurboOpCode::Return, src, 0, 0);
    chunk->code.push_back(inst);
}

void IRCodeGenerator::generateCall(IRNodePtr node) {
    if (node->inputs.empty()) return;
    
    uint8_t dest = allocateRegister(node);
    uint8_t callee = getRegister(node->inputs[0]->from);
    
    for (size_t i = 1; i < node->inputs.size(); ++i) {
        uint8_t arg = getRegister(node->inputs[i]->from);
        Instruction pushInst(TurboOpCode::PushArg, arg, 0, 0);
        chunk->code.push_back(pushInst);
    }
    
    Instruction callInst(TurboOpCode::Call, dest, callee, node->inputs.size() - 1);
    chunk->code.push_back(callInst);
}

void IRCodeGenerator::generateLoad(IRNodePtr node) {
    uint8_t dest = allocateRegister(node);
}

void IRCodeGenerator::generateStore(IRNodePtr node) {
}

TurboOpCode IRCodeGenerator::nodeTypeToOpcode(IRNodeType type) {
    switch (type) {
        case IRNodeType::Add: return TurboOpCode::Add;
        case IRNodeType::Sub: return TurboOpCode::Subtract;
        case IRNodeType::Mul: return TurboOpCode::Multiply;
        case IRNodeType::Div: return TurboOpCode::Divide;
        case IRNodeType::Neg: return TurboOpCode::Negate;
        case IRNodeType::Not: return TurboOpCode::LogicalNot;
        default: return TurboOpCode::Nop;
    }
}

