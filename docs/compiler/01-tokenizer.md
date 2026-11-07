# 1. Tokenizer (`Scanner`)

**Source → `vector<Token>`**

| File | Role |
|------|------|
| `ardan-lang/Scanner/Scanner.hpp/.cpp` | The scanner |
| `ardan-lang/Scanner/Token/Token.hpp` | `Token` record |
| `ardan-lang/Scanner/Token/TokenType.h` | `enum class TokenType` |
| `ardan-lang/Scanner/keywords/keywords.cpp` | Reserved-word table |

## The `Token`

```cpp
class Token {
public:
    TokenType type;   // category
    string lexeme;    // text (or normalised text, see below)
    int line;         // 1-based source line
};
```

Many punctuation tokens are created with `addToken(type)` and therefore have an
**empty lexeme** (`;`, `(`, `)`, `{`, `}` …). Downstream code must switch on
`type`, not on `lexeme`, for those.

## Driving loop

`Scanner::getTokens()` is a single-pass, hand-written scanner over a
`string&` with one cursor, `current`:

```
while current < source.length():
    switch source[current]:
        … produce zero or one token …
    advance()                 // current++
addToken(END_OF_FILE)
```

Two conventions make the code work:

1. **`peek()` looks one character ahead** (`source[current + 1]`), and
   `match(c)` consumes that look-ahead character if it equals `c`. Multi-char
   operators are recognised by chaining `match`, e.g. `=` → `match('=')` →
   `match('=')` gives `===`.
2. **Collectors overshoot, then `reverse()`.** `collectNumber()` and
   `collectIdentifier()` advance past the last character of the token and then
   step back once, because the main loop always ends with `advance()`.

Whitespace (`' '`, `\t`, `\r`) produces nothing; `\n` increments `line`.

## Token categories

| Input | TokenType | Lexeme stored |
|-------|-----------|---------------|
| `123`, `1.5`, `1e9` | `NUMBER` | digits as written |
| `0x1F`, `0b101`, `0o17` | `NUMBER` | **converted to decimal** (`"31"`) |
| `"..."`, `'...'` | `STRING` | contents without quotes. No escape processing in plain strings. |
| `` `a${x}b` `` | `TEMPLATE_START`, `TEMPLATE_CHUNK`, `INTERPOLATION_START`, … `INTERPOLATION_END`, `TEMPLATE_END` | chunks hold processed escapes (`\n`, `\x41`, `\u{…}`) |
| identifier | `IDENTIFIER` | the name |
| reserved word | `KEYWORD` | **upper-cased name** from `keywords` (`let` → `"LET"`) |
| `true` / `false` | `BOOLEAN` | `"TRUE"` / `"FALSE"` |
| `class`, `typeof`, `delete`, `instanceof`, `void`, `await`, `in`, `yield` | dedicated types (`CLASS`, `TYPEOF`, …) | upper-cased |
| `//…`, `/*…*/` | – | skipped |
| `@` | `AT` | `"@"` (UI view syntax) |

Identifiers match `[A-Za-z_$][A-Za-z0-9_$]*`.

### Operators

Each operator family is a `case` in the main switch that resolves the longest
match:

| Char | Possible tokens |
|------|-----------------|
| `=` | `ASSIGN` `=`, `VALUE_EQUAL` `==`, `REFERENCE_EQUAL` `===`, `ARROW` `=>` |
| `+` | `ADD`, `INCREMENT` `++`, `ASSIGN_ADD` `+=` |
| `-` | `MINUS`, `DECREMENT` `--`, `ASSIGN_MINUS` `-=` |
| `*` | `MUL`, `POWER` `**`, `ASSIGN_MUL` `*=`, `POWER_ASSIGN` `**=` |
| `/` | `DIV`, `ASSIGN_DIV`, or starts a comment |
| `<` `>` | relational, shifts (`<<`, `>>`, `>>>`) and their `=` forms |
| `&` `\|` | bitwise / logical and their assignment forms |
| `!` | `LOGICAL_NOT`, `INEQUALITY` `!=`, `STRICT_INEQUALITY` `!==` |
| `?` | `TERNARY`, `NULLISH_COALESCING` `??`, `??=`, `OPTIONAL_CHAINING` `?.` |
| `.` | `DOT`, `SPREAD` `...` |

### Template literals

`` ` `` calls `collectLiteralString()`, which emits a token *stream*, not one token:

```
`v=${a}`   →   TEMPLATE_START
               TEMPLATE_CHUNK "v="
               INTERPOLATION_START
               IDENTIFIER a          ← produced by a nested Scanner over "a"
               INTERPOLATION_END
               TEMPLATE_END
```

The text inside `${ … }` is collected with brace counting and scanned by a
**fresh `Scanner`**. Its tokens are spliced in, without its `END_OF_FILE`.
Because the nested scanner starts at `line = 1`, interpolated tokens report
line 1 regardless of their real position.

## Example

Source:

```js
let a = 90;
if (a >= 0x1F) { print(`v=${a}`, "s"); }
```

Real output of `Scanner::getTokens()` (numeric `TokenType` value in brackets):

```
[60] KEYWORD              'LET'    L1
[59] IDENTIFIER           'a'      L1
[13] ASSIGN               '='      L1
[58] NUMBER               '90'     L1
[ 3] SEMI_COLON           ''       L1
[60] KEYWORD              'IF'     L2
[ 5] LEFT_PARENTHESIS     ''       L2
[59] IDENTIFIER           'a'      L2
[37] GREATER_THAN_EQUAL   '>='     L2
[58] NUMBER               '31'     L2   ← 0x1F normalised to decimal
[ 6] RIGHT_PARENTHESIS    ''       L2
[ 7] LEFT_BRACKET         ''       L2
[59] IDENTIFIER           'print'  L2
[ 5] LEFT_PARENTHESIS     ''       L2
[65] TEMPLATE_START       ''       L2
[66] TEMPLATE_CHUNK       'v='     L2
[67] INTERPOLATION_START  ''       L2
[59] IDENTIFIER           'a'      L1   ← nested scanner, line resets
[68] INTERPOLATION_END    ''       L2
[69] TEMPLATE_END         ''       L2
[ 0] COMMA                ','      L2
[57] STRING               's'      L2
[ 6] RIGHT_PARENTHESIS    ''       L2
[ 3] SEMI_COLON           ''       L2
[ 8] RIGHT_BRACKET        ''       L2
[78] END_OF_FILE          ''       L3
```

> `LEFT_BRACKET` / `RIGHT_BRACKET` are **curly braces** `{ }`;
> square brackets are `LEFT_SQUARE_BRACKET` / `RIGHT_SQUARE_BRACKET`.

## Notes and limitations

- Multi-line `/* */` comments and multi-line strings do not increment `line`,
  so line numbers after them are too low.
- Negative numbers are not literals: `-3` is `MINUS`, `NUMBER "3"`. The parser
  turns this into a `UnaryExpression`, which the IR builder does not implement
  yet (see [IR Builder coverage](04-ir-builder.md#11-coverage-matrix)). Write
  `0 - 3` in the meantime.
- Unknown characters fall through the `default:` branch silently. No error is
  raised.
