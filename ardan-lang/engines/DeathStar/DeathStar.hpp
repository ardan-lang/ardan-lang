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

/**
 * 🌑
 */
namespace ardan {

struct Context {
    std::shared_ptr<Context> parent;
    std::vector<Value> slots;
    
    explicit Context(
            int slotCount,
            std::shared_ptr<Context> parent = nullptr
        )
            : parent(std::move(parent)),
              slots(static_cast<size_t>(std::max(slotCount, 0)),
                    Value::undefined()) {}
};

struct Closure {
    int functionIndex = -1;
    std::shared_ptr<Context> capturedContext;
};

struct VMException : std::exception {
    Value value;
    explicit VMException(Value v) : value(std::move(v)) {}
    const char* what() const noexcept override { return "uncaught VM exception"; }
};

struct CallFrame {
    
    size_t ip = 0;
    
    Value accumulator;
    
    unordered_map<int, Value> registers;
    unordered_map<int, Value> arguments;
    
    shared_ptr<Context> ownContext;
    shared_ptr<Context> parentContext;
    
    BytecodeModule bytecode_module;
    
};

}

class DeathStar {
        
private:
    
    Compiled& compiled;
    
    Value reg(ardan::CallFrame& f, int index);
    uint8_t next(ardan::CallFrame& code);
    uint8_t fetchByte(ardan::CallFrame& f);
    Bytecode fetchOp(ardan::CallFrame& f);
    uint32_t fetchU32(ardan::CallFrame& f);
    int32_t fetchI32(ardan::CallFrame& f) { return static_cast<int32_t>(fetchU32(f)); }
    
public:
    DeathStar(Compiled& compiled) : compiled(compiled) {}
    
    void runProgram();
    Value run(ardan::CallFrame& frame);
    Value call(int entry_index,
              vector<Value> args,
              shared_ptr<ardan::Context> capturedCtx = nullptr);
    
};

#endif /* DeathStar_hpp */
