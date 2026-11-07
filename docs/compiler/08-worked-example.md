# 8. Worked example: one program through every stage

The program is `ardan-lang/tests/arm64.ardan`, the file `test_ir()` runs, with
its comments removed:

```js
let one = 1;

function n() {
    let a = 90;
    print(a);

    function u() {
        a = 7;
        print(a, 80);
        return a * 2;
    }

    return u;
}

let fn1 = n();
let fn2 = n();

print(fn1());
print(fn2());
```

Every listing below is **real output** from the current code (captured with a
debug driver that links the project sources and prints each stage).

---

## Stage 1 — Tokens (excerpt)

```
KEYWORD 'LET'  IDENTIFIER 'one'  ASSIGN '='  NUMBER '1'  SEMI_COLON
KEYWORD 'FUNCTION'  IDENTIFIER 'n'  LEFT_PARENTHESIS  RIGHT_PARENTHESIS  LEFT_BRACKET
KEYWORD 'LET'  IDENTIFIER 'a'  ASSIGN  NUMBER '90'  SEMI_COLON
IDENTIFIER 'print'  LEFT_PARENTHESIS  IDENTIFIER 'a'  RIGHT_PARENTHESIS  SEMI_COLON
…
END_OF_FILE
```

## Stage 2 — AST

```
VariableStatement LET  one = NumericLiteral(1:SHORT)
FunctionDeclaration n()
  Block
    VariableStatement LET  a = NumericLiteral(90:SHORT)
    ExpressionStatement  Call(print, [Ident a])
    FunctionDeclaration u()
      Block
        ExpressionStatement  Binary(ASSIGN, Ident a, NumericLiteral 7)
        ExpressionStatement  Call(print, [Ident a, NumericLiteral 80])
        ReturnStatement      Binary(MUL, Ident a, NumericLiteral 2)
    ReturnStatement  Ident u
VariableStatement LET  fn1 = Call(Ident n, [])
VariableStatement LET  fn2 = Call(Ident n, [])
ExpressionStatement  Call(print, [Call(Ident fn1, [])])
ExpressionStatement  Call(print, [Call(Ident fn2, [])])
```

## Stage 3 — IR

### Capture analysis for `n`

- Names declared in `n`'s body: `{a}`. `u` is a function declaration, not a
  `VariableStatement`.
- Free names of the direct child `u`: `{a}`, found through `return a * 2`.
- Captured set: `{a}`, so `n` gets a one-slot context with `a → slot 0`.

`u` declares nothing and has no child functions, so it gets no context.

### Build order

```
build()                          temp=0  blockId=0
├─ entry0 created
├─ let one = 1                   %0 = Constant   (bound to "one", never used)
├─ visitFunction n               block n1
│   ├─ CreateContext             %1
│   ├─ let a = 90                %2 = Constant ; a is captured → StoreCurrentContextSlot
│   ├─ print(a)                  %3 = LoadCurrentContextSlot ; Print %3
│   ├─ visitFunction u           block u2 (scopes cleared; contexts = [n's])
│   │   ├─ a = 7                 %4 ; store() finds a in contexts[0] → StoreContextSlot depth 0
│   │   ├─ print(a, 80)          %5 load, %6 const, Print
│   │   ├─ return a * 2          %7 (result first), %8 load, %9 const, Multiply, Return
│   │   └─ functions.push(u)     → index 0
│   ├─ Closure for u             %10, captures %1
│   ├─ return u                  Return %10
│   └─ functions.push(n)         → index 1
├─ Closure for n                 %11 (no context at top level)
├─ fn1 = n()                     %12 = Call %11
├─ fn2 = n()                     %13 = Call %11
├─ print(fn1())                  %14 = Call %12 ; Print %14
├─ print(fn2())                  %15 = Call %13 ; Print %15
├─ Return <null>
└─ functions.push(entry)         → index 2
```

### Resulting IR

```
function #0 'u'
  u2:
    %4 = Constant                         ; 7
    StoreContextSlot %4 slot=0 depth=0
    %5 = LoadContextSlot slot=0 depth=0
    %6 = Constant                         ; 80
    Print %5 %6
    %8 = LoadContextSlot slot=0 depth=0
    %9 = Constant                         ; 2
    %7 = Multiply %8 %9
    Return %7
function #1 'n'
  n1:
    %1 = CreateContext slot=1
    %2 = Constant                         ; 90
    StoreCurrentContextSlot %2 slot=0
    %3 = LoadCurrentContextSlot slot=0
    Print %3
    %10 = Closure %1 fn='u'
    Return %10
function #2 '' [entry]
  entry0:
    %0 = Constant                         ; 1  (dead)
    %11 = Closure fn='n'
    %12 = Call %11
    %13 = Call %11
    %14 = Call %12
    Print %14
    %15 = Call %13
    Print %15
    Return <null>
```

## Stage 4 — Pharaoh

Skipped (commented out in `test_ir()`). A working DCE would remove `%0`.

## Stage 5 — Turbine

Register assignment, in order of first appearance during lowering:

| Function | IR value → register |
|----------|---------------------|
| `u` | `%4→r0  %5→r1  %6→r2  %8→r3  %9→r4  %7→r5` |
| `n` | `%1→r0  %2→r1  %3→r2  %10→r3` |
| entry | `%0→r0  %11→r1  %12→r2  %13→r3  %14→r4  %15→r5  <null>→r6` |

### Module #1 `n`: 68 bytes, decoded

```
135 | 1 0 0 0                       CreateFunctionContext 1
 10 | 0 0 0 0                       Star r0               ; %1
  3 | 90 0 0 0                      LdaSmi 90
 10 | 1 0 0 0                       Star r1               ; %2
  9 | 1 0 0 0                       Ldar r1
 34 | 0 0 0 0                       StaCurrentContextSlot 0
 31 | 0 0 0 0                       LdaCurrentContextSlot 0
 10 | 2 0 0 0                       Star r2               ; %3
152 | 1 0 0 0 | 2 0 0 0             Print argc=1 r2
133 | 0 0 0 0 | 0 0 0 0             CreateClosure fn=0(u) ctx=r0
 10 | 3 0 0 0                       Star r3               ; %10
144 | 3 0 0 0                       Return r3
```

### Module #0 `u`: 104 bytes, decoded

```
  3 | 7            LdaSmi 7
 10 | 0            Star r0                    ; %4
  9 | 0            Ldar r0
 33 | 0 | 0        StaContextSlot depth=0 slot=0
 29 | 0 | 0        LdaContextSlot depth=0 slot=0
 10 | 1            Star r1                    ; %5
  3 | 80           LdaSmi 80
 10 | 2            Star r2                    ; %6
152 | 2 | 1 | 2    Print argc=2 r1 r2
 29 | 0 | 0        LdaContextSlot depth=0 slot=0
 10 | 3            Star r3                    ; %8
  3 | 2            LdaSmi 2
 10 | 4            Star r4                    ; %9
 39 | 3 | 4        Mul r3 r4
 10 | 5            Star r5                    ; %7
144 | 5            Return r5
```
(each operand is 4 bytes little-endian; trailing zero bytes omitted)

### Module #2 entry: 103 bytes, decoded

```
  3 | 1                  LdaSmi 1
 10 | 0                  Star r0                ; %0 (dead)
133 | 1 | FF FF FF FF    CreateClosure fn=1(n) ctx=none
 10 | 1                  Star r1                ; %11
116 | 1 | 0              CallUndefinedReceiver r1 argc=0
 10 | 2                  Star r2                ; %12 = fn1
116 | 1 | 0              CallUndefinedReceiver r1 argc=0
 10 | 3                  Star r3                ; %13 = fn2
116 | 2 | 0              CallUndefinedReceiver r2 argc=0
 10 | 4                  Star r4
152 | 1 | 4              Print argc=1 r4
116 | 3 | 0              CallUndefinedReceiver r3 argc=0
 10 | 5                  Star r5
152 | 1 | 5              Print argc=1 r5
144 | 6                  Return r6              ; never written → undefined
```

`Compiled.entry_index = 2`.

## Stage 6 — DeathStar

```
call(2)                                   entry
 ├ CreateClosure → Closure{1, null}
 ├ call(1, ctx=null)                      n #1
 │   ├ CreateFunctionContext → ctxA{[undef]}
 │   ├ ctxA[0] = 90 ; print → "90"
 │   └ return Closure{0, ctxA}            = fn1
 ├ call(1, ctx=null)                      n #2
 │   ├ ctxB{[undef]} ; ctxB[0] = 90 ; print → "90"
 │   └ return Closure{0, ctxB}            = fn2
 ├ call(0, ctx=ctxA)                      fn1()
 │   ├ ctxA[0] = 7 ; print → "7 80"
 │   └ return 7*2 = 14
 ├ print → "14"
 ├ call(0, ctx=ctxB)                      fn2()
 │   ├ ctxB[0] = 7 ; print → "7 80"
 │   └ return 14
 └ print → "14"
```

Program output, with the blank line `kPrint` adds after each line:

```
90

90

7 80

14

7 80

14
```

> Every run of the binary also prints a single `1` line first. It comes from
> code outside this pipeline and is unrelated to the program.
