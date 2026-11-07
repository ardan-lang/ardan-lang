# 3. The IR model

This chapter describes the **data structures** of Ardan's intermediate
representation: what an IR program is made of, what every opcode means, and
which invariants producers and consumers rely on.
[04-ir-builder.md](04-ir-builder.md) explains how the AST is translated into it.

| File | Contents |
|------|----------|
| `ardan-lang/IR/ir/IRModule/IRModule.hpp` | `IRModule` |
| `ardan-lang/IR/ir/IRFunction/IRFunction.hpp` | `IROp`, `IRInstruction`, `BasicBlock`, `IRFunction` |
| `ardan-lang/IR/ir/IRValue/IRValue.hpp` | `IRValue`, `IRType` (+ unused `IRValueType`) |

---

## 3.1 Design in one paragraph

The IR is a **control-flow graph (CFG) of basic blocks** per function. Each
block holds a linear list of three-address instructions. Every instruction
reads zero or more **virtual registers** (`IRValue`s) and writes at most one.
Each virtual register is created exactly once, by `createTemp()`, so values
are in **SSA form** (static single assignment). Where control flow merges,
**`Phi`** instructions select a value per predecessor. Variables captured by
nested functions do not live in registers; they live in heap-allocated
**contexts**, accessed with explicit `*ContextSlot` instructions, the same
strategy V8's Ignition uses.

```
IRModule
 └── functions : vector<unique_ptr<IRFunction>>        (inner functions first, entry last)
      └── IRFunction  name, entry_point
           └── blocks : vector<unique_ptr<BasicBlock>> (blocks[0] is the function's entry)
                └── BasicBlock  name, predecessors, successors, terminator
                     └── instructions : vector<IRInstruction>
                          └── IRInstruction  op, result, operands, immediate,
                                             targets, childFunction, contextSlot, contextDepth
                               └── IRValue (shared_ptr)  name "%N", type
```

---

## 3.2 `IRModule`

```cpp
struct IRModule {
    std::vector<std::unique_ptr<IRFunction>> functions;
};
```

The compilation unit. It owns every `IRFunction`, flat, with no nesting, even
for nested source functions. **Order is completion order**: a function is
appended when its body has been fully built. Inner functions therefore come
before outer ones, and the top-level program, flagged `entry_point`, is always
last.

For

```js
function n() { function u() {} }
```

the order is `[u, n, <entry>]`. Turbine uses the vector index as the
function's runtime identity (see `Closure` below and
[06-turbine.md](06-turbine.md#function-indices)).

## 3.3 `IRFunction`

```cpp
class IRFunction {
public:
    std::string name;                                  // "" for the top-level program
    std::vector<std::unique_ptr<BasicBlock>> blocks;   // owning
    BasicBlock* entry = nullptr;                       // NOT set by the builder; use blocks[0]
    bool entry_point = false;                          // true only for the top-level program
};
```

- `blocks` is in **creation order**. Turbine emits blocks in exactly this
  order, so creation order is also code layout order.
- The `entry` pointer is never assigned. Consumers treat `blocks[0]` as the entry.
- No parameter list exists yet (see [known issues](09-known-issues.md)).

## 3.4 `BasicBlock`

```cpp
class BasicBlock {
public:
    std::string name;                       // prefix + global counter, e.g. "if.then1"
    std::vector<IRInstruction> instructions;
    std::vector<BasicBlock*> successors;    // CFG out-edges
    std::vector<BasicBlock*> predecessors;  // CFG in-edges (index matters for Phi!)
    IRInstruction* terminator = nullptr;    // points INTO instructions
};
```

### Naming

Block names are `prefix + blockId`, where `blockId` is a counter on the
builder that is **shared across all functions**, so names are unique
module-wide:

| Prefix | Created by |
|--------|------------|
| `entry` | top-level program |
| `<function name>` | function body entry, e.g. `n1` |
| `if.then`, `if.else`, `merge.block` | `if` |
| `while.header`, `while.body`, `merge.block` | `while` |
| `for.header`, `for.body`, `merge.block` | `for` |

### Edges

`successors` and `predecessors` are plain pointers into the owning function's
`blocks`. They are maintained manually by the builder. **The two lists are not
guaranteed to mirror each other** (for example, loop headers get no
predecessors). Consumers that need a consistent CFG should rebuild one list
from the other.

**Predecessor order is significant.** A `Phi` in block `B` has one operand per
entry of `B->predecessors`, in the same order:

```
Phi.operands[k]  is the value flowing in from  B->predecessors[k]
```

### `terminator`

A raw pointer to the block's last control instruction (`If` / `Return`).
Because it points into a `std::vector`, **any later `push_back` on that block
invalidates it**. It is informational only; no consumer dereferences it today.
Turbine instead assumes "the last instruction is the terminator".

## 3.5 `IRInstruction`

```cpp
struct IRInstruction {
    IROp op;
    std::vector<shared_ptr<IRValue>> operands;   // inputs (virtual registers)
    std::string label;                           // debug: source variable name (Phi)
    shared_ptr<IRValue> result;                  // output register, or nullptr
    R immediate;                                 // literal payload (numbers, strings)

    std::vector<BasicBlock*> targets;            // branch targets (If)
    IRFunction* childFunction = nullptr;         // Closure: which function
    int contextSlot = -1;                        // *ContextSlot: slot index; CreateContext: slot COUNT
    int contextDepth = 0;                        // LoadContextSlot / StoreContextSlot: hops up the chain
};
```

Fields are a union-by-convention: each opcode uses a subset. The full matrix
is in [§3.8](#38-opcode-reference).

### Textual notation

The repository has no IR printer. These docs use this notation, produced by a
small debug driver:

```
%7 = Multiply %8 %9                    result = Op operand operand
StoreContextSlot %4 slot=0 depth=0     no result; context fields shown
%10 = Closure %1 fn='u'                childFunction shown as fn=
%1 = Phi %0 %4 ; i                     label after ';'
If %2 -> while.body2 -> merge.block3   targets after '->'
```

`Constant`, `HeapNumber` and `StringConstant` carry their literal in
`immediate`, which is not shown inline unless noted.

## 3.6 `IRValue`: virtual registers

```cpp
struct IRValue {
    std::string name;   // "%0", "%1", …   (empty for placeholders, see below)
    IRType type;
};

enum class IRType { Void, I32, F64, Bool, String, Object, Function, Ptr,
                    Number, Array, Any, Null, Undefined };
```

- An `IRValue` is **identified by pointer identity**, not by name. Turbine maps
  `IRValue*` to a physical register (`registerOf`). Two distinct `IRValue`
  objects with the same name are different registers.
- Names come from one counter, `temp`, shared across all functions of a build.
  Numbering is therefore global and monotonic, which keeps IR dumps
  unambiguous. It also means a result can have a **lower number than its
  operands**: `visitBinary` allocates its result before visiting the operands.
- `IRType` is a **static hint** set at creation:

  | Producer | `IRType` |
  |----------|----------|
  | numeric literal | `Number` |
  | string literal | `String` |
  | `true` / `false` | `Bool` |
  | `null` / `undefined` | `Null` / `Undefined` |
  | function / closure | `Function` |
  | binary op, call, context load, `CreateContext` | `Any` |
  | `Phi` | type of the incoming value at creation |

  No pass reads `type` yet except the disabled `ConstantFold`.

### Placeholder values

Two places create `IRValue`s that are **not** produced by any instruction:

| Where | Value | Meaning |
|-------|-------|---------|
| `visitVariable` with no initializer (`let x;`) | `make_shared<IRValue>()`, empty name, **uninitialised `type`** | "undefined" |
| `build()` end of program | `{}` (a **null** `shared_ptr`) bound to the name `undefined` | operand of the final `Return` |

In Turbine both still get a physical register via `regFor` and are never
written, so they read as the VM's default `Value`, which prints `undefined`.
Other operand payloads are wrapped in `IRValue`s for readability only:
`True`/`False` carry a dummy operand named `"1"`/`"0"`, and `StringConstant`
carries an operand whose `name` is the string text. Turbine ignores these
operands.

> `IRValueType` (the first enum in `IRValue.hpp`) is unused by the live IR.

## 3.7 SSA, φ-functions and their lowering

The builder keeps SSA by never mutating a register. Assigning to a local
variable just rebinds the variable's name to a different `IRValue` in the
symbol table (see [04 §3](04-ir-builder.md#3-scopes-and-the-symbol-table)).
At join points:

```
            entry0
           x0 = …
         /         \
   if.then1        if.else2
   x1 = 5          x2 = 6
         \         /
        merge.block3
        x3 = Phi x1 x2     ; predecessors = [if.then1, if.else2]
```

Phis are **not** executed by the VM. Turbine's `resolvePhis` removes them and
inserts a copy `dst ← operand[k]` at the end of each predecessor `k`, before
that block's last instruction. This is the classic "SSA destruction by copy
insertion". See [06-turbine.md §φ resolution](06-turbine.md#phi-resolution).

## 3.8 Opcode reference

`IROp` declares 70+ opcodes, modelled on V8 TurboFan node types. The tables
below cover only those the **builder emits today** and say what each stage does
with them. The remaining enumerators (`Start`, `End`, `Region`, `Loop`, `Merge`,
`Throw`, `EffectPhi`, `Parameter`, `Neg`, `Not`, `Load`, `Store`,
`LoadProperty`, `StoreProperty`, `LoadElement`, `StoreElement`, `CallBuiltin`,
`NewObject`, `NewArray`, `ToBoolean`, `ToNumber`, `ToString`, `CheckType`,
`CheckBounds`, `Guard`, `FrameState`, `StateValues`, `Checkpoint`,
`Projection`) are reserved and never emitted.

Legend for the last two columns: ✅ handled · ⚠️ handled but broken · ❌ silently dropped.

### Constants

| Op | result | operands | other fields | Semantics | Turbine | VM |
|----|--------|----------|--------------|-----------|---------|----|
| `Zero` | `%r : Number` | – | `immediate` = literal | `%r = 0` | ✅ `LdaZero` | ✅ |
| `Constant` | `%r : Number` | – | `immediate` = literal (an `R`) | small integer (Smi range ±2³⁰) | ⚠️ `LdaSmi`, but only if `immediate` holds a `short` | ⚠️ sign is lost |
| `HeapNumber` | `%r : Number` | – | `immediate` | float, or integer outside Smi range | ⚠️ `LdaConstant` + pool | ❌ |
| `StringConstant` | `%r : String` | `[text]` (dummy) | `immediate` = `string` | string literal | ⚠️ `LdaConstant` + pool | ❌ |
| `True` / `False` | `%r : Bool` | `["1"]` / `["0"]` (dummy) | – | boolean literal | ✅ `LdaTrue/False` | ❌ |
| `Null` / `Undefined` | `%r` | – | – | `null` / `undefined` | ✅ `LdaNull/Undefined` | ❌ |

How a numeric literal is classified (`visitNumericLiteral`):

```
NumberType::NONE                                  → Zero
integer types, value == 0                         → Zero
integer types, -2^30 ≤ value ≤ 2^30-1 (IsSmi)     → Constant
integer types, otherwise                          → HeapNumber
FLOAT / DOUBLE / LONG_DOUBLE                      → HeapNumber
```

### Arithmetic, bitwise, comparison, logical

All are produced by `visitBinary` with the same shape:

```
%r : Any = <Op> %lhs %rhs
```

| Source operator | IROp | Turbine |
|-----------------|------|---------|
| `+` `-` `*` | `Add` `Subtract` `Multiply` | ✅ `kAdd` / `kSub` / `kMul` |
| `/` `%` `**` | `Divide` `Modulo` `Power` | ❌ |
| `==` `===` `!=` `!==` | `Equal` `StrictEqual` `NotEqual` `StrictNotEqual` | ❌ |
| `<` `<=` `>` `>=` | `LessThan` `LessThanOrEqual` `GreaterThan` `GreaterThanOrEqual` | ❌ |
| `&&` `\|\|` `??` | `LogicalAnd` `LogicalOr` `NullishCoalescing` | ❌ |
| `&` `\|` `^` `<<` `>>` `>>>` | `BitAnd` `BitOr` `BitXor` `ShiftLeft` `ShiftRight` `UnsignedShiftRight` | ❌ |

`&&`, `||` and `??` are emitted as **eager** binary ops: both sides are always
evaluated. Short-circuit semantics would need control flow, as with `If`.

### Control flow

| Op | result | operands | other fields | Semantics |
|----|--------|----------|--------------|-----------|
| `If` | – | `[%cond]` | `targets = [whenTrue, whenFalse]` | conditional branch; terminates the block |
| `Phi` | `%r` | one per predecessor | `label` = variable name (loops) | SSA merge |
| `Return` | – | `[%value]` or `[]` | – | return from function |

Turbine lowers `If` to **nothing** (❌), so all blocks run straight through in
layout order. See [09-known-issues.md](09-known-issues.md).

### Calls and builtins

| Op | result | operands | Semantics | Turbine / VM |
|----|--------|----------|-----------|--------------|
| `Call` | `%r : Any` | `[%callee, %arg0, %arg1, …]` | call a closure with `undefined` receiver | ✅ `kCallUndefinedReceiver` (args are passed but ignored by the callee) |
| `Print` | – | `[%arg0, …]` | intrinsic for `print(...)` | ✅ `kPrint` |

`print` is recognised **syntactically**: any `CallExpression` whose callee is
the identifier `print` becomes `Print`, regardless of scope.

### Closures and contexts

| Op | result | operands | fields | Semantics |
|----|--------|----------|--------|-----------|
| `CreateContext` | `%ctx : Any` | – | `contextSlot` = **number of slots** | allocate this function's context, parent = the closure's captured context; becomes the frame's *own* context |
| `Closure` | `%f : Function` | `[]` or `[%ctx]` | `childFunction` | create a function value capturing `%ctx` (the innermost context in scope at the definition site) |
| `LoadCurrentContextSlot` | `%r : Any` | – | `contextSlot` | read slot of the frame's **own** context |
| `StoreCurrentContextSlot` | – | `[%v]` | `contextSlot` | write slot of the frame's own context |
| `LoadContextSlot` | `%r : Any` | – | `contextSlot`, `contextDepth` | read slot of the context `depth` hops above the frame's **parent** context |
| `StoreContextSlot` | – | `[%v]` | `contextSlot`, `contextDepth` | write, same addressing |

Runtime context chain for a call:

```
frame.ownContext ──parent──▶ frame.parentContext ──parent──▶ … ──▶ nullptr
   (CreateContext)              (closure.capturedContext)
   *CurrentContextSlot          *ContextSlot depth=0          depth=1 …
```

All six are lowered by Turbine and executed by DeathStar. The depth arithmetic
is wrong when a function both owns a context and touches an outer one; see
[04 §8.4](04-ir-builder.md#84-depth-numbering-caveat).

## 3.9 Invariants (current and intended)

| # | Invariant | Holds today? |
|---|-----------|--------------|
| I1 | Every `IRValue` with a non-empty name is the `result` of exactly one instruction | ✅ (except placeholders) |
| I2 | Every use is dominated by its definition | ⚠️ broken when a variable is assigned inside a block body (assignment is dropped; see [04 §3](04-ir-builder.md#3-scopes-and-the-symbol-table)) |
| I3 | `Phi` instructions appear first in their block | ⚠️ `for.header` places `If` before its Phis |
| I4 | `len(Phi.operands) == len(block.predecessors)` | ⚠️ loop headers have 0 predecessors |
| I5 | Each block ends in exactly one terminator (`If`/`Return`) | ❌ fall-through blocks have none; Turbine treats whatever is last as terminator |
| I6 | No block is empty | ❌ `if.then` for `{}` is empty, and Turbine underflows on empty blocks |
| I7 | `functions.back()` is the entry point | ✅ |
