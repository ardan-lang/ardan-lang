# 2. Parser and AST

**`vector<Token>` → `vector<unique_ptr<Statement>>` (the AST)**

| File | Role |
|------|------|
| `ardan-lang/Parser/Parser.hpp/.cpp` | Recursive-descent parser. The expression precedence chain is in the header. |
| `ardan-lang/Statements/Statements.hpp` | Statement node classes |
| `ardan-lang/Expression/Expression.hpp` | Expression node classes, including `NumericLiteral` number parsing |
| `ardan-lang/Statements/StatementVisitor.hpp`, `ExpressionVisitor/ExpressionVisitor.hpp` | Visitor interfaces |
| `ardan-lang/Visitor/AstPrinter/` | Debug printer for the AST |

## Shape of the parser

```cpp
vector<unique_ptr<Statement>> Parser::parse() {
    while (!isAtEnd()) statements.push_back(parseStatement());
    return std::move(statements);
}
```

`parseStatement()` dispatches on the **first token**. Because reserved words
are `KEYWORD` tokens carrying an upper-case lexeme, the dispatch compares lexemes:

| Leading token | Parsed as |
|---------------|-----------|
| `;` | `EmptyStatement` |
| `{` | `BlockStatement` |
| `KEYWORD "IF" / "WHILE" / "FOR" / "DO" / "SWITCH" / "TRY" / "THROW" / "RETURN" / "BREAK" / "CONTINUE"` | the matching statement |
| `KEYWORD "VAR" / "LET" / "CONST"` | `VariableStatement` |
| `KEYWORD "FUNCTION"` | `FunctionDeclaration` |
| `KEYWORD "IMPORT" / "ASYNC" / "ENUM" / "INTERFACE"` | import / async function / enum / interface |
| `CLASS` | `ClassDeclaration` |
| `AT` (`@`) | UI view statement |
| anything else | `ExpressionStatement` |

### Bodies must be blocks

`if`, `else`, `while`, `for` and function bodies are parsed with
`parseBlockStatement()`, which **requires `{ … }`**. `if (x) y = 1;` is a parse
error (`Expected '{'`). This matters for the IR builder: every branch or loop
body introduces a block scope (see [IR Builder §Scopes](04-ir-builder.md#3-scopes-and-the-symbol-table)).

### Expression precedence (lowest → highest)

Each level is a method in `Parser.hpp` that calls the next one:

```
parseExpression
 └ parseComma              a, b            → SequenceExpression
   └ parseAssignment       = += -= … =>    → BinaryExpression(op=ASSIGN…) / ArrowFunction / YieldExpression
     └ parseConditional    ?:              → ConditionalExpression
       └ parseNullish      ??
         └ parseLogicalOr  ||
           └ parseLogicalAnd &&
             └ parseBitwiseOr |
               └ parseBitwiseXor ^
                 └ parseBitwiseAnd &
                   └ parseEquality == != === !==
                     └ parseRelational < > <= >= instanceof in
                       └ parseShift << >> >>>
                         └ parseAdditive + -
                           └ parseMultiplicative * / %
                             └ parseExponentiation **
                               └ parseUnary ! - + ~ typeof void delete await
                                 └ parseUpdate ++ --
                                   └ parseLeftHandSide  calls, a.b, a[b], new
                                     └ parsePrimary  literals, identifiers, (…), [..], {..}, function, class, template
```

> **Assignments are `BinaryExpression`s.** `parseAssignment` builds
> `BinaryExpression(op, left, right)` where `op.type` is `ASSIGN`,
> `ASSIGN_ADD`, etc. (the `AssignmentExpression` constructor is commented out).
> The IR builder therefore detects assignments inside `visitBinary`.

### Primaries

| Tokens | Node |
|--------|------|
| `NUMBER` | `NumericLiteral(lexeme)` |
| `STRING` | `StringLiteral(text)` |
| `BOOLEAN "TRUE"/"FALSE"` | `TrueKeyword` / `FalseKeyword` |
| `IDENTIFIER "undefined"` | `UndefinedKeyword` |
| other `IDENTIFIER` | `IdentifierExpression` (`name` = lexeme, `token` kept) |
| `KEYWORD "NULL" / "THIS" / "SUPER"` | `NullKeyword` / `ThisExpression` / `SuperExpression` |
| template tokens | `TemplateLiteral` |

## `NumericLiteral`: typed number literals

`NumericLiteral` parses its text in the constructor and stores both a value
(in the `R` variant) and a `NumberType` tag. The IR builder relies on this tag.

```cpp
enum class NumberType { NONE, SHORT, USHORT, INT, UINT, LONG, ULONG,
                        LONG_LONG, ULONG_LONG, FLOAT, DOUBLE, LONG_DOUBLE };
```

Integer literals get the **smallest** C++ type that fits:

| Literal | Stored as | `NumberType` |
|---------|-----------|--------------|
| `90` | `short` | `SHORT` |
| `40000` | `int` | `INT` |
| `5000000000` | `long` | `LONG` |
| `1.5` | `float` (fits float range) | `FLOAT` |
| `1.5l` | `long double` | `LONG_DOUBLE` |
| `10u`, `10ul`, `10ll` | suffix-driven | `USHORT` / `ULONG` / `LONG_LONG` … |

Prefixes `0x`, `0b`, and a leading `0` (octal) are honoured. Note that the
scanner has already converted hex, binary and `0o` literals to decimal text.

## Key AST nodes used by the IR

```cpp
struct VariableDeclarator { string id; unique_ptr<Expression> init; };      // init may be null
class VariableStatement  { string kind; vector<VariableDeclarator> declarations; }; // kind is "LET"/"VAR"/"CONST"
class FunctionDeclaration{ string id; vector<unique_ptr<Expression>> params; unique_ptr<Statement> body; };
class IfStatement        { unique_ptr<Expression> test; unique_ptr<Statement> consequent, alternate; };
class WhileStatement     { unique_ptr<Expression> test; unique_ptr<Statement> body; };
class ForStatement       { unique_ptr<Statement> init; unique_ptr<Expression> test, update; unique_ptr<Statement> body; };
class ReturnStatement    { unique_ptr<Expression> argument; };                 // may be null
class BinaryExpression   { unique_ptr<Expression> left; Token op; unique_ptr<Expression> right; };
class CallExpression     { unique_ptr<Expression> callee; vector<unique_ptr<Expression>> arguments; };
class IdentifierExpression { string name; Token token; };
```

Function parameters are general `Expression`s parsed with `parseAssignment()`:
`a` is an `IdentifierExpression`, `a = 1` is a `BinaryExpression(ASSIGN)`, and
`...rest` is a `RestParameter`.

## Visitor protocol

Every node implements `R accept(Visitor&)`, which calls the matching
`visitXxx`. `R` is the project-wide result variant (`Interpreter/R.hpp`):

```cpp
using R = std::variant<std::monostate, std::nullptr_t, double, unsigned long, unsigned short,
                       long, long long, short, unsigned int, unsigned long long, float, long double,
                       int, char, string, bool, shared_ptr<JSObject>, shared_ptr<Value>,
                       shared_ptr<JSClass>, shared_ptr<JSArray>, Value, IRValue, shared_ptr<IRValue>>;
```

The IR builder uses `R` in two ways:

- **Expression visitors return `shared_ptr<IRValue>`**, the virtual register
  holding the expression's result. Callers unwrap it with
  `get<shared_ptr<IRValue>>(expr->accept(*this))`.
- **Statement visitors return `true`** (a `bool`). Their effects go into the
  current basic block.

If an *expression* visitor returns `true` instead of a register (true of every
unimplemented visitor), the caller's `get<shared_ptr<IRValue>>` throws
`std::bad_variant_access`. See the coverage table in
[04-ir-builder.md](04-ir-builder.md#11-coverage-matrix).

## Example AST

```js
let one = 1;
function n() { let a = 90; return a; }
print(n());
```

```
Program
├─ VariableStatement kind="LET"
│   └─ VariableDeclarator id="one" init=NumericLiteral(1 : SHORT)
├─ FunctionDeclaration id="n" params=[]
│   └─ BlockStatement
│       ├─ VariableStatement kind="LET"
│       │   └─ VariableDeclarator id="a" init=NumericLiteral(90 : SHORT)
│       └─ ReturnStatement argument=IdentifierExpression("a")
└─ ExpressionStatement
    └─ CallExpression callee=IdentifierExpression("print")
        └─ CallExpression callee=IdentifierExpression("n") args=[]
```
