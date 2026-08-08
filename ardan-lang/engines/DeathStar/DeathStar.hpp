//
//  DeathStar.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 01/08/2026.
//

#ifndef DeathStar_hpp
#define DeathStar_hpp

#include <stdio.h>
#include <vector>
#include <stdio.h>
#include <cstdint>

#include "Interpreter/Utils/Utils.h"
#include "Turbine/Turbine.hpp"

using namespace std;

struct FrameContext {
    FrameContext* parent;
    std::vector<Value> slots;
};

/**
 * 🌑
 */
class DeathStar {
    
    struct CallFrame {
        size_t ip = 0;
        
        unordered_map<int, Value> registers;
        unordered_map<int, Value> arguments;
    };

private:
    Compiled& compiled;
    Value accumulator;
    CallFrame frame;
    FrameContext ctx;
    
    uint8_t next(vector<uint8_t>& code);

public:
    DeathStar(Compiled& compiled) : compiled(compiled) {}
    void run();
    
};

#endif /* DeathStar_hpp */
