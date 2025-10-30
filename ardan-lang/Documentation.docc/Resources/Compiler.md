# Compiler

```
Source (JS)
 ↓
Parser → AST
 ↓
Ignition → Bytecode
 ↓ (Hot function)
TurboFan → Machine code (optimized)
 ↓
CPU executes native instructions
```

## How Compilers Emit Native Machine Code

1. Intermediate Representation (IR)
Compilers rarely generate native machine code directly from source or AST. Instead, they translate code into an intermediate representation—often something like LLVM IR, bytecode, or custom IR. This allows the compiler to optimize code before final code generation.

2. Code Generation Phase
After optimizations, the IR is traversed by a code generator, which emits native instructions for the target CPU architecture (like x86_64, ARM64, etc.). This involves:
• Mapping IR operations to instruction patterns
• Allocating registers
• Managing calling conventions and stack layouts

3. Writing Machine Code
The compiler writes out binary opcodes into a memory buffer or file. This is now true native code that a CPU can execute.

```
let module = llvmParseIR(sourceIR)
let targetMachine = llvmCreateTargetMachine(target: "x86_64-apple-macosx")
let objectBuffer = llvmEmitObject(module, targetMachine)
// The buffer now contains native code instructions
```

Example with JIT (Just-In-Time Compilation):
Some systems (like V8 or Swift’s LLJIT) can generate machine code at runtime, allocating executable memory and patching in generated instructions.

4. Execution
The emitted code is either saved as an executable file (static compilation) or executed directly from memory (JIT).


