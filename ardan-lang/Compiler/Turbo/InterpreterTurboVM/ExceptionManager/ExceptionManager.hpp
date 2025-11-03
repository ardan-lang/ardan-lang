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

struct TryFrame {
    int catchIP;
    int finallyIP;
    int stackDepth;
    int ipAfterTry;
    uint8_t regCatch;
};

class ExceptionManager {
public:
    void handleRethrow();
private:
    std::vector<TryFrame> tryStack;
};

#endif /* ExceptionManager_hpp */
