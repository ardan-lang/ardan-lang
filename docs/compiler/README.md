# Ardan Compiler Pipeline

This folder documents how an Ardan program travels from source text to execution
on the **DeathStar** virtual machine, following the flowchart in `Ardan Lang.pdf`:

```
 ┌────────┐   ┌───────────┐   ┌────────┐   ┌─────┐
 │ Source │──▶│ Tokenizer │──▶│ Parser │──▶│ AST │
 └────────┘   └───────────┘   └────────┘   └──┬──┘
                                              │
   ┌──────────────────────────────────────────┘
   ▼
 ┌────────────┐   ┌───────────┐   ┌───────────────────┐   ┌──────────┐   ┌────────────────┐
 │ IR Builder │──▶│ Optimizer │──▶│ Turbine           │──▶│ Bytecode │──▶│ VM (DeathStar) │
 │            │   │ (Pharaoh) │   │ (bytecode lowerer)│   │          │   │                │
 └────────────┘   └───────────┘   └───────────────────┘   └──────────┘   └────────────────┘
```

| # | Stage | Input → Output | Main type / entry point | Source |
|---|-------|----------------|-------------------------|--------|
| 1 | [Tokenizer](01-tokenizer.md) | `string` → `vector<Token>` | `Scanner::getTokens()` | `ardan-lang/Scanner/` |
| 2 | [Parser & AST](02-parser-ast.md) | `vector<Token>` → `vector<unique_ptr<Statement>>` | `Parser::parse()` | `ardan-lang/Parser/`, `Statements/`, `Expression/` |
| 3 | [IR model](03-ir-model.md) | – | `IRModule`, `IRFunction`, `BasicBlock`, `IRInstruction`, `IRValue` | `ardan-lang/IR/ir/` |
| 4 | [IR Builder](04-ir-builder.md) | AST → `IRModule` | `IRBuilderVisitor::build()` | `ardan-lang/IR/ir/IRBuilderVisitor/` |
| 5 | [Optimizer (Pharaoh)](05-pharaoh.md) | `IRModule` → `IRModule` (in place) | `Pharoah::start()` | `ardan-lang/IR/optimizer/Pharoah/` |
| 6 | [Turbine](06-turbine.md) | `IRModule` → `Compiled` | `AssemblyLine::start()` | `ardan-lang/Turbine/` |
| 7 | [DeathStar VM](07-deathstar-vm.md) | `Compiled` → side effects | `DeathStar::runProgram()` | `ardan-lang/engines/DeathStar/` |

Supporting chapters:

- [08 — Worked example](08-worked-example.md): one program traced through every stage, with the real IR and decoded bytes.
- [09 — Status & known issues](09-known-issues.md): what works, what does not, with minimal reproductions.

> **Note on spelling.** The flowchart says *Pharaoh*; the C++ class is spelled `Pharoah`.
> These docs use the class name when referring to code.

## Where the pipeline is driven

There is no CLI flag for the IR pipeline yet. It runs from `test_ir()` in
`ardan-lang/main.cpp`, which `main()` calls unconditionally before returning:

```cpp
// ardan-lang/main.cpp — test_ir()
string source = read_file(entryFileName);          // Source
auto ast = get_ast(source, entryFileName);         // Scanner + Parser  → AST

IRBuilderVisitor builder;
builder.build(ast);                                // AST → IRModule

//    Pharoah pharoah;                             // Optimizer (currently disabled)
//    pharoah.start(builder.irModule);

AssemblyLine assemblyLine;
Compiled compiled = assemblyLine.start(builder.irModule);   // Turbine → Bytecode

DeathStar deathStar(compiled);
deathStar.runProgram();                            // VM
```

`get_ast()` is the shared front end, also used by the tree-walking `Interpreter`
(`--interpret`) and the older `Compiler` (`--compile`, Cascade/Nova engines):

```cpp
vector<unique_ptr<Statement>> get_ast(string source, string filename) {
    Scanner scanner(source);
    auto tokens = scanner.getTokens();
    Parser parser(tokens);
    parser.sourceFile = filename;
    return parser.parse();
}
```

## Other engines in the tree

The repository contains several execution back ends. Only the path above is
the subject of these docs. The others share the front end (stages 1–2) but
generate code directly from the AST, without the IR:

| Engine | Directory | Notes |
|--------|-----------|-------|
| Tree-walking interpreter | `Interpreter/` | `--interpret` |
| Cascade | `engines/Cascade/` | AST → `Chunk` bytecode, used by `Compiler` / `.ardar` files |
| Nova (Turbo) | `engines/Nova/` | register-based Turbo bytecode |
| Peregrine, Atlas, Titan | `engines/…` | experimental VMs |
| ARM64 | `Compiler/arm64/` | native code generation experiment |

### Legacy and dead IR files

`ardan-lang/IR/` also holds earlier attempts that are **not** part of the live
pipeline. Most are fully commented out:

| Path | Status |
|------|--------|
| `IR/ir/IRNode/`, `IR/ir/IRGraph/`, `IR/ir/IRBuilder/` | Commented out. A V8 TurboFan-style "sea of nodes" graph (control/effect/value edges) that was built *from* Nova bytecode. |
| `IR/ir/IRGenerator/`, `IR/optimizer/Optimizer/`, `IR/optimizer/TurbofanOptimizer/` | Commented out; target the sea-of-nodes graph. |
| `IR/IRBuilderVisitor*.cpp`, `IR/BytecodeEmitter*.hpp`, `IR/ir/IRBuilderVisitor*.cpp`, `IR/ardan-lang/*` | Older snapshots, commented out. |
| `IR/IRBuilderVisitor-2.cpp` | Snapshot whose header comment is malformed (`/  IRBuilderVisitor.cpp`). It does **not** compile, and the root `CMakeLists.txt` glob picks it up, so `cmake --build` fails. See [known issues](09-known-issues.md#build). |

The live IR is the **basic-block / SSA-style** representation described in
[03-ir-model.md](03-ir-model.md).
