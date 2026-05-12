//
//  IRNode.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/05/2026.
//

#include "IRNode.hpp"
#include <sstream>

void IRNode::addInput(IRNodePtr input, int index) {
    if (index == -1) {
        index = inputs.size();
    }
    auto edge = std::make_shared<IREdge>(input, shared_from_this(), index);
    inputs.push_back(edge);
    input->outputs.push_back(edge);
}

void IRNode::addControlInput(IRNodePtr control) {
    controlInputs.push_back(control);
    control->controlOutputs.push_back(shared_from_this());
}

void IRNode::addEffectInput(IRNodePtr effect) {
    effectInputs.push_back(effect);
    effect->effectOutputs.push_back(shared_from_this());
}

void IRNode::replaceInput(int index, IRNodePtr newInput) {
    if (index >= inputs.size()) return;
    
    auto oldEdge = inputs[index];
    oldEdge->from->outputs.erase(
        std::remove(oldEdge->from->outputs.begin(), oldEdge->from->outputs.end(), oldEdge),
        oldEdge->from->outputs.end()
    );
    
    oldEdge->from = newInput;
    newInput->outputs.push_back(oldEdge);
}

void IRNode::removeInput(int index) {
    if (index >= inputs.size()) return;
    
    auto edge = inputs[index];
    edge->from->outputs.erase(
        std::remove(edge->from->outputs.begin(), edge->from->outputs.end(), edge),
        edge->from->outputs.end()
    );
    inputs.erase(inputs.begin() + index);
}

std::string IRNode::toString() const {
    std::stringstream ss;
    ss << id << ": " << static_cast<int>(type);
    if (!debugName.empty()) {
        ss << " (" << debugName << ")";
    }
    return ss.str();
}

