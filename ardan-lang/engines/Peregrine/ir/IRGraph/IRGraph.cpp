//
//  IRGraph.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#include "IRGraph.hpp"

IRNodePtr IRGraph::newNode(IRNodeType type, IRValueType vt) {
    auto node = std::make_shared<IRNode>(nextId++, type, vt);
    nodes.push_back(node);
    return node;
}

IRNodePtr IRGraph::newConstant(Value value) {
    auto node = newNode(IRNodeType::Constant, IRValueType::Any);
    node->debugName = "const";
    return node;
}

IRNodePtr IRGraph::newParameter(int index) {
    auto node = newNode(IRNodeType::Parameter, IRValueType::Any);
    std::stringstream ss;
    ss << "param" << index;
    node->debugName = ss.str();
    return node;
}

void IRGraph::addEdge(IRNodePtr from, IRNodePtr to, int inputIndex) {
    to->addInput(from, inputIndex);
}

void IRGraph::addControlEdge(IRNodePtr from, IRNodePtr to) {
    to->addControlInput(from);
}

void IRGraph::addEffectEdge(IRNodePtr from, IRNodePtr to) {
    to->addEffectInput(from);
}

void IRGraph::removeNode(IRNodePtr node) {

    for (auto& edge : node->inputs) {
        edge->from->outputs.erase(
            std::remove(edge->from->outputs.begin(), edge->from->outputs.end(), edge),
            edge->from->outputs.end()
        );
    }
    for (auto& edge : node->outputs) {
        edge->to->inputs.erase(
            std::remove(edge->to->inputs.begin(), edge->to->inputs.end(), edge),
            edge->to->inputs.end()
        );
    }
    
    nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());
}

void IRGraph::replaceNode(IRNodePtr oldNode, IRNodePtr newNode) {
    
    for (auto& edge : oldNode->outputs) {
        edge->from = newNode;
        newNode->outputs.push_back(edge);
        
        for (auto& input : edge->to->inputs) {
            if (input->from == oldNode) {
                input->from = newNode;
            }
        }
    }
    
    removeNode(oldNode);
}

std::vector<IRNodePtr> IRGraph::getReversePostOrder() {
    std::vector<IRNodePtr> order;
    std::unordered_set<IRNodePtr> visited;
    
    std::function<void(IRNodePtr)> dfs = [&](IRNodePtr node) {
        if (visited.count(node)) return;
        visited.insert(node);
        
        for (auto& output : node->outputs) {
            dfs(output->to);
        }
        for (auto& control : node->controlOutputs) {
            dfs(control);
        }
        order.push_back(node);
    };
    
    if (start) dfs(start);
    std::reverse(order.begin(), order.end());
    return order;
}

std::vector<IRNodePtr> IRGraph::getDominators() {
    return getReversePostOrder();
}
