//
//  Scanner.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#ifndef Scanner_hpp
#define Scanner_hpp

#include <stdio.h>
#include <iostream>
#include <cstring>
#include "Token/Token.hpp"

using namespace std;

class Scanner {
  
public:
    Scanner(string& source): source(source) {};
    vector<Token>& getTokens();
    void advance();
    void addToken(string type);
    void addToken(string type, string value);
    char& currentCharacter();
    
private:
    int current = 0;
    vector<Token> tokens;
    string& source;
    
};

#endif /* Scanner_hpp */
