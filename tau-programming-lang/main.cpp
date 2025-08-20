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
#include "Visitor/PrintVisitor.hpp"
#include "Parser/Parser.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    
    string _lang = R"(
                      
                      var mod = 8%5
                      var bl = true
                      var m %= 9
                    
                      var square = 9**9
                      var ref = ref1 == uiop
                      var d="nnamdi chidume"
                      var t= 90
                      var g= sqrt()
                      print(g,d,t,34)
                    )";
    
    string lang = "var t= 90;";

    Scanner scanner(_lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.value << endl;
    }
    
    Parser parser(scanner.getTokens());
    auto ast = parser.parse();
    
    PrintVisitor printer;
    
    for (auto& stmt : ast) {
        stmt->accept(printer);
    }
    
    return EXIT_SUCCESS;
}
