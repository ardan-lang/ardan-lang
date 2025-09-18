# Ardan Programming Language

Ardan is a dynamically typed programming language, featuring a hand-written lexer, parser, and interpreter implemented in C++. Ardan supports variables, functions, classes, inheritance, arrays, objects, control flow, and modern syntax features such as `let/const/var`, scoping.

## Features
- **Dynamic Typing**: Variable types are determined at runtime.
- **Lexical Scope**: Supports block, function, and global scoping with `var`, `let`, `const`.
- **First-Class Functions**: Functions can be assigned to variables, passed as arguments, and returned.
- **Classes & Inheritance**: Class syntax with fields, methods, and inheritance via `extends`.
- **Arrays & Objects**: Flexible objects and arrays with dot and bracket notation.
- **Comprehensive Operator Support**: Arithmetic, logical, bitwise, assignment, comparison, and more.
- **Exception Handling**: `Try/catch/finally` for robust error handling.

⸻

**Ardan Programming Language**

Ardan is a modern programming language, implemented in C++. It features block scoping, classes and inheritance, closures, rich expressions, built-in error handling, customizable standard library, and a flexible interpreter architecture. This documentation provides a comprehensive guide for Ardan, including language syntax, semantics, and usage, with detailed examples for every concept.

⸻

## Table of Contents

1. [Getting Started](#getting-started)
2. [Language Overview](#language-overview)
    - [Variables](#variables)
    - [Data Types & Literals](#data-types--literals)
    - [Expressions and Operators](#expressions-and-operators)
    - [Functions](#functions)
    - [Control Flow](#control-flow)
    - [Classes & Inheritance](#classes--inheritance)
    - [Error Handling](#error-handling)
    - [Modules & Standard Library](#modules--standard-library)
    - [Arrays & Objects](#arrays--objects)
3. [Reference](#reference)
    - [Keywords](#keywords)
    - [Operators](#operators)
    - [Statements](#statements)
    - [Special Values](#special-values)
4. [Complete Example](#complete-example)
5. [Extending Ardan](#extending-ardan)
6. [Frequently Asked Questions](#frequently-asked-questions)

⸻

## Getting Started

### Installation

Ardan is a C++ project. Build it with CMake or your preferred C++ toolchain.

```sh
git clone https://github.com/ardan-lang/ardan-lang
cd ardan-lang
mkdir build && cd build
cmake ..
cmake --build .
```

Using scripts:

```sh
git clone https://github.com/ardan-lang/ardan-lang
cd ardan-lang
sudo sh ./build.sh
```

Running a Ardan File

```
./ardan path/to/script.ardan --interpret
./ardan path/to/code_file.ardan --i
```

Or start a REPL (not yet available):

```
./ardan
```

⸻

## Language Overview

### Variables

Ardan supports three kinds of variable declarations:

```
var x = 42;     // Function-scoped, mutable
let y = "ardan";  // Block-scoped, mutable
const z = 3.14; // Block-scoped, immutable
```

### Example: Variable Shadowing

```
let x = 1;
{
    let x = 2;
    print(x); // 2
}
print(x); // 1
```

### const Requires Initialization

```
const fail; // Error: Missing initializer in const declaration
```

⸻

## Data Types & Literals

### Numbers

```
let a = 10;
let b = -3.5;
let c = 2e10;
```

### Strings

```
let msg = "Hello, world!";
let templ = `Hello,
multiline!`;
```

### Booleans

```
let flag = true;
let off = false;
```

### Arrays

```
let arr = [1, 2, 3];
print(arr[1]); // 2
```

### Objects

```
let user = { name: "Alice", age: 30 };
print(user.name); // "Alice"
```

⸻

### Expressions and Operators

Ardan supports a wide range of expressions and operators akin to JS.

### Arithmetic

```
let sum = 1 + 2 * 3;   // 7
let div = 10 / 2;      // 5
let mod = 9 % 4;       // 1
let pow = 2 ** 3;      // 8
```

### Assignment and Compound Assignment

```
let x = 10;
x += 2;  // x = 12
x *= 3;  // x = 36
```

### Comparison

```
5 == "5";   // true (value)
5 === "5";  // false (strict type)
5 !== 5;    // false
7 < 10;     // true
```

### Logical

```
true && false; // false
true || false; // true
!false;        // true
x = null ?? 42; // 42
```

### Bitwise

```
let mask = 0b1010 & 0b1100; // 0b1000 (8)
```

### Increment/Decrement

```
let i = 0;
i++;
++i;
i--;
```

⸻

### Functions

Ardan supports first-class functions, closures, and parameters.

```
function square(n) {
    return n * n;
}

print(square(5)); // 25
```

### Anonymous Functions and Closures

```
let adder = function(x) {
    return function(y) {
        return x + y;
    };
};
let add5 = adder(5);
print(add5(3)); // 8
```

### Default and Rest Arguments

### 1. Default

Default parameters allow a function parameter to have a fallback value if no argument is provided, or if `undefined` is explicitly passed.

**Syntax:**

```
function greet(name = "World") {
  console.log("Hello, " + name);
}

greet();         // Hello, World
greet("Alice");  // Hello, Alice
greet(undefined); // Hello, World (default applies)
greet(null);     // Hello, null (default does NOT apply)
```

**Semantics:**

* Default value expressions are evaluated **at call time**, not at function definition.
* Defaults are only used when the argument is `missing` or explicitly `undefined`.
* Defaults can reference earlier parameters:

  ```
  function f(a = 1, b = a + 2) {
    console.log(a, b);
  }
  f();    // 1 3
  f(5);   // 5 7
  ```

---

### 2. Rest

Rest parameters allow a function to capture all **remaining arguments** into a single array.

**Syntax:**

```
function sum(...nums) {
  return nums.reduce((a, b) => a + b, 0);
}

console.log(sum(1, 2, 3, 4)); // 10
```

**Semantics:**

* Rest parameters must be the **last parameter** in the list.
* They collect all extra arguments into a proper array.
* If no extra arguments are supplied, the rest parameter is an empty array.
* Rest parameters **cannot** have defaults, and there can only be **one**.

---

### 3. Combined Example

```js
function demo(a, b = 2, ...rest) {
  console.log("a:", a);
  console.log("b:", b);
  console.log("rest:", rest);
}

demo(1);           // a:1, b:2, rest:[]
demo(1, 10, 20);   // a:1, b:10, rest:[20]
demo(1, undefined, 3, 4, 5);
// a:1, b:2 (default used), rest:[3,4,5]
```

⸻

### Control Flow

Ardan supports all standard control flow constructs.

### If-Else

```ardan
if (x > 10) {
    print("big");
} else if (x > 5) {
    print("medium");
} else {
    print("small");
}
```

### While Loops

```
let i = 0;
while (i < 3) {
    print(i);
    i++;
}
```

### For Loops

```
for (let i = 0; i < 5; i++) {
    print(i);
}
```

### For-in (over object properties):

```
let obj = {a: 1, b: 2};
for (let key in obj) {
    print(key, obj[key]);
}
```

### For-of (over array values):

```
let arr = [1, 2, 3];
for (let value of arr) {
    print(value);
}
```

### Break & Continue

```
for (let i = 0; i < 10; i++) {
    if (i == 5) break;
    if (i % 2 == 0) continue;
    print(i);
}
```

### Switch Statement

```
switch (fruit) {
    case "apple":
        print("Apple!");
        break;
    case "banana":
        print("Banana!");
        break;
    default:
        print("Unknown fruit");
}
```

### Do-While

```
let x = 0;
do {
    print(x);
    x++;
} while (x < 3);
```

⸻

### Classes & Inheritance

Ardan enables object-oriented programming with ES6-like classes.

### Basic Class

```
class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
    move(dx, dy) {
        this.x += dx;
        this.y += dy;
    }
}

let pt = new Point(0, 0);
pt.move(3, 4);
print(pt.x, pt.y); // 3, 4
```

### Inheritance

```
class Animal {
    constructor(name) {
        this.name = name;
    }
    speak() {
        print(this.name + " makes a noise.");
    }
}

class Dog extends Animal {
    speak() {
        print(this.name + " barks.");
    }
}

let d = new Dog("Rex");
d.speak(); // Rex barks.
```

### Access Modifiers

Ardan supports public, private, protected, and static keywords (syntax).

```
class Counter {
    public value = 0;
    private secret = 42;

    increment() {
        this.value++;
    }
}
```

⸻

### Error Handling

Ardan supports robust error handling.

```
try {
    mightThrow();
} catch (e) {
    print("Caught error:", e);
} finally {
    print("This always runs.");
}
```

### Throwing Errors

```
if (somethingWrong) {
    throw "Something bad happened!";
}
```

⸻

### Modules & Standard Library

### Print

```
print("Hello, world!");
```

### Math & File Modules

Ardan can be extended with modules like Math and File (see C++ source).

```
let pi = Math.PI;
let result = Math.pow(2, 8); // 256
```

⸻

### Arrays & Objects

### Arrays

```
let arr = [10, 20, 30];
print(arr[0]); // 10
arr[1] = 99;
print(arr.length); // 3
```

### Objects

```
let user = { name: "Bob", age: 25 };
print(user.name); // Bob
user.email = "bob@example.com";
```

⸻

### Reference

### Keywords

let, const, var, function, class, extends, constructor, if, else, while, for, in, of, break, continue, return, switch, case, default, try, catch, finally, throw, this, super, public, private, protected, static

### Operators

Arithmetic: +, -, *, /, %, **, ++, --
Comparison: ==, !=, ===, !==, <, <=, >, >=
Logical: &&, ||, !, ??
Assignment: =, +=, -=, *=, /=, %= etc.
Bitwise: &, |, ^, ~, <<, >>, >>>
Member access: ., []

### Statements

• Expression Statement
• Block Statement { ... }
• If / Else
• While / Do-While
• For / For-In / For-Of
• Break / Continue / Return / Throw
• Switch
• Try / Catch / Finally
• Function Declaration
• Class Declaration

### Special Values

• true, false
• null (reserved)
• undefined (reserved)

⸻

### Complete Example

```
// Factorial Example
function factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

print("5! =", factorial(5)); // 5! = 120

// Array iteration
let nums = [1, 2, 3, 4, 5];
let sum = 0;
for (let num of nums) {
    sum += num;
}
print("Sum:", sum); // 15

// Object usage
let dog = { name: "Max", age: 4 };
for (let key in dog) {
    print(key, dog[key]);
}

// Class usage
class Greeter {
    constructor(name) {
        this.name = name;
    }
    greet() {
        print("Hello, " + this.name + "!");
    }
}
let g = new Greeter("Ardan");
g.greet(); // Hello, Ardan!
```

⸻

### Extending Ardan

- **New built-ins**: Implement in C++ in Interpreter/ or builtin/.
- **Language features**: Add to Parser.hpp and Statements/, Expression/.
- **Standard library**: Add modules in C++, expose via Env or as Ardan objects.

⸻

### Frequently Asked Questions

- Q: How do I debug my Ardan code?
- A: Use print() statements, and the AstPrinter for AST debugging in C++.

- Q: Can I use recursion and closures?
- A: Yes! Ardan fully supports both.

- Q: Is there file or network IO?
- A: File IO is in progress via the File module. See File.hpp􀰓.

- Q: How do I add my own functions to the interpreter?
- A: Extend the interpreter in C++ and register new functions in Env.

⸻


- Developer: Chidume Nnamdi
- Email: kurtwanger40@gmail.com
