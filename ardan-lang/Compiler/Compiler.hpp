//
//  Compiler.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#pragma once
#ifndef Compiler_hpp
#define Compiler_hpp

#include "VM/VM.hpp"
#include "CodeGenerator.hpp"
#include "VM/Module.hpp"
#include "WriteArdarFile/WriteArdarFile.hpp"
#include "ArdarFileReader/ArdarFileReader.hpp"

#include "Turbo/TurboVM.hpp"
#include "Turbo/TurboCodeGenerator.hpp"
#include "Turbo/TurboModule.hpp"

class Chunk;

class Compiler {
public:
    Compiler();
    ~Compiler();
    shared_ptr<Module> compile(const vector<unique_ptr<Statement>> &ast);
    shared_ptr<Module> read_ardar(string outputFilename);
    void write_ardar(string outputFilename,
                               shared_ptr<Module> module_,
                               uint32_t entryChunkIndex);
    void run(shared_ptr<Module> module_);
    void test_compile(const std::vector<std::unique_ptr<Statement>>& ast);
    void test_turbo_compile(const std::vector<std::unique_ptr<Statement>>& ast);
    
};

#endif /* Compiler_hpp */
