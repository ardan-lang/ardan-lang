            ┌──────────────┐
            │  Source Code  │
            └──────┬───────┘
                   │
          ┌────────▼────────┐
          │    Compiler     │
          └───┬────────┬────┘
              │        │
     VM Bytecode   UI Bytecode
        (logic)      (UI tree)
              │        │
              ▼        ▼
            VM     ArdanUI Runtime
                        │
                        ▼
                 SDL / Cocoa / Win32


| VM Type            | Engine-style Name |
| ------------------ | ----------------- |
| Stack + Upvalue    | CascadeVM       |
| Register + Upvalue | NovaVM          |
| Register + Context | PeregrineVM     |
| Stack + Context    | TitanVM         |

AtlasVM

class VM {};
class StackVM : public VM {};
class RegisterVM : public VM {};

class StackUpvalueVM : public StackVM {};
class RegisterUpvalueVM : public RegisterVM {};

class StackExecutionContextVM : public StackVM {};
class RegisterExecutionContextVM : public RegisterVM {};


Parser
 ↓
AST
 ↓
Semantic Analyzer
 ↓
HIR
 ↓
Type Inference
 ↓
MIR
 ↓
Optimization Passes
 ↓
Register Allocation
 ↓
Bytecode IR
 ↓
TurboBytecode
 ↓
VM


%1 = load b
%2 = load c
%3 = mul %1, %2
%4 = load a
%5 = add %4, %3

let x = b * c
let y = a + x

Important insight (this is where many compilers go wrong)

You should treat this like a directed dependency graph:

- inputs = “who I depend on”
- outputs = “who depends on me”

So in compiler IR terms:

- inputs = operands
- outputs = users

# Effects and Control

These are from advanced compiler IR design, especially Sea-of-Nodes style IRs like in TurboFan.

They separate:

1. **Data flow**
2. **Control flow**
3. **Effect flow**

because they represent different kinds of dependencies.

---

# 1. Data Inputs/Outputs

These are the normal value dependencies.

Example:

```js
let c = a + b;
```

IR:

```txt
a ----\
       Add ----> c
b ----/
```

Meaning:

* `Add` depends on values `a` and `b`

---

# 2. Control Inputs/Outputs

These track:

> “When is this node allowed to execute?”

They represent execution order / branching / program flow.

---

Example:

```js
if (x) {
  y = 1;
}
```

Control flow graph:

```txt
        Start
           |
          If
         /  \
      True  False
        |
      Store(y=1)
        |
       End
```

---

Here:

```cpp
controlInputs
```

means:

> Which control path reaches this node?

---

Example:

```txt
Store(y=1)
```

has:

```cpp
controlInputs = [TrueBranch]
```

because the store only executes if the condition is true.

---

`controlOutputs` are reverse edges:

```cpp
If.controlOutputs = [TrueBranch, FalseBranch]
```

---

# Why control edges matter

Consider:

```js
if (x) {
  foo();
}
bar();
```

You CANNOT move `foo()` after `bar()`.

Control edges preserve execution legality.

---

# 3. Effect Inputs/Outputs

These are VERY important.

They track:

> Side effects and memory ordering.

---

Pure math operations don't need effect edges:

```js
a + b
```

has no side effects.

---

But these DO:

```js
obj.x = 10
print("hi")
arr.push(1)
```

because they modify state/world/memory.

---

Example:

```js
obj.x = 10;
obj.y = 20;
```

You cannot reorder these arbitrarily.

So IR builds an effect chain:

```txt
StartEffect
     |
 Store(obj.x)
     |
 Store(obj.y)
     |
 EndEffect
```

---

Meaning:

```cpp
StoreY.effectInputs = [StoreX]
```

This says:

> StoreY depends on StoreX happening first.

---

# Why effects exist separately from control

Because execution order alone is not enough.

Example:

```js
a = obj.x;
b = obj.x;
```

These loads can be reordered or optimized.

But:

```js
obj.x = 10;
a = obj.x;
```

The load MUST happen after the store.

Effect edges preserve memory correctness.

---

# Real example

JavaScript:

```js
obj.x = 10;
print(obj.x);
```

IR idea:

```txt
          Control Flow
Start ---------------------> Print

          Effect Flow
StartEffect
      |
   Store(obj.x=10)
      |
   Load(obj.x)
      |
    Print
```

---

# Summary

## Data edges

```cpp
inputs / outputs
```

Track:

```txt
values
```

Example:

```txt
a + b
```

---

## Control edges

```cpp
controlInputs / controlOutputs
```

Track:

```txt
execution order / branches / loops
```

Example:

```txt
if, loop, return, branch
```

---

## Effect edges

```cpp
effectInputs / effectOutputs
```

Track:

```txt
memory/state side effects
```

Example:

```txt
store, print, function call, IO
```

---

# Full visualization

Code:

```js
if (x) {
   obj.a = 10;
}

print(obj.a);
```

IR conceptually:

```txt
CONTROL:

Start
  |
 If(x)
  |
TrueBranch
  |
Store
  |
Merge
  |
Print


EFFECT:

StartEffect
    |
 Store(obj.a)
    |
 Load(obj.a)
    |
 Print
```

---

# Why this architecture is powerful

This allows optimizations like:

* dead code elimination
* instruction scheduling
* common subexpression elimination
* loop invariant code motion
* speculative optimization

while still preserving correctness.

This is why modern optimizing compilers use these graph structures.

| Type       | Storage             |
| ---------- | ------------------- |
| local vars | registers           |
| closures   | context objects     |
| globals    | global object slots |


| Your system | LLVM equivalent            |
| ----------- | -------------------------- |
| symTable    | Value naming in IRBuilder  |
| IRValue     | SSA virtual register       |
| regMap      | Machine register allocator |
| VM regs     | CPU registers              |
