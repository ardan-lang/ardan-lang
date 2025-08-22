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
#include "Visitor/AstPrinter/AstPrinter.h"
#include "Parser/Parser.hpp"

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

                    )";
    
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

    )";

    Scanner scanner(lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.lexeme << endl;
    }
    
    Parser parser(scanner.getTokens());
    auto ast = parser.parse();
    
    AstPrinter printer;
    
    for (auto& stmt : ast) {
        stmt->accept(printer);
    }
    
    return EXIT_SUCCESS;
}
