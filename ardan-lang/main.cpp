//
//  main.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

#include "Scanner/Scanner.hpp"
#include "overloads/operators.h"
#include "Parser/Parser.hpp"

#include "Visitor/AstPrinter/AstPrinter.h"
#include "Interpreter/Interpreter.h"

#include "REPL/REPL.hpp"

string read_file(const string& filename);

using namespace std;

void run_interpreter_inline_test() {
    
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
    
    string lang = R"(
    
//    var outside_name = "outside";
//    
//    class Parent {
//        var p = 90;
//    }
//    
//    class User extends Parent {
//    constructor() {
//    super();
//    }
//    
//    var name = outside_name;
//    var age = 90;
//    getAge() {
//    print(this.age);
//    this.displayName();
//    }
//    
//    displayName() {
//    print(this.name);
//    }
//    
//    }
//    
//    var user_obj = new User();
//    
//    print(user_obj.p, 90, 80, 7);
//    
//    var a = false, b = false, c = false;
//    
//    const d = 900;
//    let e = 80;
//    
//    print(d, e);
//    
//    if (a) {
//    print(a);
//    }
//    else if (b) {
//    print(b);
//    }
//    else if (c) {
//    print(c);    
//    }
//    else {
//        print("else");
//    }
    
//    let test = 90;
//    
//    function square(n) {
//        return n * n;
//    }

    //print(Math.pow(2,3), square(5), test);
//    console.log(Math.pow(2,3), square(5), test);
//    console.log(Math.pow(2,3), square(5), test);
    
//    const fs = require('fs');
//
    //    fs.readFile('input.txt', 'utf8', (err, data) => {
    //        if (err) {
    //            console.error('Error reading file:', err);
    //            return;
    //        }
    //        console.log('File content:', data);
    //    });
    
    let v = 90;
    
    //let func = (x, y) => { return x * y; };
    
    //print(func(v, 90));
    //print(((x, y) => { return x*y; })(2, 8));
    
//    class Parent {
//
//        var parentName = "PP";
//
//        testParent() {
//            print(this.name);
//        }
//    
//    }
//    
//    class User extends Parent {
//        public var name = "NN";
//        private static var age = -8.9;
//    
//        change(){
//    
//            print("--------------");
//            this["name"] = "changed to nameee";
//            this["age"]++;
//            print(this["age"]);
//            User.age = 4 + User.age;
//            print(User.age);
//            print("----------end----------");
//    
//        }
//    }
//    
//    const user_obj = new User();
//    print(user_obj.parentName);
//    user_obj.testParent();
//    
//    user_obj.name = "CC";
//    print(user_obj.name);
//      user_obj.testParent();
//      user_obj.change();
//      print(user_obj.name);
//      print(User.age++);
//      print(--User.age);
      //print(User.age);
//    let name = "NChi";
//    let n = `Hello ${name}, you are ${1 + 2}${1 + 2} years old`;
//    print(n);
    
//    class Point {
//        var x;
//        var y;
//        constructor(x, y) {
//            this.x = x;
//            this.y = y;
//        }
//        move(dx, dy) {
//            this.x += dx;
//            this.y += dy;
//        }
//    }

//    let pt = new Point(0, 0);
//    pt.move(3, 4);
//    print(pt.x, pt.y); // 3, 4
    
    let greet = (firstName, lastName, ...otherInfo) => {
      console.log(`Hello, ${firstName} ${lastName}!`);
      console.log("Additional info:", otherInfo);
    };

    greet("John", "Doe", "Developer", "New York", "Single");
    
    )";

    Scanner scanner(lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.lexeme << " Line: " << token.line << endl;
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

}

void run_interpreter(string& filename, string& source) {
    
    Scanner scanner(source);
    
    auto tokens = scanner.getTokens();

    Parser parser(tokens);
    parser.sourceFile = filename;
    
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(std::move(ast));

}

int main(int argc, const char * argv[]) {
        
//    REPL repl;
//    repl.start_repl();
    
//    return 0;
    
    bool interpret = false;
    bool compile = false;
    bool repl_it = false;
    
    string filename;

    for (int i = 1; i < argc; i++) {

        string param = argv[i];
                
        if (param == "--i" || param == "--interpret") {
            interpret = true;
        } else if (param == "--c" || param == "--compile") {
            compile = true;
        } else if (param == "--r" || param == "--repl") {
            repl_it = true;
        } else {
            filename = param;
        }
        
    }
            
    if (interpret) {
        
        string source = read_file(filename);
        run_interpreter(filename, source);
        
    } else if (compile) {
        
    } else if (repl_it) {

        REPL repl;
        repl.start_repl();

    } else {
        
        REPL repl;
        repl.start_repl();

        // run_interpreter_inline_test();
    }
    
    return EXIT_SUCCESS;
    
}
