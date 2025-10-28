//
//  Compiler.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#include "Compiler.hpp"

Compiler::Compiler() {}

Compiler::~Compiler() {}

shared_ptr<Module> Compiler::compile(const std::vector<std::unique_ptr<Statement>>& ast) {

    shared_ptr<Module> module_ = make_shared<Module>();
    auto codegen = make_shared<CodeGen>(module_);

    auto entryChunkIndex = codegen->generate(ast);
    module_->entryChunkIndex = (uint32_t)entryChunkIndex;
    
    return module_;
        
}

void Compiler::test_compile(const std::vector<std::unique_ptr<Statement>>& ast) {
    shared_ptr<Module> module_ = make_shared<Module>();
    auto codegen = make_shared<CodeGen>(module_);

    // generate fills module_->chunks and module_->constants
    auto entryChunkIndex = codegen->generate(ast);
    module_->entryChunkIndex = (uint32_t)entryChunkIndex;

    std::string outputFilename = "/Users/chidumennamdi/Documents/MacBookPro2020/developerse/xcode-prjs/ardan-lang/ardan-lang/tests/myprogram.adar";
    uint32_t version = 1;

    WriteArdarFile writer(outputFilename,
                          module_.get(),
                          (uint32_t)entryChunkIndex,
                          version);

    writer.writing();

    cout << "File written successfully!" << endl;
        
    // load and run
    ArdarFileReader reader(outputFilename);
    shared_ptr<Module> _module_ = reader.readModule();

    VM vm(_module_);

    // OR explicitly by chunk index
    Value ret = vm.run(_module_->chunks[_module_->entryChunkIndex], {});
}

void Compiler::test_turbo_compile(const std::vector<std::unique_ptr<Statement>>& ast) {

    shared_ptr<TurboModule> module_ = make_shared<TurboModule>();
    auto codegen = make_shared<TurboCodeGen>(module_);

    // generate fills module_->chunks and module_->constants
    auto entryChunkIndex = codegen->generate(ast);
    // module_->entryChunkIndex = (uint32_t)entryChunkIndex;

    std::string outputFilename = "/Users/chidumennamdi/Documents/MacBookPro2020/developerse/xcode-prjs/ardan-lang/ardan-lang/tests/myprogram.adar";
    uint32_t version = 2;

    WriteArdarFile writer(outputFilename,
                          module_.get(),
                          (uint32_t)entryChunkIndex,
                          version);

    writer.writingTurbo(module_.get());

    cout << "File written successfully!" << endl;
        
    // load and run
    ArdarFileReader reader(outputFilename);
    shared_ptr<TurboModule> readModule = reader.readTurboModule(outputFilename);

    TurboVM vm(readModule);

    // OR explicitly by chunk index
    Value ret = vm.run(readModule->chunks[readModule->entryChunkIndex], {});
}

void Compiler::runTurbo(shared_ptr<TurboModule> module_) {
    
    TurboVM vm(module_);

    // OR explicitly by chunk index
    Value ret = vm.run(module_->chunks[module_->entryChunkIndex], {});

}

void Compiler::write_ardar_turbo(string outputFilename, shared_ptr<TurboModule> module_, uint32_t entryChunkIndex) {
    
    WriteArdarFile writer(outputFilename,
                          module_.get(),
                          (uint32_t)entryChunkIndex,
                          2);

    writer.writingTurbo(module_.get());

    cout << "File written successfully!" << endl;

}

shared_ptr<TurboModule> Compiler::read_ardar_turbo(string outputFilename) {
    ArdarFileReader reader(outputFilename);
    shared_ptr<TurboModule> readModule = reader.readTurboModule(outputFilename);
    return readModule;
}

void Compiler::run(shared_ptr<Module> module_) {

    VM vm(module_);

    Value ret = vm.run(module_->chunks[module_->entryChunkIndex], {});

}

void Compiler::write_ardar(string outputFilename,
                           shared_ptr<Module> module_,
                           uint32_t entryChunkIndex) {
    
    uint32_t version = 1;

    WriteArdarFile writer(outputFilename,
                          module_.get(),
                          (uint32_t)entryChunkIndex,
                          version);

    writer.writing();

    cout << "File written successfully!" << endl;

}

shared_ptr<Module> Compiler::read_ardar(string outputFilename) {

    ArdarFileReader reader(outputFilename);
    shared_ptr<Module> _module_ = reader.readModule();
    
    return _module_;

}
