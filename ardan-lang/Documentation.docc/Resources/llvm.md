# llvm

```cpp

// llvm_jit_add.cpp
#include <iostream>
#include <memory>

#include "llvm/ADT/APInt.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

int main() {
    // Initialize native target (makes LLVM generate host machine code)
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    LLVMContext context;
    std::unique_ptr<Module> module = std::make_unique<Module>("my_module", context);
    IRBuilder<> builder(context);

    // Create function type: int(int, int)
    FunctionType *funcType = FunctionType::get(builder.getInt32Ty(),
                                               {builder.getInt32Ty(), builder.getInt32Ty()},
                                               false);

    // Create function `int add(int a, int b)`
    Function *addFunc = Function::Create(funcType, Function::ExternalLinkage, "add", module.get());

    // Name the arguments for clarity
    Function::arg_iterator args = addFunc->arg_begin();
    Value* argA = args++;
    argA->setName("a");
    Value* argB = args++;
    argB->setName("b");

    // Entry block
    BasicBlock *entry = BasicBlock::Create(context, "entry", addFunc);
    builder.SetInsertPoint(entry);

    // body: return a + b;
    Value *sum = builder.CreateAdd(argA, argB, "sum");
    builder.CreateRet(sum);

    // Verify module
    std::string err;
    raw_string_ostream rso(err);
    if (verifyModule(*module, &rso)) {
        std::cerr << "Error constructing module:\n" << rso.str() << std::endl;
        return 1;
    }

    // Print the IR to stdout for debugging
    module->print(outs(), nullptr);

    // Create ExecutionEngine (MCJIT)
    std::string engineError;
    EngineBuilder builderEngine(std::move(module));
    builderEngine.setEngineKind(EngineKind::JIT);
    builderEngine.setErrorStr(&engineError);
    builderEngine.setMCJITMemoryManager(std::make_unique<SectionMemoryManager>());

    ExecutionEngine *ee = builderEngine.create();
    if (!ee) {
        std::cerr << "Failed to create ExecutionEngine: " << engineError << std::endl;
        return 1;
    }

    // Finalize object to ensure JIT code is emitted
    ee->finalizeObject();

    // Get function pointer
    void *fnPtr = ee->getPointerToFunction(addFunc);
    if (!fnPtr) {
        std::cerr << "Failed to get function pointer." << std::endl;
        return 1;
    }

    // Cast to callable and call it
    using AddFn = int (*)(int, int);
    AddFn add = reinterpret_cast<AddFn>(fnPtr);

    int a = 42, b = 58;
    int result = add(a, b);
    std::cout << "add(" << a << ", " << b << ") = " << result << std::endl;

    // Clean up
    delete ee;
    return 0;
}

```
