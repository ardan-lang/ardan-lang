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




High-Level Approach

1. Create an ARM64 Emitter: Handles writing raw instructions to a buffer.
2. Write Visitor Methods for Each AST Node: Each visitor lowers its node to ARM64 using the emitter.
3. Manage Registers and Stack: Allocate/free registers, handle locals/args, manage stack for function calls, etc.


⸻

2. Register Allocator

Use a simple allocator as in your code already.

⸻

3. Generic Codegen Visitor

For each AST node, emit ARM64 that implements its semantics. Here is a comprehensive sample for the core constructs:

⸻

4. Assembly Helpers

You need helpers for each instruction form. Example:

⸻

5. Data Section for Strings/Constants

Add a data section for literal strings, arrays, etc.:

⸻

6. Example Output

Here’s a minimal real codegen for return 3+5;:


7. Final Notes

• Scopes and Locals: Use frame pointer (FP) and stack offsets for locals.
• Arguments: Pass in x0-x7 registers.
• Function Calls: Move callee address to a register, use blr, pass/receive args in x0-x7, x0.
• Objects/Arrays: For these, you’ll write custom memory layouts and field access logic.
• Control Flow: Use labels, branch instructions, and register/stack management.

⸻

In summary:
For each AST node, emit the corresponding ARM64 instructions into an in-memory buffer using a visitor pattern, a register allocator, and an instruction emitter. Each node’s codegen can be made as above (see pseudo-implementations per node). For a real compiler, expand each node handler to cover your language’s semantics.
