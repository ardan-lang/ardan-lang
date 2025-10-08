//
//  Compiler.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "Compiler.hpp"

Compiler::Compiler() {}

Compiler::~Compiler() {}

//void Compiler::compile(const vector<unique_ptr<Statement>> &ast) {
//    auto codegen = std::make_shared<CodeGen>();
//    Module module_;
//    // ... codegen fills module.chunks and module.constants ...
//    VM vm(&module_);
//    auto bytceodes = codegen->generate(ast);
//    vm.run(bytceodes);
//    
//    //
//    //    // To call a top-level chunk (entry) with arguments:
//    //    Value ret = vm.run(module_.chunks[entryIndex], {}); // run will still work if you keep it
//    
//}

void Compiler::compile(const std::vector<std::unique_ptr<Statement>>& ast) {

    shared_ptr<Module> module_ = make_shared<Module>();   // shared Module
    auto codegen = make_shared<CodeGen>(module_);

    // generate fills module_->chunks and module_->constants
    // auto entryChunk = codegen->generate(ast);
    auto entryChunkIndex = codegen->generate(ast);
    module_->entryChunkIndex = (uint32_t)entryChunkIndex;

    // VM vm(module_);
    // vm.run(entryChunk);

    std::string outputFilename = "/Users/chidumennamdi/Documents/MacBookPro2020/developerse/xcode-prjs/ardan-lang/ardan-lang/tests/myprogram.adar";
    uint32_t version = 1;

    WriteArdarFile writer(outputFilename,
                          module_.get(),
                          version,
                          (uint32_t)entryChunkIndex);
    writer.writing();
    cout << "File written successfully!" << endl;
        
    // load and run
    ArdarFileReader reader(outputFilename);
    shared_ptr<Module> _module_ = reader.readModule();

    // Pass module to VM for execution!
    VM vm(_module_);

    // OR explicitly by chunk index
    Value ret = vm.run(_module_->chunks[_module_->entryChunkIndex], {});

}
