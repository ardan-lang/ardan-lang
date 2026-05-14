//
//  Compiler.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#pragma once
#ifndef Compiler_hpp
#define Compiler_hpp

#include "engines/Cascade/VM/VM.hpp"
#include "engines/Cascade/CodeGenerator.hpp"
#include "engines/Cascade/VM/Module.hpp"

#include "ArdarFileManager/WriteArdarFile/WriteArdarFile.hpp"
#include "ArdarFileManager/ArdarFileReader/ArdarFileReader.hpp"

#include "engines/Nova/TurboVM.hpp"
#include "engines/Nova/TurboCodeGenerator.hpp"
#include "engines/Nova/TurboModule.hpp"

#include "Compiler/arm64/ARM64Emitter.hpp"
#include "Compiler/arm64/ARM64CodeGen.hpp"

#include "engines/Peregrine/peregrine/PeregrineCodeGen.hpp"
#include "engines/Peregrine/PeregrineVM.hpp"

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
    
    void runTurbo(shared_ptr<TurboModule> module_);
    void write_ardar_turbo(string outputFilename, shared_ptr<TurboModule> module_, uint32_t entryChunkIndex);
    shared_ptr<TurboModule> read_ardar_turbo(string outputFilename);
    void run_arm(const std::vector<std::unique_ptr<Statement>>& ast);
};

#endif /* Compiler_hpp */
