# 6. Turbine — bytecode lowerer

**`IRModule` → `Compiled`**

| File | Role |
|------|------|
| `ardan-lang/Turbine/Turbine.hpp` | `Bytecode` enum, container structs, `AssemblyLine`, `Turbine` |
| `ardan-lang/Turbine/Turbine.cpp` | Lowering |

Turbine turns each `IRFunction` into a flat byte array for an
**accumulator + register** machine, modelled on V8 Ignition. Arithmetic reads
its operands from registers and writes the accumulator. `Star r` / `Ldar r`
move values between the accumulator and register `r`.

## Output containers

```cpp
struct ConstantPool   { vector<Value> constants; };

struct BytecodeModule {            // one per IRFunction
    string id;                     // IRFunction::name
    vector<uint8_t> code;
    ConstantPool constantPool;
};

struct Compiled {
    vector<BytecodeModule> modules;   // same order as IRModule::functions
    int entry_index;                  // index of the entry_point function
};
```

## `AssemblyLine`: the driver

```cpp
Compiled AssemblyLine::start(IRModule& irModule) {
    // 1. function identity: IRFunction* → index in irModule.functions
    unordered_map<IRFunction*, int> functionIndex;
    // 2. one fresh Turbine per function
    for each function i:
        Turbine turbine(&functionIndex);
        if (fn->entry_point) compiled.entry_index = i;
        BytecodeModule m = turbine.start(fn);
        m.constantPool = turbine.constantPool;
        modules.push_back(m);
}
```

<a id="function-indices"></a>
**Function indices.** `CreateClosure` encodes the callee as an index into
`Compiled::modules`. That index equals the position in `IRModule::functions`,
so the closure example compiles to `u = 0`, `n = 1`, `entry = 2`.

## Register allocation

```cpp
int Turbine::regFor(const shared_ptr<IRValue>& v) {
    auto it = registerOf.find(v.get());
    if (it != registerOf.end()) return it->second;
    int r = registerOf.size();          // next unused number
    registerOf[v.get()] = r;
    return r;
}
```

- **One register per SSA value**, numbered in order of **first appearance
  during lowering** (not IR name order). There is no reuse or liveness.
- Registers are per function (each `Turbine` is fresh).
- A null `shared_ptr` (the final `Return <null>`) is keyed by `nullptr` and
  gets its own register, which is never written, so it reads as `undefined`.

## Encoding

- **Opcode:** 1 byte, the `Bytecode` enumerator value.
- **Every operand:** 4 bytes, `uint32_t` little-endian (`emitU32`). There
  are no narrow operands; `kWide`/`kExtraWide` exist in the enum but are unused.

Numeric values of the opcodes Turbine emits:

| Value | Bytecode | Value | Bytecode |
|------:|----------|------:|----------|
| 2 | `kLdaZero` | 31 | `kLdaCurrentContextSlot` |
| 3 | `kLdaSmi` | 33 | `kStaContextSlot` |
| 4 | `kLdaUndefined` | 34 | `kStaCurrentContextSlot` |
| 5 | `kLdaNull` | 37 | `kAdd` |
| 6 | `kLdaTrue` | 38 | `kSub` |
| 7 | `kLdaFalse` | 39 | `kMul` |
| 8 | `kLdaConstant` | 116 | `kCallUndefinedReceiver` |
| 9 | `kLdar` | 133 | `kCreateClosure` |
| 10 | `kStar` | 135 | `kCreateFunctionContext` |
| 29 | `kLdaContextSlot` | 144 | `kReturn` |
| | | 152 | `kPrint` (declared after `kLast`) |

## Lowering table

`r(x)` means `regFor(x)`, encoded as u32.

| IR | Bytes emitted | Notes |
|----|---------------|-------|
| `%d = Zero` | `LdaZero 0` · `Star r(%d)` | ⚠️ emits an operand `0` that the VM does not consume; its 4 zero bytes then decode as `kWide` no-ops |
| `%d = Constant` | `LdaSmi get<short>(imm)` · `Star r(%d)` | ⚠️ throws `bad_variant_access` unless the literal was stored as `short` (−32768…32767) |
| `%d = HeapNumber` | `LdaConstant idx` · `Star r(%d)` | ⚠️ `idx = size-1` is computed **before** the push (off by one; the first constant gets `0xFFFFFFFF`) |
| `%d = StringConstant` | `LdaConstant idx` · `Star r(%d)` | same off-by-one |
| `%d = True/False/Null/Undefined` | `LdaTrue/…` · `Star r(%d)` | |
| `%d = Add/Subtract/Multiply %a %b` | `Add r(%a) r(%b)` · `Star r(%d)` | |
| `%d = Call %f %a…` | `CallUndefinedReceiver r(%f) argc r(%a)…` · `Star r(%d)` | |
| `Print %a…` | `Print argc r(%a)…` | |
| `%d = Closure [%ctx] fn=F` | `CreateClosure index(F) (r(%ctx) or 0xFFFFFFFF)` · `Star r(%d)` | |
| `%d = CreateContext slot=n` | `CreateFunctionContext n` · `Star r(%d)` | |
| `%d = LoadCurrentContextSlot s` | `LdaCurrentContextSlot s` · `Star r(%d)` | |
| `%d = LoadContextSlot s, depth` | `LdaContextSlot depth s` · `Star r(%d)` | operand order: depth first |
| `StoreCurrentContextSlot %v s` | `Ldar r(%v)` · `StaCurrentContextSlot s` | |
| `StoreContextSlot %v s, depth` | `Ldar r(%v)` · `StaContextSlot depth s` | |
| `Return %v` | `Return r(%v)` | ⚠️ `Return` with no operand emits no operand bytes, but the VM always reads one |
| `If` | *nothing* | ❌ no jumps are generated |
| everything else | *nothing* | ❌ silently dropped (`default: break`) |

## Function lowering

```cpp
void Turbine::lowerFunction(IRFunction* fn) {
    resolvePhis(fn->blocks);              // SSA destruction
    for (block : fn->blocks) lowerBlock(block);   // straight layout, creation order
}

void Turbine::lowerBlock(BasicBlock* b) {
    for (j = 0; j < b->instructions.size() - 1; j++)   // all but last
        lowerInstruction(b->instructions[j]);
    flushPendingPhis(b);                               // phi copies
    lowerInstruction(b->instructions.back());          // "terminator"
}
```

- Blocks are concatenated in creation order with **no jumps between them**.
  Every block runs, so both arms of an `if` execute and loops run their body
  once.
- ⚠️ `instructions.size() - 1` is `size_t`. On an **empty block** it wraps to
  `SIZE_MAX`, and `back()` is undefined behaviour. An empty `{}` body segfaults.

<a id="phi-resolution"></a>
## φ resolution

```cpp
void Turbine::resolvePhis(blocks) {
    for (block : blocks)
        for (i = 0; i < block->instructions.size(); i++) {
            inst = block->instructions[i];
            if (inst.op != Phi) continue;
            for (k : operands)
                pendingPhis[block->predecessors[k]].push_back({ operands[k], inst.result });
            block->instructions.erase(begin + i);       // ⚠️ i is not decremented
        }
}

void Turbine::flushPendingPhis(BasicBlock* pred) {
    for ({from, to} : pendingPhis[pred]) { Ldar r(from); Star r(to); }
}
```

Each predecessor ends with `to ← from` copies placed **just before its last
instruction**. This is standard copy insertion, but:

| Issue | Effect |
|-------|--------|
| `erase` then `i++` | two consecutive Phis: the second is skipped and stays in the block, and `lowerInstruction` then drops it |
| `predecessors[k]` without a bounds check | loop-header Phis (no predecessors) are out of bounds: segfault |
| copies placed before the last instruction even when it is not a terminator | if that last instruction defines `from`, the copy reads a stale register |
| parallel-copy semantics ignored | Phis that swap values (`a,b = b,a`) are sequentialised incorrectly (lost-copy / swap problem) |

## What a correct lowering needs

1. **Jumps.** Record each block's start offset, emit
   `JumpIfToBooleanFalse <offset>` (or `JumpIfFalse`) for `If`, and an
   unconditional `Jump` for fall-through edges whose target is not the next
   block. Back-patch forward offsets.
2. **Comparisons.** Lower `LessThan` etc. to `TestLessThan r(a)` (V8 style:
   compare the accumulator with a register), then `Star`.
3. **Constants.** Classify by value, not variant alternative (`toValue(imm).numberValue`),
   and push to the pool *before* taking the index.
4. **Phis.** Iterate over Phis without skipping; require predecessors; emit
   copies before the terminator using a parallel-copy sequentialiser.
5. **Return.** Always emit an operand (`kLdaUndefined` + `Star` into a scratch
   register when there is no value).
