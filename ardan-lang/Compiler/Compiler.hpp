//
//  Compiler.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#pragma once
#ifndef Compiler_hpp
#define Compiler_hpp

#include "VM/VMv2.hpp"
#include "CodeGeneratorV2.hpp"
#include "VM/Module.hpp"

class Chunk;

class Compiler {
public:
    Compiler();
    ~Compiler();
    void compile(const vector<unique_ptr<Statement>> &ast);
};

#endif /* Compiler_hpp */
