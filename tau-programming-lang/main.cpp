//
//  main.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include <iostream>
#include <cstring>
#include "Scanner/Scanner.hpp"
#include "overloads/operators.h"
#include "Parser/Parser.hpp"

#include "Visitor/AstPrinter/AstPrinter.h"
#include "Interpreter/Interpreter.h"

using namespace std;

int main(int argc, const char * argv[]) {
    
    string _lang = R"(
                      
                      var mod = 8%5;
                      var bl = true;
                      mod %= 9;
                    
                      var square = 9**9;
                      var ref = ref1 == uiop;
                      var d="nnamdi chidume";
                      var t= 90;
                      var g= sqrt();
                      print(g,d,t,34);
                    
                        m = 9; m+=8; i++; i--;
                        --i;
                        ++i;
                        call();

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

                        try {}
                        catch (e) {}
                        finally {}
                    class Person {
                      
                      var name = "Unknown";
                      var age = 0;

                      constructor(name, age) {
                        this.name = name; 
                        this.age = age;
                      }

                      greet() {
                        console.log(`Hi, I'm ${this.name} and I'm ${this.age} years old.`);
                      }
                    }

                    )";
    
    string __lang = R"(    
                    class Person {
                      
                      var name = "Unknown";
                      var age = 0;

                      constructor(name, age) {
                        this.name = name; 
                        this.age = age;
                      }

                      greet() {
                        console.log(`Hi, I'm ${this.name} and I'm ${this.age} years old.`);
                      }
                    }
    
    var person = new Person(67, pot);
    
    function getName(67, pot) {
    
    }
    function n(90, uiop, ui=90){}
    
    x = 10;
    callFunction(7, u, true, 89, false, "78", call());
    !w;
    ++i;
    i++;
    obj.method();
    obj.method;
    x;
    var y = 10;
    let t = 90 + 9;
    x += 9;
    (6, 7);
    function n(90, uiop, ui=90){}
    
                            try {
        x;
        var y = 10;
        let t = 90 + 9;
        x += 9;
        (6, 7);
        function n(90, uiop, ui=90){}

    }
                            catch (e) {
        x;
        var y = 10;
        let t = 90 + 9;
        x += 9;
        (6, 7);
        function n(90, uiop, ui=90){}

    }
                            finally {
        x;
        var y = 10;
        let t = 90 + 9;
        x += 9;
        (6, 7);
        function n(90, uiop, ui=90){}

    }
    class Person {
      
      public var name = "Unknown";
      static public var age = 0;

      constructor(name, age) {
        this.name = name;
        this.age = age;
      }

      private greet() {
        console.log(`Hi, I'm ${this.name} and I'm ${this.age} years old.`);
      }
    }
    for(let i = 0; i < 9; i++) {}
    
    if(true){}
    for (let value of iterable) {
      value += 1;
      console.log(value);
    }
        for (let key in person) {    }
        var t;
        for (let key in person) {    }
        for (let value of iterable) {
          value += 1;
          console.log(value);
        }
    for(let i = 0; i < 9; i++) {}
    for(i = 0; i < 9; i++) {}
    for(i; i < 9; i++) {}
    
    while(true) {}
    while(9<0) {}
    true;
    var arr = [8, 9];
    var user = {name: "chi", age: 90};
    function displayName() {
        var name = "Nnamdi Chidume";
        print(name, true);
    }

         function displayName(name) {
             print(name, true);
         }
                 var name = "Nnamdi Chidume";

         displayName(name);
         var i = 9;
         print(i);
         ++i;
         print(i);
         
         switch(i) {
         case 9: 
             print("9er");
         case 10:
             print("10er");
         default:
             print("default");
         }

    )";
    
    string lang = R"(
    
    class User {
    constructor() {
    super();
    }
    
    var name = "NN";
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
    
    print(user_obj.name);
    user_obj.getAge();
    print("----------------");
    for(let key in user_obj) {
    print(key);
    }
    print("----------------");
    
    var arr = [9,8,7,6];
    for(let key of arr) {
    print(key);
    }
    
    )";

    Scanner scanner(lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.lexeme << endl;
    }
    
    Parser parser(scanner.getTokens());
    vector<unique_ptr<Statement>> ast = parser.parse();
    
    AstPrinter printer;
    Interpreter interpreter;
    
    cout << endl;
    cout << "---------------" << endl;
    cout << "Interpreting..." << endl;
    cout << "---------------" << endl;
    cout << endl;

    interpreter.execute(std::move(ast));
    
    return EXIT_SUCCESS;
}
