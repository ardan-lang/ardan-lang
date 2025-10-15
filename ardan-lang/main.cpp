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

#include <filesystem>

#include "Scanner/Scanner.hpp"
#include "overloads/operators.h"
#include "Parser/Parser.hpp"

#include "Visitor/AstPrinter/AstPrinter.h"
#include "Interpreter/Interpreter.h"

#include "REPL/REPL.hpp"
#include "JSON/JSON.hpp"

#include "Compiler/CodeGenerator.hpp"
#include "Compiler/Compiler.hpp"

//#include "GUI/gui.h"
//#import <Cocoa/Cocoa.h>

string read_file(const string& filename);

using namespace std;
namespace fs = std::filesystem;

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
    
//    let greet = (firstName, lastName, ...otherInfo) => {
//      console.log(`Hello, ${firstName} ${lastName}!`);
//      console.log("Additional info:", otherInfo);
//    };
//
//    greet("John", "Doe", "Developer", "New York", "Single");
    
//    let a = [1,2,3];
//    let b = [a, [4,5], 6];
//    print(b[0][1], b[1][0]);
//
//    let o = {foo:123, bar:{baz:42}};
//    print(o.bar.baz);
//    o.bar.qux = [7,8];
//    print(o.bar.qux);
//    print(o.bar.qux[1]++);
//
//    let s = [];
//    s[2] = 9;
//    print(s[0], s[2], s.length);
//    
//    a.push(44); print(a);
//    a.pop(); print(a);
    
    let obj = {};
    obj["1"] = "one";
    obj[42] = "forty-two";
    print(obj[1], obj["42"]);


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

void create_ardan_project() {
    
    std::string projectName;
    std::cout << "Enter new Ardan project name: ";
    std::cin >> projectName;
    
    fs::path dirPath = fs::current_path() / projectName;
    
    try {
        // Create the directory
        if (!fs::exists(dirPath)) {
            fs::create_directory(dirPath);
            std::cout << "Created directory: " << dirPath << "\n";
        } else {
            std::cerr << "Directory already exists!\n";
            return;
        }
        
        // Create main.ardan file inside it
        fs::path filePath = dirPath / "main.ardan";
        std::ofstream ardanFile(filePath);
        if (ardanFile.is_open()) {
            ardanFile << "function main() {\n";
            ardanFile << "    print(\"Hello World\");\n";
            ardanFile << "}\n";
            ardanFile << "main();\n";
            ardanFile.close();
            std::cout << "Created file: " << filePath << "\n";
        } else {
            std::cerr << "Failed to create main.ardan\n";
        }
        
        // Create ardan.json file inside it
        fs::path ardanJsonFilePath = dirPath / "ardan.json";
        std::ofstream ardanJsonFile(ardanJsonFilePath);
        if (ardanJsonFile.is_open()) {
            ardanJsonFile << "{\n";
            ardanJsonFile << "    \"main\": \"main.ardan\"\n";
            ardanJsonFile << "}\n";
            ardanJsonFile.close();
            std::cout << "Created file: " << ardanJsonFilePath << "\n";
        } else {
            std::cerr << "Failed to create ardan.json\n";
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

fs::path findFileRecursive(const fs::path& root, const std::string& fileName) {
    for (auto& p : fs::recursive_directory_iterator(root)) {
        if (p.is_regular_file() && p.path().filename() == fileName) {
            return p.path();
        }
    }
    return {};
}

string get_entry_file() {
    
    string source;
    
    if (fs::exists(fs::current_path() / "ardan.json")) {
        
        fs::path projectRoot = fs::current_path();
        
        // read ardan.json
        ifstream file(fs::current_path() / "ardan.json");
        JSON json;
        
        auto data = json.readJson("ardan.json");
        
        if (data.empty()) {
            std::cerr << "No data found in ardan.json\n";
            throw runtime_error("No data found in ardan.json\n");
        }
        
        string entryFileName;
        
        for (const auto& [key, value] : data) {
            if (key == "main") {
                entryFileName = value;
            }
        }
        
        if (entryFileName.empty()) {
            cerr << "No main start script found.\n";
            throw runtime_error("No main start script found.\n");
        }
        
        // launch main
        // read the value from fs
        
        // Search for entry file anywhere in the project dir
        fs::path entryPath = findFileRecursive(projectRoot, entryFileName);
        if (!entryPath.empty()) {
            
            source = read_file(entryPath);
            // run_interpreter(entryFileName, source);
            
        } else {
            std::cerr << "Could not find " << entryFileName << " in project.\n";
        }
        
        
    }
    else {
        throw runtime_error("Cannot find ardan.json file. Make sure this is an Ardan project.");
    }
    
    return source;
    
}

vector<unique_ptr<Statement>> get_ast(string source, string filename) {
    
    Scanner scanner(source);
    
    auto tokens = scanner.getTokens();

    Parser parser(tokens);
    parser.sourceFile = filename;
    
    auto ast = parser.parse();
    
    return ast;

}

void run_interpreter(string& filename, string& source) {
    
    auto ast = get_ast(source, filename);
    
    Interpreter interpreter;
    interpreter.execute(std::move(ast));

}

void run_compiler(string& filename, string& source) {
    
    auto ast = get_ast(source, filename);

    Compiler compiler;
    
    auto module_ = compiler.compile(ast);
    
}

void test() {
    
    string entryFileName = "/Users/chidumennamdi/Documents/MacBookPro2020/developerse/xcode-prjs/ardan-lang/ardan-lang/tests/server.ardan";
        
    string source = read_file(entryFileName);
    auto ast = get_ast(source, entryFileName);
    
    Compiler compiler;
    
    // compiler.test_turbo_compile(ast);
    compiler.test_compile(ast);

    // run_interpreter(entryFileName, source);
    // showWindow();
    // run_interpreter_inline_test();
    
    // REPL repl;
    // repl.start_repl();
        
}

int main(int argc, const char * argv[]) {
    
//    test();
//    return 0;
    
    bool interpret = false;
    bool compile = false;
    bool compile_run = false;
    bool repl_it = false;
    bool new_project = false;
    string e;
    
    string filename;

    for (int i = 1; i < argc; i++) {

        string param = argv[i];
                        
        if (param == "--i" || param == "--interpret") {
            interpret = true;
        } else if (param == "--c" || param == "--compile") {
            compile = true;
        } else if (param == "--r" || param == "--repl") {
            repl_it = true;
        } else if (param == "--new" || param == "--n") {
            new_project = true;
        } else if (param == "--compile_run" || param == "--cr") {
            compile_run = true;
        } if (param.find("--e=") == 0) {
            e = param.substr(4); // after "--e="
        } else if (param == "--e" && (i+1 < argc)) {
            e = argv[++i];
        } else {
            filename = param;
        }
        
    }
            
    if (interpret) {
        
        string source = read_file(filename);
        run_interpreter(filename, source);
        
    } else if (compile) {
        
        // find the entry file
        // then compile to .ardar
        
        // ardan --compile
        // ardan --compile --e=main.ardan

        Compiler compiler;
        string source;

        if (!e.empty()) {
            
            // there is entry file: --e set
            filename = e;
            source = read_file(filename);

        } else {
            
            // no entry file. figure it out.
            source = get_entry_file();
            
        }
                
        auto ast = get_ast(source, filename);
        auto module_ = compiler.compile(ast);
        string outputFilename = "bin.ardar";
        compiler
            .write_ardar(outputFilename, module_, module_->entryChunkIndex);

    } else if (compile_run) {

        // ardan -compile-and-run
        // ardan -compile-and-run --e=main.ardan

        Compiler compiler;
        string source;

        if (!e.empty()) {
            filename = e;
            source = read_file(filename);
        } else {
            source = get_entry_file();
        }
        
        auto ast = get_ast(source, filename);
        auto module_ = compiler.compile(ast);
        string outputFilename = "bin.ardar";
        compiler
            .write_ardar(outputFilename, module_, module_->entryChunkIndex);
        
        auto module_compiled = compiler.read_ardar(outputFilename);
        compiler.run(module_compiled);

    } else if (new_project) {
        
        create_ardan_project();
        
    } else if (repl_it) {

        REPL repl;
        repl.start_repl();

    } else {
        
        // if dir is a ardan project
        if (fs::exists(fs::current_path() / "ardan.json")) {
            
            fs::path projectRoot = fs::current_path();
                        
            // read ardan.json
            ifstream file(fs::current_path() / "ardan.json");
            JSON json;
            
            auto data = json.readJson("ardan.json");
            
            if (data.empty()) {
                std::cerr << "No data found in ardan.json\n";
                return 1;
            }
            
            string entryFileName;
            
            for (const auto& [key, value] : data) {
                if (key == "main") {
                    entryFileName = value;
                }
            }
            
            if (entryFileName.empty()) {
                cerr << "No main start script found.\n";
                return 1;
            }
            
            // launch main
            // read the value from fs
            
            // Search for entry file anywhere in the project dir
                fs::path entryPath = findFileRecursive(projectRoot, entryFileName);
                if (!entryPath.empty()) {
                    
                    string source = read_file(entryPath);
                    run_interpreter(entryFileName, source);

                } else {
                    std::cerr << "Could not find " << entryFileName << " in project.\n";
                }
            
            
        } else {
            
            REPL repl;
            repl.start_repl();
        }

    }
    
    return EXIT_SUCCESS;
    
}
