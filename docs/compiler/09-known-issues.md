# 9. Status and known issues

Every item below was **reproduced** against the current source, using a build
that links the project's own `.cpp` files. Each repro is a complete `.ardan`
program.

## What works end to end today

- `let`/`var`/`const` with numeric literals in −32768…32767 (stored as `short`)
- `+`, `-`, `*` on those values
- `print(...)` of numbers
- Function declarations **without parameters**, calls, and `return`
- Closures capturing outer `let` variables, including mutation from the inner
  function (the [worked example](08-worked-example.md))

## Issues by stage

### Build

| # | Issue | Repro / evidence | Fix |
|---|-------|------------------|-----|
| B1 | `cmake --build` fails: `ardan-lang/IR/IRBuilderVisitor-2.cpp` line 2 is `/  IRBuilderVisitor.cpp` (single slash) and the root glob compiles it | `cmake -S . -B build && cmake --build build` → `error: expected unqualified-id` | delete the file, or fix the comment, or exclude it in `CMakeLists.txt` |
| B2 | The `"ardan-lang/*.cpp"` glob also picks up `ardan-lang/v8/main.cpp`, giving a duplicate `_main` at link time | link error `duplicate symbol '_main'` | `list(FILTER SRC_FILES EXCLUDE REGEX ".*/v8/.*")` |

### Parser

| # | Issue | Repro |
|---|-------|-------|
| P1 | `if` / `while` bodies must be braced | `if (x < 2) x = 5;` → `Parse error: expected Expected '{'` |

### IR builder

| # | Issue | Repro | Observed | Expected |
|---|-------|-------|----------|----------|
| IR1 | Assignment inside any `{}` body is lost (`store` rebinds in the innermost scope) | `let x = 1; if (x < 2) { x = 5; } print(x);` | `1` | `5` |
| IR2 | Function parameters are never declared | `function f(a) { return a; } print(f(3));` | `runtime_error: Undefined variable` | `3` |
| IR3 | Unary minus, `!`, `++`/`--` are unimplemented | `let y = -3;` | `bad_variant_access` | `-3` |
| IR4 | An owner function assigning its own captured variable emits `StoreContextSlot` (parent) instead of `StoreCurrentContextSlot` | `function n(){ let a=1; a=2; function u(){ return a; } return u; } print(n()());` | segfault (exit 139) | `2` |
| IR5 | Context depth is off by one when a function owns a context and reads a grand-parent context | 3-level nesting | wrong slot / crash | – |
| IR6 | `while` back-edge is a conditional on the stale test; loop headers have no predecessors | `let i=0; while (i<3) { i = i + 1; }` | segfault (in Turbine) | `3` |
| IR7 | `for` evaluates the test once, before the header; header Phis come after the branch | see [04 §7](04-ir-builder.md#7-for) | – | – |
| IR8 | Capture analysis only sees `var/let/const` initialisers and `return` expressions, inside direct child `function` declarations | inner function that only does `print(a)` | `Undefined variable` | works |
| IR9 | No implicit `return` at the end of a function | `function f() { print(1); } f(); print(2);` | prints `1` repeatedly as the VM runs into garbage bytes, then segfaults | `1`, `2` |
| IR10 | `x = …` used as an expression returns `true` | `let y = (x = 1);` | `bad_variant_access` | – |
| IR11 | Compound assignments are dropped | `x += 1;` | no effect | – |

### Pharaoh

| # | Issue | Repro |
|---|-------|-------|
| O1 | `DeadCode` dereferences a null `result` | enable `pharoah.start(...)` with any program containing `print` → segfault |
| O2 | Passes are block-local and ignore side effects | – |
| O3 | `ConstantFold` produces a `double` immediate, but Turbine expects `short` | – |

### Turbine

| # | Issue | Repro | Observed |
|---|-------|-------|----------|
| T1 | `If` emits nothing, so **both branches always run** | `let x=1; if (x<2) { print(10); } else { print(20); }` | prints `10` **and** `20` |
| T2 | Comparisons, `/`, `%`, `**`, logical and bitwise ops are dropped | `let b = 1 && 0; print(b);` | `undefined` |
| T3 | `Constant` uses `get<short>` | `let x = 100000; print(x);` | `bad_variant_access` (abort) |
| T4 | Constant-pool index is off by one (`size-1` taken before `push_back`) | `print("hi")` | `undefined` (with VM gap V1) |
| T5 | Empty block: `size()-1` underflow | `let x=1; if (x<2) { } print(x);` | segfault |
| T6 | `resolvePhis` skips consecutive Phis and indexes `predecessors` without bounds checks | while loop with two variables | – |
| T7 | `Return` with no value emits no operand | `function f(){ return; }` | VM reads the next bytes as the operand |
| T8 | `LdaZero` gets an operand the VM does not consume | – | harmless by accident (decodes as 4× `kWide`) |

### DeathStar

| # | Issue | Observed |
|---|-------|----------|
| V1 | `kLdaConstant` not implemented | strings and floats print `undefined` |
| V2 | `kLdaTrue/False/Null/Undefined` not implemented | `let t = true; print(t);` → `undefined` |
| V3 | `kLdaSmi` stores an unsigned u32 | negative small integers become huge positive numbers |
| V4 | Call arguments are discarded | – |
| V5 | No `ip` bounds check; unknown opcodes are skipped silently | silent mis-decoding |

## Suggested order of work

1. **Build hygiene:** B1, B2, so `cmake` works out of the box.
2. **CFG correctness in the builder:** IR1, IR6, IR7, and I5 (every block ends in a terminator).
3. **Jumps in Turbine and VM:** T1, plus `Jump`, `JumpIfFalse` and `Test*` bytecodes. This
   unlocks `if`/`while`/`for` semantics.
4. **Constants:** T3, T4, V1, V2, V3.
5. **Parameters:** IR2 and V4, which are needed for any real function.
6. **Closures hardening:** IR4, IR5, IR8.
7. **Remaining expressions:** IR3, IR10, IR11, then member, array and object
   support.
8. **Pharaoh:** a whole-function DCE with side-effect awareness, then
   constant folding.

Adding an IR printer (for example `--dump-ir`) would make every step above
easier to test. The listings in these docs follow the notation described in
[03 §3.5](03-ir-model.md#textual-notation).
