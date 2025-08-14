//
//  Scanner.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include "Scanner.hpp"
#include <unordered_map>
#include <string>

const char* operators[] = {
    "=", "==", "===", "-", "-=", "*", "*=", "/", "/=", "+", "+=",
    ">", "<", ">=", "=<", "(", ")", "[", "]", "{", "}"
};

std::unordered_map<std::string, std::string> keywords = {
    
    // Control flow
    {"if", "IF"}, {"else", "ELSE"},
    {"switch", "SWITCH"}, {"case", "CASE"}, {"default", "DEFAULT"},
    {"for", "FOR"}, {"while", "WHILE"}, {"do", "DO"},
    {"break", "BREAK"}, {"continue", "CONTINUE"},
    {"return", "RETURN"}, {"throw", "THROW"},
    {"try", "TRY"}, {"catch", "CATCH"}, {"finally", "FINALLY"},

    // Declarations
    {"var", "VAR"}, {"let", "LET"}, {"const", "CONST"},
    {"function", "FUNCTION"}, {"class", "CLASS"}, {"extends", "EXTENDS"},
    {"import", "IMPORT"}, {"export", "EXPORT"},

    // Operators / expressions
    {"new", "NEW"}, {"delete", "DELETE"}, {"typeof", "TYPEOF"},
    {"instanceof", "INSTANCEOF"}, {"in", "IN"}, {"void", "VOID"},
    {"yield", "YIELD"}, {"await", "AWAIT"},

    // Literals / special
    {"true", "TRUE"}, {"false", "FALSE"}, {"null", "NULL"},
    {"this", "THIS"}, {"super", "SUPER"},

    // Reserved / future
    {"enum", "ENUM"}, {"implements", "IMPLEMENTS"}, {"interface", "INTERFACE"},
    {"package", "PACKAGE"}, {"private", "PRIVATE"}, {"protected", "PROTECTED"},
    {"public", "PUBLIC"}, {"static", "STATIC"},

    // Module-specific
    {"as", "AS"}, {"from", "FROM"}, {"of", "OF"}
};

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
                
            case '"':
                
                collectString();
                
                break;

            default:
                
                if (isDigit()) {
                    // collect number
                    collectNumber();
                    
                }
                
                if (isAlpha()) {
                    // collect identifier
                    collectIdentifier();
                }
                
                break;
        }
        
    }
    
    Token token;
    token.value = "EOF";
    
    tokens.push_back(token);
    
    return tokens;
    
}

void Scanner::collectString() {
    
    advance();
    
    string concat = "";
    
    while (currentCharacter() != '"' && !eof()) {
        concat += currentCharacter();
        advance();
    }
    
    addToken("STRING", concat);
    concat = "";
    advance();

}

void Scanner::collectNumber() {
    
    string num;
    
    while (isAlpha()) {

        num += currentCharacter();

        advance();
        
    }
    
    addToken("NUMBER", num);

}

void Scanner::collectIdentifier() {
    
    string identifier;
    
    while (isAlpha()) {
        
        identifier += currentCharacter();
        
        advance();
        
    }
    
    if(isKeyword(identifier)) {
        
        addToken(identifier);
        
    } else {
        
        addToken("IDENTIFIER", identifier);
        
    }

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

bool Scanner::isKeyword(const string& identifier) {
    return keywords.contains(identifier);
}

char& Scanner::currentCharacter() {
    return source[current - 1];
}

bool Scanner::isDigit() {
    
    char character = currentCharacter();
    
    return  character >= '0' && character <= '9';
}

bool Scanner::isAlpha() {
    
    char character = currentCharacter();

    return character >= 'a' && (character <= 'z' || character == '_');

}

bool Scanner::eof() {
    return current >= source.length();
}
