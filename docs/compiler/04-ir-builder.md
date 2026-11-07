# 4. IR Builder (`IRBuilderVisitor`)

**AST → `IRModule`**

| File | Role |
|------|------|
| `ardan-lang/IR/ir/IRBuilderVisitor/IRBuilderVisitor.hpp` | Class, `Scope`, `Context`, helper inlines |
| `ardan-lang/IR/ir/IRBuilderVisitor/IRBuilderVisitor.cpp` | All lowering rules |

`IRBuilderVisitor` implements both `ExpressionVisitor` and `StatementVisitor`.
It walks the AST once, top-down, appending instructions to a *current block*
of a *current function*. It resolves variables to SSA values as it goes, and
it decides closure-capture layout (contexts) per function before visiting that
function's body.

```cpp
IRBuilderVisitor builder;
builder.build(ast);           // fills builder.irModule
IRModule& m = builder.irModule;
```

---

## 1. Builder state

```cpp
IRModule irModule;                      // output

IRFunction* currentFunction;            // function receiving new blocks
BasicBlock* currentBlock;               // block receiving new instructions

int temp = 0;                           // next virtual register number  (%temp)
int blockId = 0;                        // next block-name suffix

vector<Scope>   scopes;                 // lexical symbol tables (innermost = back())
vector<Context> contexts;               // closure-context frames (innermost = back())
bool currentFunctionOwnsTopContextFrame;// does currentFunction own contexts.back()?
```

Supporting records:

```cpp
struct Scope {
    enum class Type { Global, Function, Block };
    Type type;
    Scope* parent;                                        // unused (always nullptr)
    unordered_map<string,int> locals;                     // unused
    unordered_map<string, shared_ptr<IRValue>> symbols;   // name → current SSA value
};

struct Context {
    unordered_map<string,int> slots;     // captured name → slot index
    shared_ptr<IRValue> contextValue;    // register holding the context (result of CreateContext)
};
```

Two separate structures track where a variable lives:

| Where a variable lives | Tracked in | Accessed with |
|------------------------|------------|---------------|
| Not captured by any inner function | `scopes[*].symbols` (name → `IRValue`) | nothing to emit: the value *is* the register |
| Captured by an inner function | `contexts[*].slots` (name → slot) | `Load/Store[Current]ContextSlot` |

## 2. Helpers

```cpp
shared_ptr<IRValue> createTemp(IRType t);      // new IRValue "%<temp++>"
BasicBlock* createBlock(const string& prefix); // new block "<prefix><blockId++>", appended to currentFunction->blocks
void emit(IRInstruction inst);                 // currentBlock->instructions.push_back(inst)
IROp getBinaryOp(const Token& op);             // TokenType → IROp, throws on unknown
```

`createBlock` appends to `currentFunction->blocks` immediately, so **block
order = creation order**. This order is also the final bytecode layout.

---

## 3. Scopes and the symbol table

### 3.1 Declaring: `declare(name, value, kind)`

```
if currentFunction owns contexts.back() and name ∈ contexts.back().slots:
    emit StoreCurrentContextSlot value  slot=…        // captured: goes to the heap
    return
switch kind:
    Let, Const → scopes.back().symbols[name] = value  // innermost scope
    Var        → nearest scope whose type != Block    // function/global hoisting target
```

`kind` comes from `getBindingKind(stmt->kind)`, which accepts both `"let"` and
the scanner's `"LET"`. Anything else is treated as `Const`.

### 3.2 Reading: `lookup(name)`

```
for scope in scopes, innermost → outermost:
    if name ∈ scope.symbols: return scope.symbols[name]          // no instruction emitted

for depth = 0 .. contexts.size()-1:                              // innermost context first
    ctx = contexts[size-1-depth]
    if name ∈ ctx.slots:
        if depth == 0 and currentFunctionOwnsTopContextFrame:
            emit %r = LoadCurrentContextSlot slot=…
        else:
            emit %r = LoadContextSlot slot=… depth=depth
        return %r

throw runtime_error("Undefined variable")
```

Consequences:

- Reading an **uncaptured** variable costs nothing; the expression evaluates
  directly to the register bound to the name.
- Reading a **captured** variable emits a fresh load **every time** it is read,
  because context slots can be changed by other closures.
- An unknown name, including globals and builtins other than `print`, is a
  **compile-time error**.

### 3.3 Assigning: `store(name, value)`

Called for `x = expr`:

```
for depth = 0 .. contexts.size()-1:
    if name ∈ contexts[size-1-depth].slots:
        emit StoreContextSlot value slot=… depth=depth
        return
scopes.back().symbols[name] = value          // rebind in the INNERMOST scope
```

> ⚠️ **Rebinding happens in `scopes.back()`, not in the scope that declared
> the variable.** Inside `if (c) { x = 5; }` the innermost scope is the
> `BlockStatement`'s scope, which is popped at `}`, so the new binding is lost.
> The merge-point Phi logic (§5) never sees a change, and `x` keeps its old
> value after the `if`. The same happens in `while` bodies. Because the parser
> requires braces, this affects **every** assignment inside a control-flow
> body. The fix is to walk `scopes` from innermost outwards and rebind in the
> first scope that already contains `name`.

### 3.4 Scope push/pop sites

| Construct | Pushes |
|-----------|--------|
| `build()` | `Global` |
| function body | the caller's scopes are saved and cleared, then one `Function` scope is pushed |
| `BlockStatement` | `Block` |
| `for (…)` | `Block` (for the `init` bindings) |

Functions **do not see enclosing `scopes`**: they are cleared on entry. An inner
function can reach an outer variable **only** through a context. That is why
capture analysis (§8) must be exact. A variable it misses produces
`Undefined variable` at compile time.

---

## 4. Expressions

Each rule shows the AST, then the emitted IR.

### 4.1 Literals

```js
0          →  %0 = Zero                   immediate=0
90         →  %0 = Constant               immediate=(short)90
2000000000 →  %0 = HeapNumber             immediate=(int)2000000000
1.5        →  %0 = HeapNumber             immediate=(float)1.5
"hi"       →  %0 = StringConstant "hi"    immediate="hi"
true       →  %0 = True  "1"
false      →  %0 = False "0"
null       →  %0 = Null
undefined  →  %0 = Undefined
```

### 4.2 Identifiers

`visitIdentifier` returns `lookup(expr->token.lexeme)` (§3.2).

### 4.3 Binary operators

```js
a * 2
```
```
%2 = Multiply %0 %1        ; result allocated FIRST, then lhs, then rhs
```

Exact order: `result = createTemp(Any)`, then `op = getBinaryOp(...)`, then
visit `left`, then visit `right`, then emit. Operands are evaluated
left-to-right, but the result register number is lower than theirs.

### 4.4 Assignment

`visitBinary` checks the 16 assignment token types first and routes them to
`emitAssignment`:

```cpp
value = visit(right)
if op == ASSIGN and left is IdentifierExpression:
    store(left.name, value)
return true                 // ⚠️ not a register, so `y = (x = 1)` throws bad_variant_access
```

Only plain `=` to a plain identifier is implemented. Compound assignments
(`+=`, …) evaluate the right-hand side and then **discard** it. Member targets
(`o.x = …`) are ignored.

### 4.5 Calls

```js
f(a, b)
```
```
%3 = Call %f %a %b
```

Order: visit the callee, allocate the result, then visit arguments
left-to-right.

`print(...)` is special-cased before the callee is resolved:

```js
print(a, 80)
```
```
%6 = Constant 80
Print %a %6               ; no result
```

`visitCall` returns `true` for `print`, so `print(...)` cannot be used as a
value.

---

## 5. `if` / `else`: diamonds and Phis

```js
if (test) { … } else { … }
```

Algorithm (`visitIf`):

1. Evaluate `test` into `%c` in the current block (`entryBlock`).
2. Create `if.then`, an optional `if.else`, and `merge.block`.
3. Append `If %c -> then -> (else or merge)` to `entryBlock`; set
   `entryBlock->successors` the same way.
4. Snapshot `before = scopes`.
5. Visit `consequent` in `then`, then record `thenExit = currentBlock` and
   `afterThen = scopes`.
6. If there is an `alternate`: reset `scopes = before`, visit it in `else`, and
   record `elseExit` and `afterElse`. Otherwise `afterElse = before`.
7. Link `thenExit → merge` and `elseExit → merge`. Set
   `merge.predecessors = [thenExit, elseExit or entryBlock]`.
8. Restore `scopes = before` and continue in `merge`.
9. **Phi insertion:** for each scope `i` and each `name` in `before[i]`, if
   `afterThen[i][name]` or `afterElse[i][name]` differs from `before[i][name]`,
   emit `%p = Phi afterThen[i][name] afterElse[i][name]` in `merge` and rebind
   `scopes[i][name] = %p`.

```
             ┌──────────────────────┐
             │ entry: %c = …        │
             │ If %c → then, else   │
             └───────┬──────┬───────┘
                     ▼      ▼
          ┌──────────────┐ ┌──────────────┐
          │ if.then      │ │ if.else      │
          └──────┬───────┘ └──────┬───────┘
                 ▼                ▼
             ┌──────────────────────┐
             │ merge.block          │
             │ %p = Phi %t %e       │
             └──────────────────────┘
```

Notes:

- Only variables that **existed before** the `if` can get a Phi. New
  declarations inside a branch go out of scope.
- Phi operand order is `[then, else-or-entry]`, matching `merge.predecessors`.
- The `then` and `else` blocks get **no explicit jump** to `merge`. The CFG
  edge exists, but nothing is emitted for it.
- In practice no Phi is produced today, because of the §3.3 rebinding bug.

Real output for `let x = 1; if (x < 2) { x = 5; } else { x = 6; } print(x);`:

```
entry0:  ; succs: if.then1 if.else2
  %0 = Constant            ; x = 1
  %2 = Constant            ; 2
  %1 = LessThan %0 %2
  If %1 -> if.then1 -> if.else2
if.then1:  ; succs: merge.block3
  %3 = Constant            ; 5 — bound to x in the block scope, then lost
if.else2:  ; succs: merge.block3
  %4 = Constant            ; 6 — same
merge.block3:  ; preds: if.then1 if.else2
  Print %0                 ; ⚠️ prints 1; expected 5
  Return <null>
```

## 6. `while`: loop headers and back-edge Phis

```js
while (test) { body }
```

Block layout: `entry → while.header → while.body`, and `while.header → merge.block`.

1. `entry.successors += header`.
2. In `header`, for **every** name visible in every scope, emit a
   provisional `%φ = Phi <current value>` (label = name) and rebind the name
   to `%φ`. Remember each Phi's index in `phiIndexForName`.
3. Evaluate `test` in `header` (it now reads the Phis), then emit
   `If %c -> body -> merge`.
4. Visit `body` in `body`.
5. Append the **back-edge** to `body`: `If %c -> merge -> body`.
6. For every name still in scope, append its current value to its header Phi.
   This becomes the back-edge operand.
7. Continue in `merge`.

```
            ┌───────────────┐
            │ entry         │
            └──────┬────────┘
                   ▼
            ┌────────────────────────────┐
            │ while.header               │◀───────────┐
            │ %φ = Phi %init, %next      │            │
            │ %c = test(%φ)              │            │
            │ If %c → body, merge        │            │
            └──────┬───────────────┬─────┘            │
                   ▼               ▼                  │
            ┌──────────────┐   ┌──────────┐           │
            │ while.body   │   │ merge    │           │
            │ …            │   └──────────┘           │
            │ If %c → merge, body  (back-edge, ⚠️)     │
            └──────┬───────┘                          │
                   └──────────────────────────────────┘ (intended)
```

Real output for `let i = 0; while (i < 3) { i = i + 1; } print(i);`:

```
entry0:  ; succs: while.header1
  %0 = Zero
while.header1:  ; succs: while.body2 merge.block3
  %1 = Phi %0 %1 ; i            ← back-edge operand is %1 itself (assignment lost, §3.3)
  %3 = Constant
  %2 = LessThan %1 %3
  If %2 -> while.body2 -> merge.block3
while.body2:  ; preds: while.header1
  %5 = Constant
  %4 = Add %1 %5                ← i + 1 computed, never rebound
  If %2 -> merge.block3 -> while.body2
merge.block3:  ; preds: while.header1
  Print %1
  Return <null>
```

Issues visible here:

| Issue | Effect |
|-------|--------|
| The back-edge is `If` on the **stale** `%c` with targets `[merge, body]` | The test is not re-evaluated; the edge should be an unconditional jump to `header` |
| The back-edge is appended to `bodyBlock`, not to `currentBlock` | With nested control flow in the body, the edge comes out of the wrong block |
| `header.predecessors` is never filled | Phi operand *k* has no matching predecessor. Turbine indexes `predecessors[k]` out of bounds and segfaults. |
| `body.successors` never contains `header` | The CFG has no cycle |
| Phis are created for every visible name, including functions | Harmless but noisy; a liveness check would prune them |

## 7. `for`

```js
for (init; test; update) { body }
```

1. Push a `Block` scope and visit `init` in the **current** block.
2. Evaluate `test` in the **current** block. ⚠️ This happens before the header
   exists, so the test is computed once, from the initial values.
3. Create `for.header`, `for.body` and `merge.block`.
4. Append `If %c -> body -> merge` to the header, **then** the Phis
   (invariant I3 is violated).
5. Visit `body`, then `update`, in `for.body`. Append the back-edge
   `If %c -> merge -> body`. Patch the Phi back-edge operands.
6. Pop the `for` scope and continue in `merge`.

Because `update` runs after the body's own `Block` scope is popped, an update
like `i = i + 1` *does* rebind `i` in the `for` scope, so the Phi gets the
right back-edge value:

```
entry0:  ; succs: for.header1
  %0 = Zero
  %2 = Constant
  %1 = LessThan %0 %2           ← test computed once, from %0
for.header1:  ; succs: for.body2 merge.block3
  If %1 -> for.body2 -> merge.block3
  %3 = Phi %0 %4 ; i            ← Phi AFTER the branch
for.body2:  ; preds: for.header1
  Print %3
  %5 = Constant
  %4 = Add %3 %5
  If %1 -> merge.block3 -> for.body2
merge.block3:  ; preds: for.header1
  Return <null>
```

---

## 8. Functions, closures and contexts

This is the most involved part of the builder. Ardan follows the Ignition
model:

> A local that is **captured by an inner function** is allocated in a
> heap *context* owned by the declaring function, not in a register. Each
> closure records the context that was current when it was created. At call
> time, that context becomes the callee's *parent context*.

### 8.1 `visitFunction` step by step

```
1.  Create IRFunction F(name = stmt.id).
2.  Save (currentFunction, currentBlock, scopes, ownsTopFrame); clear scopes.
3.  currentFunction = F; currentBlock = createBlock(stmt.id); push Function scope.

4.  Capture analysis:
      vars        = collectDeclaredNames(body)     // names declared by top-level `var/let/const` in F's body
      nested_vars = collectNestedVariables(body)   // free names of DIRECT child function declarations
      captured    = vars ∩ nested_vars

5.  If captured ≠ ∅:
      %ctx = CreateContext  slot=|captured|        // slot holds the COUNT here
      push Context{ slots = {name_i → i}, contextValue = %ctx }
      ownsTopFrame = true
    else:
      ownsTopFrame = false

6.  Visit each statement of the body.
7.  Pop the context (if pushed) and the scope; append F to irModule.functions.
8.  Restore the saved state.

9.  In the OUTER function:
      %f = Closure [contexts.back().contextValue if any]  childFunction=F
      declare(stmt.id, %f, Var)
```

Steps 7 and 9 explain the module order (inner first) and why the `Closure`
instruction comes *after* the inner function's IR exists.

### 8.2 Capture analysis

| Function | What it does | Limits |
|----------|--------------|--------|
| `collectDeclaredNames(body)` | names of `VariableStatement` declarations **directly** in the body block | ignores parameters, nested blocks, and function names |
| `collectNestedVariables(body)` | for each **direct** child `FunctionDeclaration`, runs `freeVariablesOf` on its body | ignores arrow functions, function expressions, and grandchildren |
| `freeVariablesOf(stmts, params)` | walks statements with a `boundStack` | only `VariableStatement` initialisers and `ReturnStatement` arguments are walked |
| `walkForFreeVars(Expression)` | `IdentifierExpression` → free if not bound; recurses into `BinaryExpression` | calls, members, unary, etc. are not traversed |

So in

```js
function n() {
    let a = 90;
    function u() { a = 7; print(a); return a * 2; }
    return u;
}
```

`a` is found only through `return a * 2` (`a = 7` is an `ExpressionStatement`
and `print(a)` is a call, and neither is walked). If `u` lacked that `return`,
`a` would not be captured. Compiling `u` would then throw
`Undefined variable`, because `u`'s scopes are cleared and `a` is not in any
context.

**Hardening suggestion:** replace the ad-hoc walker with a full
`ExpressionVisitor`/`StatementVisitor`-based free-variable pass that runs over
all nested function forms and also treats parameters as declarations.

### 8.3 The emitted IR for the closure example

```
function #0 'u'
  u2:
    %4 = Constant                          ; 7
    StoreContextSlot %4 slot=0 depth=0     ; a = 7      (n's context = u's parent)
    %5 = LoadContextSlot slot=0 depth=0    ; read a
    %6 = Constant                          ; 80
    Print %5 %6
    %8 = LoadContextSlot slot=0 depth=0    ; read a again (no caching)
    %9 = Constant                          ; 2
    %7 = Multiply %8 %9
    Return %7
function #1 'n'
  n1:
    %1 = CreateContext slot=1              ; 1 slot: a → 0
    %2 = Constant                          ; 90
    StoreCurrentContextSlot %2 slot=0      ; let a = 90 (captured → heap)
    %3 = LoadCurrentContextSlot slot=0
    Print %3
    %10 = Closure %1 fn='u'                ; u captures n's context
    Return %10
```

### 8.4 Depth numbering caveat

`lookup`/`store` count `depth` over `contexts` from the top **including the
current function's own context**. At runtime, `LdaContextSlot`/`StaContextSlot`
start from `frame.parentContext`, which **excludes** the own context. The two
schemes agree only when the current function owns no context. Otherwise:

| Situation | IR emits | Runtime does | Result |
|-----------|----------|--------------|--------|
| Owner reads its own captured var | `LoadCurrentContextSlot` | own context | ✅ |
| Owner **assigns** its own captured var (`a = 2` in `n`) | `StoreContextSlot depth=0` (`store` never uses the *Current* form) | writes to `parentContext`, which is `nullptr` at the top level | ❌ segfault |
| Owner reads a var from a grand-parent context | `LoadContextSlot depth=d` | walks `d` hops from the parent | ❌ off by one (should be `d-1`) |

Fix: in `store`, emit `StoreCurrentContextSlot` when
`depth == 0 && currentFunctionOwnsTopContextFrame`. In both `lookup` and
`store`, subtract one from `depth` when the current function owns the top
frame.

### 8.5 Parameters

`stmt->params` is never read. Parameters are not declared, so any reference to
one throws `Undefined variable`. The VM side is missing too: `call()` receives
`args` but does not place them anywhere. A complete design needs:

1. An `IROp::Parameter` (already in the enum) with `immediate = index`,
   declared at function entry, or stored into the context when captured.
2. A Turbine bytecode that reads `frame.arguments[i]`.
3. `DeathStar::call` filling `frame.arguments`.

---

## 9. `return` and program end

```js
return expr;   →   Return %expr
return;        →   Return            (no operands; see Turbine note)
```

`build()` finishes the top level with

```cpp
declare("undefined", {}, Var);              // binds the name to a NULL shared_ptr
emit(Return lookup("undefined"));           // Return <null>
```

Functions get **no implicit return**. A function body that falls off the end
runs past its bytecode in the VM.

## 10. Variable declarations

```js
let a = 90, b;
```

For each declarator: the value is `visit(init)`, or a fresh empty `IRValue`
placeholder if there is no initializer. Then `declare(id, value, kind)`
(§3.1). No instruction is emitted for an uncaptured variable; the
initializer's register simply *becomes* the variable.

## 11. Coverage matrix

| AST node | IR builder | Note |
|----------|-----------|------|
| `ExpressionStatement`, `BlockStatement`, `VariableStatement` | ✅ | |
| `IfStatement` | ⚠️ | §5 |
| `WhileStatement` | ⚠️ | §6 |
| `ForStatement` | ⚠️ | §7; `test` is required (a null `test` crashes) |
| `ReturnStatement` | ✅ | |
| `FunctionDeclaration` | ⚠️ | §8; no parameters |
| `NumericLiteral`, `StringLiteral`, `True/False/Null/UndefinedKeyword` | ✅ | |
| `IdentifierExpression` | ✅ | |
| `BinaryExpression` (arithmetic / compare / logical / bitwise) | ✅ | logical ops are eager |
| `BinaryExpression` (`=`) | ⚠️ | identifiers only; returns `true` |
| `BinaryExpression` (`+=` and other compound assignments) | ❌ | RHS evaluated, then dropped |
| `CallExpression` | ✅ | `print` special-cased |
| `ForInStatement` | 🚧 | started (uncommitted): evaluates object, creates `for.of.body`, visits `init`; no loop yet |
| `UnaryExpression`, `UpdateExpression` (`-x`, `!x`, `i++`) | ❌ | returns `true`, so parents hit `bad_variant_access` |
| `MemberExpression`, `NewExpression`, `ArrayLiteral`, `ObjectLiteral`, `ConditionalExpression`, `ArrowFunction`, `FunctionExpression`, `TemplateLiteral`, `ClassDeclaration`/`ClassExpression`, `This`, `Super` | ❌ | returns `true` |
| `DoWhile`, `Switch`, `Try`/`Catch`, `Throw`, `Break`, `Continue`, `ForOf` | ❌ | silently produce nothing |
| `Import`, `Enum`, `Interface`, `Await`, `Yield`, `Spread`, `Sequence`, `Comma`, UI view | ❌ | |

## 12. Checklist for adding a new construct

1. **Expressions must return a register**: `shared_ptr<IRValue>` from
   `createTemp`. Never return `true` from an expression visitor.
2. Allocate the result with the right `IRType`.
3. Use `emit()` (it targets `currentBlock`, which may have changed while
   visiting children).
4. For control flow:
   - create blocks with `createBlock` in the order you want them laid out;
   - set **both** `successors` and `predecessors`, in Phi-operand order;
   - end every block with exactly one terminator;
   - snapshot and restore `scopes` around branches and insert Phis at joins.
5. Name lookup must go through `lookup`/`store`/`declare`, so captured
   variables are routed to contexts.
6. Add a case to `Turbine::lowerInstruction` and, if the bytecode is new, to
   `DeathStar::run`. Otherwise the instruction silently disappears.
