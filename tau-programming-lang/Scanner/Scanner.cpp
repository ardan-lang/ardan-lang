//
//  Scanner.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include "Scanner.hpp"

vector<Token>& Scanner::getTokens() {

    while(current < source.length()) {
        
        advance();
        char& character = currentCharacter();
        
        switch (character) {
                
            case ',':
                addToken("COMMA");
                break;

            case '(':
                addToken("LEFT_PARENTHESIS");
                break;

            case ')':
                addToken("RIGHT_PARENTHESIS");
                break;

            case '{':
                addToken("LEFT_BRACKET");
                break;

            case '}':
                addToken("RIGHT_BRACKET");
                break;

            default:
                break;
        }
        
    }
    
    Token token;
    token.value = "EOF";
    
    tokens.push_back(token);
    
    return tokens;
    
}

void Scanner::advance() {
    current++;
}

void Scanner::addToken(string type) {
    
    Token token;
    token.type = type;
    
    tokens.push_back(token);
    
}

void Scanner::addToken(string type, string value) {
    
    Token token;
    token.type = type;
    token.value = value;
    
    tokens.push_back(token);
}

char& Scanner::currentCharacter() {
    return source[current - 1];
}
