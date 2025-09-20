//
//  Compiler.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "Compiler.hpp"

Compiler::Compiler() {}

Compiler::~Compiler() {}

void Compiler::compile(const vector<unique_ptr<Statement>> &ast) {
    auto codegen = std::make_shared<CodeGen>();
    VM vm;
    vm.run(codegen->generate(ast));
}
