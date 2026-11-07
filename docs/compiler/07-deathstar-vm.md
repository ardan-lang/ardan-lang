# 7. DeathStar VM

**`Compiled` → execution**

| File | Role |
|------|------|
| `ardan-lang/engines/DeathStar/DeathStar.hpp` | `ardan::Context`, `ardan::Closure`, `ardan::CallFrame`, `DeathStar` |
| `ardan-lang/engines/DeathStar/DeathStar.cpp` | Interpreter loop |

DeathStar is an accumulator + register interpreter for the bytecode produced by
[Turbine](06-turbine.md). Values are the project-wide `Value` type
(`Interpreter/ExecutionContext/Value/Value.h`).

## Runtime structures

```cpp
namespace ardan {
struct Context {                         // heap environment for captured variables
    shared_ptr<Context> parent;
    vector<Value> slots;                 // initialised to undefined
};

struct Closure {                         // a function value
    int functionIndex;                   // index into Compiled::modules
    shared_ptr<Context> capturedContext; // context current at creation
};

struct CallFrame {
    size_t ip = 0;
    Value accumulator;
    unordered_map<int, Value> registers;  // sparse; missing → default Value (undefined)
    unordered_map<int, Value> arguments;  // never filled
    shared_ptr<Context> ownContext;       // set by CreateFunctionContext
    shared_ptr<Context> parentContext;    // = closure.capturedContext
    BytecodeModule bytecode_module;       // copied per call
};
}
```

Closures and contexts are stored in `Value` through its `anyValue`
(`std::any`), as `shared_ptr<ardan::Closure>` / `shared_ptr<ardan::Context>`.

## Execution model

```cpp
void DeathStar::runProgram() { call(compiled.entry_index, {}); }

Value DeathStar::call(int fnIndex, vector<Value> args, shared_ptr<Context> captured) {
    CallFrame frame;
    frame.parentContext = captured;
    frame.bytecode_module = compiled.modules[fnIndex];
    return run(frame);                     // native C++ recursion per Ardan call
}
```

`run` is a `while (true) { switch (fetch opcode) }` loop. It exits only through
`kReturn`; there is no `ip` bounds check.

## Instruction semantics

`u32` means four little-endian bytes. `R[x]` means `frame.registers[x]`, and
`acc` is the accumulator.

| Opcode | Operands | Effect |
|--------|----------|--------|
| `kLdaZero` | – | `acc = 0` |
| `kLdaSmi` | `u32 n` | `acc = n` (unsigned: negative values become 2³²−n) |
| `kLdar` | `u32 r` | `acc = R[r]` |
| `kStar` | `u32 r` | `R[r] = acc` |
| `kAdd` / `kSub` / `kMul` | `u32 a, u32 b` | `acc = R[a].num ∘ R[b].num` |
| `kCreateFunctionContext` | `u32 n` | `ctx = Context(n, parent = frame.parentContext)`; `frame.ownContext = ctx`; `acc = ctx` |
| `kLdaCurrentContextSlot` | `u32 s` | `acc = ownContext.slots[s]` |
| `kStaCurrentContextSlot` | `u32 s` | `ownContext.slots[s] = acc` |
| `kLdaContextSlot` | `u32 d, u32 s` | `c = parentContext`, then `c = c.parent` *d* times; `acc = c.slots[s]` |
| `kStaContextSlot` | `u32 d, u32 s` | same walk; `c.slots[s] = acc` |
| `kCreateClosure` | `u32 fn, u32 ctxReg` | `acc = Closure{fn, ctxReg == 0xFFFFFFFF ? null : R[ctxReg].ctx}` |
| `kCallUndefinedReceiver` | `u32 callee, u32 argc, argc × u32 r` | collect args; `acc = call(closure.fn, args, closure.capturedContext)` |
| `kPrint` | `u32 argc, argc × u32 r` | print each `R[r]` with a trailing space, then `"\n"` and `endl` (a blank line) |
| `kReturn` | `u32 r` | `acc = R[r]`; return `acc` |
| *any other byte* | – | **ignored** (`default: break`); execution continues with the next byte |

### Context chain at runtime

For the closure example (`n` creates a context holding `a`, then returns `u`):

```
call n  (parentContext = null)
  CreateFunctionContext 1 → ctxN { slots=[undefined], parent=null };  frame.ownContext = ctxN
  StaCurrentContextSlot 0 → ctxN.slots[0] = 90
  CreateClosure u, ctxN   → Closure{0, ctxN}
  Return closure

call u  (parentContext = ctxN)
  StaContextSlot 0 0      → ctxN.slots[0] = 7      (depth 0 = parentContext)
  LdaContextSlot 0 0      → 7
```

Calling `n()` twice creates **two independent** contexts, so each returned `u`
has its own `a`. This is correct closure semantics.

## Gaps

| Missing | Effect |
|---------|--------|
| `kLdaConstant` | strings and floats load nothing; the following u32 operand bytes are then executed as opcodes |
| `kLdaTrue/False/Null/Undefined` | the accumulator keeps its previous value, so a register may get a stale value |
| Jumps / tests | none, which matches Turbine emitting none |
| Arguments | `call()` drops `args` |
| Bounds checks | a module without `kReturn` reads past `code` (undefined behaviour) |
| Constant pool | never read |
| Exceptions | `VMException` is declared but never thrown or caught |
