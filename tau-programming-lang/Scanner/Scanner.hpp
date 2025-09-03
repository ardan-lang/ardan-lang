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
    void reverse();
    void addToken(TokenType type);
    void addToken(TokenType type, string lexeme);
    char& currentCharacter();
    bool eof();
    bool isDigit();
    bool isAlpha();
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
    bool isAlphaNumeric();
    void collectString();
    void collectNumber();
    void collectIdentifier();
    bool isKeyword(const string& identifier);
    bool match(char str);
    char& peek();
    void collectLiteralString();
    void consumeComment();
    void consumeMultilineComment();
    
private:
    int current = 0;
    int line = 1;
    vector<Token> tokens;
    string& source;
    
};

#endif /* Scanner_hpp */
