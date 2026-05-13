//
//  IRBuilder.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#include "IRBuilder.hpp"
#include <stdexcept>

IRBuilder::IRBuilder() {
    graph.start = graph.newNode(IRNodeType::Start);
    graph.end = graph.newNode(IRNodeType::End);
}

IRNodePtr IRBuilder::buildFromChunk(std::shared_ptr<TurboChunk> chunk) {
    currentChunk = chunk;
    registerNodes.resize(256, nullptr);
    
    for (uint32_t i = 0; i < chunk->arity; ++i) {
        registerNodes[i] = graph.newParameter(i);
    }
    
    controlStack.push(graph.start);
    effectStack.push(graph.start);
    
    size_t ip = 0;
    while (ip < chunk->code.size()) {
        auto node = processInstruction(chunk->code[ip], ip);
        if (node) {
            
            if (!controlStack.empty()) {
                graph.addControlEdge(controlStack.top(), node);
            }
            if (!effectStack.empty()) {
                graph.addEffectEdge(effectStack.top(), node);
            }
        }
        ++ip;
    }
    
    if (!controlStack.empty()) {
        graph.addControlEdge(controlStack.top(), graph.end);
    }
    if (!effectStack.empty()) {
        graph.addEffectEdge(effectStack.top(), graph.end);
    }
    
    return graph.end;
}

IRNodePtr IRBuilder::getRegisterNode(uint8_t reg) {
    if (reg >= registerNodes.size()) return nullptr;
    return registerNodes[reg];
}

void IRBuilder::setRegisterNode(uint8_t reg, IRNodePtr node) {
    if (reg >= registerNodes.size()) {
        registerNodes.resize(reg + 1, nullptr);
    }
    registerNodes[reg] = node;
}

IRNodePtr IRBuilder::processInstruction(const Instruction& inst, size_t& ip) {
    switch (inst.op) {
        case TurboOpCode::LoadConst: {
            auto value = buildLoadConst(inst.b);
            setRegisterNode(inst.a, value);
            return value;
        }
        case TurboOpCode::Move: {
            auto src = getRegisterNode(inst.b);
            setRegisterNode(inst.a, src);
            return src;
        }
        case TurboOpCode::Add: {
            auto left = getRegisterNode(inst.b);
            auto right = getRegisterNode(inst.c);
            auto result = buildBinaryOp(inst.op, left, right);
            setRegisterNode(inst.a, result);
            return result;
        }
        case TurboOpCode::Subtract: {
            auto left = getRegisterNode(inst.b);
            auto right = getRegisterNode(inst.c);
            auto result = buildBinaryOp(inst.op, left, right);
            setRegisterNode(inst.a, result);
            return result;
        }
        case TurboOpCode::Multiply: {
            auto left = getRegisterNode(inst.b);
            auto right = getRegisterNode(inst.c);
            auto result = buildBinaryOp(inst.op, left, right);
            setRegisterNode(inst.a, result);
            return result;
        }
        case TurboOpCode::Divide: {
            auto left = getRegisterNode(inst.b);
            auto right = getRegisterNode(inst.c);
            auto result = buildBinaryOp(inst.op, left, right);
            setRegisterNode(inst.a, result);
            return result;
        }
        case TurboOpCode::Return: {
            auto value = getRegisterNode(inst.a);
            return buildReturn(value);
        }
        case TurboOpCode::Call: {
            std::vector<IRNodePtr> args;
            auto callee = getRegisterNode(inst.a);
            auto result = buildCall(callee, args);
            setRegisterNode(inst.a, result);
            return result;
        }
        case TurboOpCode::JumpIfFalse: {
            auto cond = getRegisterNode(inst.a);
            size_t target = ip + inst.b;
            return buildBranch(cond, ip + 1, target);
        }
        default:
            return nullptr;
    }
}

IRNodePtr IRBuilder::buildBinaryOp(TurboOpCode op, IRNodePtr left, IRNodePtr right) {
    IRNodeType nodeType;
    switch (op) {
        case TurboOpCode::Add: nodeType = IRNodeType::Add; break;
        case TurboOpCode::Subtract: nodeType = IRNodeType::Sub; break;
        case TurboOpCode::Multiply: nodeType = IRNodeType::Mul; break;
        case TurboOpCode::Divide: nodeType = IRNodeType::Div; break;
        default: nodeType = IRNodeType::Add; break;
    }
    
    auto node = graph.newNode(nodeType, opcodeToValueType(op));
    graph.addEdge(left, node, 0);
    graph.addEdge(right, node, 1);
    return node;
}

IRNodePtr IRBuilder::buildUnaryOp(TurboOpCode op, IRNodePtr operand) {
    IRNodeType nodeType = (op == TurboOpCode::Negate) ? IRNodeType::Neg : IRNodeType::Not;
    auto node = graph.newNode(nodeType, opcodeToValueType(op));
    graph.addEdge(operand, node, 0);
    return node;
}

IRNodePtr IRBuilder::buildLoadConst(uint32_t index) {
    if (index >= currentChunk->constants.size()) {
        throw std::runtime_error("Invalid constant index");
    }
    return graph.newConstant(currentChunk->constants[index]);
}

IRNodePtr IRBuilder::buildLoad(uint8_t reg) {
    return getRegisterNode(reg);
}

IRNodePtr IRBuilder::buildStore(uint8_t reg, IRNodePtr value) {
    setRegisterNode(reg, value);
    return value;
}

IRNodePtr IRBuilder::buildCall(IRNodePtr callee, const std::vector<IRNodePtr>& args) {
    auto node = graph.newNode(IRNodeType::Call, IRValueType::Any);
    graph.addEdge(callee, node, 0);
    for (size_t i = 0; i < args.size(); ++i) {
        graph.addEdge(args[i], node, i + 1);
    }
    return node;
}

IRNodePtr IRBuilder::buildReturn(IRNodePtr value) {
    auto node = graph.newNode(IRNodeType::Return, IRValueType::Any);
    if (value) {
        graph.addEdge(value, node, 0);
    }
    return node;
}

IRNodePtr IRBuilder::buildJump(size_t target) {
    return graph.newNode(IRNodeType::Region, IRValueType::Any);
}

IRNodePtr IRBuilder::buildBranch(IRNodePtr condition, size_t trueTarget, size_t falseTarget) {
    auto node = graph.newNode(IRNodeType::If, IRValueType::Boolean);
    graph.addEdge(condition, node, 0);
    return node;
}

IRValueType IRBuilder::opcodeToValueType(TurboOpCode op) {
    switch (op) {
        case TurboOpCode::Add:
        case TurboOpCode::Subtract:
        case TurboOpCode::Multiply:
        case TurboOpCode::Divide:
            return IRValueType::Float64;
        case TurboOpCode::Equal:
        case TurboOpCode::NotEqual:
        case TurboOpCode::LessThan:
        case TurboOpCode::GreaterThan:
            return IRValueType::Boolean;
        default:
            return IRValueType::Any;
    }
}
