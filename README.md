# Tau programming languague

Tau programming languague is written in C++.

## Basics

**Variable declarations**

```js
var mod = 8 % 5;
var bl = true;


var square = 9 ** 9;
var ref = ref1 == uiop;
var d = "nnamdi chidume";
var t = 90;
var g = sqrt();
var x = (1, 2, 3);

```

**Assignment**

```js
mod %= 9;
```

**Printing to console**

```js
print(g, d, t, 34);
```

**Class usage**

```js
class Foo {
  greet(name, age) {
    print(name, age);
  }
}
```

**do while**

```js
do {
  print("hello");
  throw "Hello";
} while (x < 10);
```

**Switch**

```js
switch (x) {
  case 1:
    print("one");
    break;
  case 2:
    print("two");
    break;
  default:
    print("other");
}
```

# Running the code

## Scanning

```cpp
    string lang = R"(
    
    var outside_name = "outside";
    
    class Parent {
        var p = 90;
    }
    
    class User extends Parent {
    constructor() {
    super();
    }
    
    var name = outside_name;
    var age = 90;
    getAge() {
    print(this.age);
    this.displayName();
    }
    
    displayName() {
    print(this.name);
    }
    
    }
    
    var user_obj = new User();
    
    print(user_obj.p);
    
    )";


    Scanner scanner(lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.lexeme << endl;
    }
```

## Parsing

```cpp
    Parser parser(scanner.getTokens());
    auto ast = parser.parse();

    AstPrinter printer;

    for (auto& stmt : ast) {
        stmt->accept(printer);
    }

```

Output:

```sh
VariableDeclaration VAR =
mod
  Binary(%)
    Numeric 8
    Numeric 5
VariableDeclaration VAR =
bl
  True
ExpressionStatement:
  Binary(%=)
    Identifier(mod)
    Numeric 9
VariableDeclaration VAR =
square
  Binary()
    Numeric 9
    Numeric 9
VariableDeclaration VAR =
ref
  Binary(==)
    Identifier(ref1)
    Identifier(uiop)
VariableDeclaration VAR =
d
  String nnamdi chidume
VariableDeclaration VAR =
t
  Numeric 90
VariableDeclaration VAR =
g
  Call
    Identifier(sqrt)
ExpressionStatement:
  Call
    Identifier(print)
    Identifier(g)
    Identifier(d)
    Identifier(t)
    Numeric 34
ExpressionStatement:
  Sequence:
    Identifier(g)
    Identifier(d)
    Identifier(t)
    Numeric 34
VariableDeclaration VAR =
x
  Sequence:
    Numeric 1
    Numeric 2
    Numeric 3
Class oo {
  Method greet(name, age)   Block:
    ExpressionStatement:
      Call
        Identifier(print)
        Identifier(name)
        Identifier(age)
}
Continue
DoWhileStatement {
  body: Block:
  ExpressionStatement:
    Call
      Identifier(print)
      String hello
  Throw
    String Hello

  condition: Binary()
  Identifier(x)
  Numeric 10

}
SwitchStatement {
  discriminant: Identifier(x)

  cases: [
    SwitchCase {
  test: Numeric 1

  consequent: [
    ExpressionStatement:
  Call
    Identifier(print)
    String one

    Break

  ]
}
    SwitchCase {
  test: Numeric 2

  consequent: [
    ExpressionStatement:
  Call
    Identifier(print)
    String two

    Break

  ]
}
    SwitchCase {
  test: default
  consequent: [
    ExpressionStatement:
  Call
    Identifier(print)
    String other

  ]
}
  ]
}
```

## Author

Chidume Nnamdi

- 📫 [Email](mailto:kurtwanger40@gmail.com)
- 💼 [LinkedIn](https://www.linkedin.com/in/chidume-nnamdi/)
- 📺 [YouTube - Understanding Dev](https://www.youtube.com/channel/UCUCHv7YOQXWy2dsL-0IrlPw)
