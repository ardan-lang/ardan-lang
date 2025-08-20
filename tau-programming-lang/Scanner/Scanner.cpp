//
//  Scanner.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#include "Scanner.hpp"
#include <unordered_map>
#include <string>
#include "Token/TokenType.h"

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
                addToken(TokenType::COMMA);
                break;
                
            case '.':
                
                if (match('.')) {
                    
                    if (match('.')) {
                        
                        addToken(TokenType::SPREAD);
                        break;
                        
                    }
                    
                }
                
                addToken(TokenType::DOT);
                break;
                
            case ':':
                
                addToken(TokenType::SEMI_COLON);
                break;

            case '(':
                addToken(TokenType::LEFT_PARENTHESIS);
                break;

            case ')':
                addToken(TokenType::RIGHT_PARENTHESIS);
                break;

            case '{':
                addToken(TokenType::LEFT_BRACKET);
                break;

            case '}':
                addToken(TokenType::RIGHT_BRACKET);
                break;
                
            case '[':
                
                addToken(TokenType::LEFT_SQUARE_BRACKET);
                break;
                
            case ']':
                
                addToken(TokenType::RIGHT_SQUARE_BRACKET);
                break;

            case '=':
                
                if (match('=')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::REFERENCE_EQUAL);
                        break;
                        
                    }
                    
                    addToken(TokenType::VALUE_EQUAL);
                    break;
                    
                }
                
                addToken(TokenType::ASSIGN);
                break;

            case '-':
                
                if (match('-')) {
                    
                    addToken(TokenType::DECREMENT);
                    break;
                    
                }
                
                if (match('=')) {
                    
                    addToken(TokenType::ASSIGN_MINUS);
                    break;
                    
                }
                
                addToken(TokenType::MINUS);
                
                break;
                
            case ';':
                addToken(TokenType::EOL);
                break;

            case '*':
                
                if (match('*')) {
                    
                    if (match('=')) {
                        addToken(TokenType::POWER_ASSIGN);
                        break;
                    }
                    
                    addToken(TokenType::POWER);
                    break;
                }
                
                if (match('=')) {
                    addToken(TokenType::ASSIGN_MUL);
                    break;
                }
                
                addToken(TokenType::MUL);
                break;

            case '/':
                
                if (match('=')) {
                    addToken(TokenType::ASSIGN_DIV);
                    break;
                }
                
                addToken(TokenType::DIV);
                break;

            case '+':
                
                if (match('+')) {
                    addToken(TokenType::INCREMENT);
                    break;
                }
                
                if (match('=')) {
                    addToken(TokenType::ASSIGN_ADD);
                    break;
                }
                
                addToken(TokenType::ADD);
                break;
                
            case '%':

                if(match('=')) {
                    
                    addToken(TokenType::MODULI_ADD);
                    break;
                    
                }
                
                addToken(TokenType::MODULI);
                break;
                
            case '<':
                
                if(match('<')) {
                    
                    if(match('=')) {
                        
                        addToken(TokenType::BITWISE_LEFT_SHIFT_ASSIGN);
                        break;
                        
                    }
                    
                    addToken(TokenType::BITWISE_LEFT_SHIFT);
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken(TokenType::LESS_THAN_EQUAL);
                    break;
                    
                }
                
                addToken(TokenType::LESS_THAN);
                break;

            case '>':
                
                if(match('>')) {

                    if(match('>')) {

                        if(match('=')) {
                            
                            addToken(TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN);
                            break;
                            
                        }

                        addToken(TokenType::UNSIGNED_RIGHT_SHIFT);
                        break;
                        
                    }
                    
                    if(match('=')) {
                        
                        addToken(TokenType::BITWISE_RIGHT_SHIFT_ASSIGN);
                        break;
                        
                    }
                    
                    addToken(TokenType::BITWISE_RIGHT_SHIFT);
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken(TokenType::GREATER_THAN_EQUAL);
                    break;
                    
                }
                
                addToken(TokenType::GREATER_THAN);
                break;

            case '&':

                if(match('&')) {
                    
                    if(match('=')) {
                        
                        addToken(TokenType::LOGICAL_AND_ASSIGN);
                        break;
                        
                    }
                    
                    addToken(TokenType::LOGICAL_AND);
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken(TokenType::BITWISE_AND_ASSIGN);
                    break;
                    
                }

                addToken(TokenType::BITWISE_AND);
                break;
                
            case '|':
                
                if (match('|')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::LOGICAL_OR_ASSIGN);
                        break;
                        
                    }
                    
                    addToken(TokenType::LOGICAL_OR);
                    break;
                    
                }
                
                if (match('=')) {
                    
                    addToken(TokenType::BITWISE_OR_ASSIGN);
                    break;
                    
                }
                
                addToken(TokenType::BITWISE_OR);
                break;
                
            case '!':
                
                if (match('=')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::STRICT_INEQUALITY);
                        break;
                        
                    }

                    addToken(TokenType::INEQUALITY);
                    break;
                    
                }
                
                addToken(TokenType::LOGICAL_NOT);
                break;
                
            case '?':
                
                if (match('?')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::NULLISH_COALESCING_ASSIGN);
                        break;
                        
                    }
                    
                    addToken(TokenType::NULLISH_COALESCING);
                    break;
                    
                }
                
                if (match('.')) {
                    
                    addToken(TokenType::OPTIONAL_CHAINING);
                    break;
                    
                }
                
                addToken(TokenType::TERNARY);
                break;
                
            case '~':
                addToken(TokenType::BITWISE_NOT);
                break;
                
            case '^':
                
                if (match('=')) {
                    
                    addToken(TokenType::BITWISE_XOR_ASSIGN);
                    break;
                    
                }
                
                addToken(TokenType::BITWISE_XOR);
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

            case '`':
                collectLiteralString();
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
    
    addToken(TokenType::END_OF_FILE);
    
    return tokens;
    
}

void Scanner::collectString() {
    
    advance();
    
    string concat = "";
    
    while (currentCharacter() != '"' && !eof()) {
        concat += currentCharacter();
        advance();
    }
    
    addToken(TokenType::STRING, concat);
    concat = "";
    advance();

}

void Scanner::collectLiteralString() {
    
    advance();
    
    string concat = "";
    
    while (currentCharacter() != '`' && !eof()) {
        concat += currentCharacter();
        advance();
    }
    
    addToken(TokenType::STRING, concat);
    concat = "";
    advance();

}

void Scanner::collectNumber() {
    
    string num;
    
    while (isDigit()) {

        num += currentCharacter();

        advance();
        
    }
    
    addToken(TokenType::NUMBER, num);
    
    reverse();

}

void Scanner::collectIdentifier() {
    
    string identifier;
        
    while (isAlpha() || isDigit()) {
        
        identifier += currentCharacter();
        
        advance();
        
    }
    
    if(isKeyword(identifier)) {
        
        addToken(TokenType::KEYWORD, keywords[identifier]);

    } else {
        
        addToken(TokenType::IDENTIFIER, identifier);
        
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

void Scanner::addToken(TokenType type) {
    
    Token token;
    token.type = type;
    
    tokens.push_back(token);

}

void Scanner::addToken(TokenType type, string value) {
    
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
