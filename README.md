# Tau programming languague

Tau programming languague is written in C++.

## Scanning

```cpp
    string lang = R"(
                      var mod = 8%5;
                      var bl = true;
                      mod %= 9;
                      var square = 9**9;
                      var ref = ref1 == uiop;
                      var d="nnamdi chidume";
                      var t= 90;
                      var g= sqrt();
                      print(g,d,t,34);
                          (g,d,t,34);
    var x = (1, 2, 3);
    class Foo {
        greet(name, age) {
            print(name, age);
        }
    }
    continue;
    
    do {
       print("hello");
       throw "Hello";
    } while (x < 10);

    switch(x) {
       case 1: print("one"); break;
       case 2: print("two"); break;
       default: print("other");
    }


    )";

    Scanner scanner(lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.lexeme << endl;
    }

```

Output:

```sh
KEYWORD : VAR
IDENTIFIER : mod
ASSIGN : =
NUMBER : 8
MODULI : %
NUMBER : 5
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : bl
ASSIGN : =
BOOLEAN : TRUE
SEMI_COLON : 
IDENTIFIER : mod
MODULI_ASSIGN : %=
NUMBER : 9
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : square
ASSIGN : =
NUMBER : 9
SQUARE : 
NUMBER : 9
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : ref
ASSIGN : =
IDENTIFIER : ref1
VALUE_EQUAL : ==
IDENTIFIER : uiop
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : d
ASSIGN : =
STRING : nnamdi chidume
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : t
ASSIGN : =
NUMBER : 90
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : g
ASSIGN : =
IDENTIFIER : sqrt
LEFT_PARENTHESIS : 
RIGHT_PARENTHESIS : 
SEMI_COLON : 
IDENTIFIER : print
LEFT_PARENTHESIS : 
IDENTIFIER : g
COMMA : ,
IDENTIFIER : d
COMMA : ,
IDENTIFIER : t
COMMA : ,
NUMBER : 34
RIGHT_PARENTHESIS : 
SEMI_COLON : 
LEFT_PARENTHESIS : 
IDENTIFIER : g
COMMA : ,
IDENTIFIER : d
COMMA : ,
IDENTIFIER : t
COMMA : ,
NUMBER : 34
RIGHT_PARENTHESIS : 
SEMI_COLON : 
KEYWORD : VAR
IDENTIFIER : x
ASSIGN : =
LEFT_PARENTHESIS : 
NUMBER : 1
COMMA : ,
NUMBER : 2
COMMA : ,
NUMBER : 3
RIGHT_PARENTHESIS : 
SEMI_COLON : 
CLASS : CLASS
IDENTIFIER : oo
LEFT_BRACKET : 
IDENTIFIER : greet
LEFT_PARENTHESIS : 
IDENTIFIER : name
COMMA : ,
IDENTIFIER : age
RIGHT_PARENTHESIS : 
LEFT_BRACKET : 
IDENTIFIER : print
LEFT_PARENTHESIS : 
IDENTIFIER : name
COMMA : ,
IDENTIFIER : age
RIGHT_PARENTHESIS : 
SEMI_COLON : 
RIGHT_BRACKET : 
RIGHT_BRACKET : 
KEYWORD : CONTINUE
SEMI_COLON : 
KEYWORD : DO
LEFT_BRACKET : 
IDENTIFIER : print
LEFT_PARENTHESIS : 
STRING : hello
RIGHT_PARENTHESIS : 
SEMI_COLON : 
KEYWORD : THROW
STRING : Hello
SEMI_COLON : 
RIGHT_BRACKET : 
KEYWORD : WHILE
LEFT_PARENTHESIS : 
IDENTIFIER : x
LESS_THAN : 
NUMBER : 10
RIGHT_PARENTHESIS : 
SEMI_COLON : 
KEYWORD : SWITCH
LEFT_PARENTHESIS : 
IDENTIFIER : x
RIGHT_PARENTHESIS : 
LEFT_BRACKET : 
KEYWORD : CASE
NUMBER : 1
COLON : 
IDENTIFIER : print
LEFT_PARENTHESIS : 
STRING : one
RIGHT_PARENTHESIS : 
SEMI_COLON : 
KEYWORD : BREAK
SEMI_COLON : 
KEYWORD : CASE
NUMBER : 2
COLON : 
IDENTIFIER : print
LEFT_PARENTHESIS : 
STRING : two
RIGHT_PARENTHESIS : 
SEMI_COLON : 
KEYWORD : BREAK
SEMI_COLON : 
KEYWORD : DEFAULT
COLON : 
IDENTIFIER : print
LEFT_PARENTHESIS : 
STRING : other
RIGHT_PARENTHESIS : 
SEMI_COLON : 
RIGHT_BRACKET : 
END_OF_FILE : 
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
