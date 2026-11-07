# 5. Optimizer — Pharaoh (`Pharoah`)

**`IRModule` → `IRModule` (in place)**

| File | Role |
|------|------|
| `ardan-lang/IR/optimizer/Pharoah/Pharoah.hpp/.cpp` | Pass manager and passes |

> **Status: disabled.** The call in `test_ir()` is commented out. Enabling it
> crashes on any program that contains `Print`, `Return`, `If` or a store
> (exit code 139). See [Dead code](#deadcode) below.

## Architecture

```cpp
class OptimizationPass {
public:
    virtual void run(std::vector<IRInstruction>& instructions) = 0;
};

class Pharoah {
    vector<OptimizationPass*> passes = {
        // new ConstantFold(),
        new DeadCode(),
    };
public:
    void start(IRModule& irModule);
};
```

`Pharoah::start` is a triple loop: every function, then every block, then
every pass, and each pass runs on **one block's instruction vector**:

```cpp
for (fn : irModule.functions)
  for (block : fn->blocks)
    for (pass : passes)
      pass->run(block->instructions);
```

Every pass is therefore **block-local**. A pass cannot see uses in other
blocks, Phi operands that refer to it, or other functions.

## `ConstantFold` (disabled in the pass list)

1. Records `result->name → immediate` for every `Constant` in the block.
2. For every `Add` whose operands are both typed `IRType::Number`, it reads
   their constants, computes `lhs.numberValue + rhs.numberValue`, and rewrites
   the instruction in place into `Constant` with that `immediate`.

Problems to fix before enabling it:

- Literal operands are typed `Number`, but **binary results are `Any`**, so
  folding never chains (`1 + 2 + 3` folds only the inner add).
- If an operand is a `Zero`/`HeapNumber` (not recorded), `constants[name]`
  default-constructs an `R` (`monostate`) and `toValue` of that is not a number.
- The folded `immediate` is a `double`, but Turbine lowers `Constant` with
  `get<short>(immediate)`, which throws `bad_variant_access`. A folded value
  must be either re-classified (`Zero`/`Constant`/`HeapNumber`) or stored as `short`.
- Only `Add` is handled.

## `DeadCode`

Intended: remove instructions whose result is never used.

```cpp
for inst in block: use[inst.result->name] = 0;     // ⚠️ result may be nullptr
for inst in block: for op in inst.operands: use[op->name]++;  // ⚠️ operand may be nullptr
keep inst iff use[inst.result->name] > 0
```

Defects:

| Defect | Consequence |
|--------|-------------|
| Dereferences `result` for instructions without one (`Print`, `Return`, `If`, `Store*ContextSlot`) | segfault |
| Even with a null check, it would delete every instruction without a result | would delete all side effects and terminators |
| Block-local use counts | deletes values used only in another block, or only by a Phi in a successor |
| Single pass | does not cascade (dead `a` feeding dead `b`) |
| Ignores side effects | would delete an unused `Call` |

A correct version:

```text
worklist DCE over the whole function:
  live := { inst | inst has side effects: Print, Call, Return, If, Store*, CreateContext, Closure? }
  for inst in live: mark operands' defining instructions live (transitively)
  delete everything not marked
```

Here `CreateContext` must stay live while any context access exists, because
it sets the frame's own context as a side effect.

## Roadmap (from the header comments)

The header lists the intended passes: constant folding and propagation, copy
propagation, DCE / ADCE, CSE / GVN, type propagation, loop optimisations (LICM,
strength reduction, induction variables), inlining, peephole, and register
allocation. Most need a whole-function view and a correct CFG. In practice,
the builder fixes in [04-ir-builder.md](04-ir-builder.md) (predecessors,
terminators, assignment rebinding) come before most of these passes.
