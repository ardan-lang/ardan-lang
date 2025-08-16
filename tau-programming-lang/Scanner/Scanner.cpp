//
//  Scanner.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include "Scanner.hpp"
#include <unordered_map>
#include <string>

char null = ' ';

const char* operators[] = {
    // Assignment
    "=", "+=", "-=", "*=", "/=", "%=", "**=", "<<=", ">>=", ">>>=",
    "&=", "^=", "|=", "&&=", "||=", "??=",

    // Arithmetic
    "+", "-", "*", "/", "%", "**", "++", "--",

    // Comparison
    "==", "!=", "===", "!==", ">", ">=", "<", "<=",

    // Logical
    "&&", "||", "!", "??",

    // Bitwise
    "&", "|", "^", "~", "<<", ">>", ">>>",

    // Other / Misc
    "typeof", "instanceof", "in", "void", "delete",
    "?", ":", ",", "...", ".", "?.", "[", "]", "(", ")", "{", "}"
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

            case '=':
                
                if (match('=')) {
                    
                    if (match('=')) {
                        
                        addToken("REFERENCE_EQUAL");
                        break;
                        
                    }
                    
                    addToken("VALUE_EQUAL");
                    break;
                    
                }
                
                addToken("ASSIGN");
                break;

            case '-':
                
                if (match('-')) {
                    
                    addToken("DECREMENT");
                    break;
                    
                }
                
                if (match('=')) {
                    
                    addToken("ASSIGN_MINUS");
                    break;
                    
                }
                
                addToken("MINUS");
                
                break;
                
            case ';':
                addToken("EOL");
                break;

            case '*':
                
                if (match('*')) {
                    
                    if (match('=')) {
                        addToken("SQUARE_ASSIGN");
                        break;
                    }
                    
                    addToken("SQUARE");
                    break;
                }
                
                if (match('=')) {
                    addToken("ASSIGN_MUL");
                    break;
                }
                
                addToken("MUL");
                break;

            case '/':
                
                if (match('=')) {
                    addToken("ASSIGN_DIV");
                    break;
                }
                
                addToken("DIV");
                break;

            case '+':
                
                if (match('+')) {
                    addToken("INCREMENT");
                    break;
                }
                
                if (match('=')) {
                    addToken("ASSIGN_ADD");
                    break;
                }
                
                addToken("ADD");
                break;
                
            case '%':

                if(match('=')) {
                    
                    addToken("MODULI_ADD");
                    break;
                    
                }
                
                addToken("MODULI");
                break;
                
            case '<':
                
                if(match('<')) {
                    
                    if(match('=')) {
                        
                        addToken("BITWISE_LEFT_SHIFT_ASSIGN");
                        break;
                        
                    }
                    
                    addToken("BITWISE_LEFT_SHIFT");
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken("LESS_THAN_ASSIGN");
                    break;
                    
                }
                
                addToken("LESS_THAN");
                break;

            case '>':
                
                if(match('>')) {

                    if(match('>')) {

                        if(match('=')) {
                            
                            addToken("UNSIGNED_RIGHT_SHIFT_ASSIGN");
                            break;
                            
                        }

                        addToken("UNSIGNED_RIGHT_SHIFT");
                        break;
                        
                    }
                    
                    if(match('=')) {
                        
                        addToken("BITWISE_RIGHT_SHIFT_ASSIGN");
                        break;
                        
                    }
                    
                    addToken("BITWISE_RIGHT_SHIFT");
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken("GREATER_THAN_ASSIGN");
                    break;
                    
                }
                
                addToken("GREATER_THAN");
                break;

            case '&':

                if(match('&')) {
                    
                    if(match('=')) {
                        
                        addToken("LOGICAL_AND_ASSIGN");
                        break;
                        
                    }
                    
                    addToken("LOGICAL_AND");
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken("BITWISE_AND_ASSIGN");
                    break;
                    
                }

                addToken("BITWISE_AND");
                break;
                
            case '|':
                
                if (match('|')) {
                    
                    if (match('=')) {
                        
                        addToken("LOGICAL_OR_ASSIGN");
                        break;
                        
                    }
                    
                    addToken("LOGICAL_OR");
                    break;
                    
                }
                
                if (match('=')) {
                    
                    addToken("BITWISE_OR_ASSIGN");
                    break;
                    
                }
                
                addToken("BITWISE_OR");
                break;
                
            case '!':
                
                if (match('=')) {
                    
                    if (match('=')) {
                        
                        addToken("STRICT_INEQUALITY");
                        break;
                        
                    }

                    addToken("INEQUALITY");
                    break;
                    
                }
                
                addToken("LOGICAL_NOT");
                break;
                
            case '?':
                
                if (match('?')) {
                    
                    if (match('=')) {
                        
                        addToken("NULLISH_COALESCING_ASSIGN");
                        break;
                        
                    }
                    
                    addToken("NULLISH_COALESCING");
                    break;
                    
                }
                
                if (match('.')) {
                    
                    addToken("OPTIONAL_CHAINING");
                    break;
                    
                }
                
                addToken("TERNARY");
                break;
                
            case '~':
                addToken("BITWISE_NOT");
                break;
                
            case '^':
                
                if (match('=')) {
                    
                    addToken("BITWISE_XOR_ASSIGN");
                    break;
                    
                }
                
                addToken("BITWISE_XOR");
                break;
                
            case '\t':
            case ' ':
            case '\r':
                break;
                
            case '\n':
                advance();
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
    
    while (isDigit()) {

        num += currentCharacter();

        advance();
        
    }
    
    addToken("NUMBER", num);
    
    reverse();

}

void Scanner::collectIdentifier() {
    
    string identifier;
    
    // TODO: refactor to contain variables with digits
    
    while (isAlpha() /* && (peek() >= '0' && peek() <= '9')*/) {
        
        identifier += currentCharacter();
        
        advance();
        
    }
    
    if(isKeyword(identifier)) {
        
        addToken(keywords[identifier], identifier);
        
    } else {
        
        addToken("IDENTIFIER", identifier);
        
    }
    
    reverse();

}

bool Scanner::match(char character) {

    if (character != peek()) {
        return false;
    }
    
    advance();
    return true;
    
}

void Scanner::advance() {
    current++;
}

void Scanner::reverse() {
    current--;
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
    return source[current];
}

bool Scanner::isDigit() {
    
    char character = currentCharacter();
    
    return  character >= '0' && character <= '9';
}

bool Scanner::isAlpha() {
    
    char character = currentCharacter();

    return character >= 'a' && (character <= 'z' || character == '_' || character == '$');

}

bool Scanner::eof() {
    return current >= source.length();
}

char& Scanner::peek() {
        
    if(eof()) {
        
        return null;
        
    }
    
    return source[current + 1];
    
}
