//
//  ExceptionManager.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 03/11/2025.
//

#ifndef ExceptionManager_hpp
#define ExceptionManager_hpp

#pragma once
#include <stdio.h>
#include <vector>

class ExceptionManager {
    
    struct TryFrame {
        int catchIP = -1;
        int finallyIP = -1;
        int stackDepth = 0;
        int ipAfterTry = -1;
        uint8_t regCatch = 0;
    };
    
public:
    void pushTryFrame(const TryFrame& f) { tryStack_.push_back(f); }
    void popTryFrame() { if (!tryStack_.empty()) tryStack_.pop_back(); }
    bool empty() const { return tryStack_.empty(); }
    TryFrame& top() { return tryStack_.back(); }
    void clear() { tryStack_.clear(); }
    
    // simplified rethrow handling - caller must provide access to current VM frame/state
    const std::vector<TryFrame>& stack() const { return tryStack_; }
    
private:
    std::vector<TryFrame> tryStack_;
};

#endif /* ExceptionManager_hpp */
