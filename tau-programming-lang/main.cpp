//
//  main.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include <iostream>
#include <cstring>
#include "Scanner/Scanner.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    
    string lang = R"(
                      var d="nnamdi chidume"
                      var t= 90
                      var g= sqrt()
                      print(g,d,t,34)

                      )";
    
    Scanner scanner(lang);
    for(Token token : scanner.getTokens()) {
        cout << token.type << " : " << token.value << endl;
    }
    
    return EXIT_SUCCESS;
}
