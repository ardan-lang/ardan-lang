//
//  CallStackManager.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#include "CallStackManager.hpp"

void CallStackManager::pushFrame(TurboCallFrame&& frame) {
    stack.push_back(std::move(frame));
}

void CallStackManager::popFrame() {
    if (!stack.empty()) stack.pop_back();
}

TurboCallFrame* CallStackManager::top() {
    return stack.empty() ? nullptr : &stack.back();
}

bool CallStackManager::empty() const {
    return stack.empty();
}
