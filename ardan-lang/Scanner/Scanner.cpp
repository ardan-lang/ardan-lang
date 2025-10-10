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

using namespace std;

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
    {"yield", "YIELD"}, {"await", "AWAIT"}, {"async", "ASYNC"},

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

        char& character = currentCharacter();
        
        switch (character) {

            case ',':
                addToken(TokenType::COMMA, ",");
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
                
                addToken(TokenType::COLON);
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
                        
                        addToken(TokenType::REFERENCE_EQUAL, "===");
                        break;
                        
                    }
                    
                    addToken(TokenType::VALUE_EQUAL, "==");
                    break;
                    
                }
                
                if (match('>')) {
                    addToken(TokenType::ARROW, "=>");
                    break;
                }
                
                addToken(TokenType::ASSIGN, "=");
                break;

            case '-':
                
                if (match('-')) {
                    
                    addToken(TokenType::DECREMENT, "--");
                    break;
                    
                }
                
                if (match('=')) {
                    
                    addToken(TokenType::ASSIGN_MINUS, "-=");
                    break;
                    
                }
                
                addToken(TokenType::MINUS);
                
                break;
                
            case ';':
                addToken(TokenType::SEMI_COLON);
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
                
                if (match('*')) {
                    if (match('*')) {
                        // we are in a multi-line comment
                        
                        consumeMultilineComment();
                        break;
                        
                    }
                    
                    consumeMultilineComment();
                    break;
                    
                }

                if (match('/')) {
    
                    // we have a comment.
                    // loop till we hit /n
                    consumeComment();
                    break;
                    
                }
                
                if (match('=')) {
                    addToken(TokenType::ASSIGN_DIV, "/=");
                    break;
                }
                
                addToken(TokenType::DIV, "/");
                break;

            case '+':
                
                if (match('+')) {
                    addToken(TokenType::INCREMENT, "++");
                    break;
                }
                
                if (match('=')) {
                    addToken(TokenType::ASSIGN_ADD, "+=");
                    break;
                }
                
                addToken(TokenType::ADD, "+");
                break;
                
            case '%':

                if(match('=')) {
                    
                    addToken(TokenType::MODULI_ASSIGN, "%=");
                    break;
                    
                }
                
                addToken(TokenType::MODULI, "%");
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
                    
                    addToken(TokenType::GREATER_THAN_EQUAL, ">=");
                    break;
                    
                }
                
                addToken(TokenType::GREATER_THAN, ">");
                break;

            case '&':

                if(match('&')) {
                    
                    if(match('=')) {
                        
                        addToken(TokenType::LOGICAL_AND_ASSIGN, "&&=");
                        break;
                        
                    }
                    
                    addToken(TokenType::LOGICAL_AND, "&&");
                    break;
                    
                }
                
                if(match('=')) {
                    
                    addToken(TokenType::BITWISE_AND_ASSIGN, "&=");
                    break;
                    
                }

                addToken(TokenType::BITWISE_AND, "&");
                break;
                
            case '|':
                
                if (match('|')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::LOGICAL_OR_ASSIGN, "||=");
                        break;
                        
                    }
                    
                    addToken(TokenType::LOGICAL_OR, "||");
                    break;
                    
                }
                
                if (match('=')) {
                    
                    addToken(TokenType::BITWISE_OR_ASSIGN, "|=");
                    break;
                    
                }
                
                addToken(TokenType::BITWISE_OR, "|");
                break;
                
            case '!':
                
                if (match('=')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::STRICT_INEQUALITY, "!==");
                        break;
                        
                    }

                    addToken(TokenType::INEQUALITY, "!=");
                    break;
                    
                }
                
                addToken(TokenType::LOGICAL_NOT, "!");
                break;
                
            case '?':
                
                if (match('?')) {
                    
                    if (match('=')) {
                        
                        addToken(TokenType::NULLISH_COALESCING_ASSIGN, "??=");
                        break;
                        
                    }
                    
                    addToken(TokenType::NULLISH_COALESCING, "??");
                    break;
                    
                }
                
                if (match('.')) {
                    
                    addToken(TokenType::OPTIONAL_CHAINING, "?.");
                    break;
                    
                }
                
                addToken(TokenType::TERNARY, "?");
                break;
                
            case '~':
                addToken(TokenType::BITWISE_NOT, "~");
                break;
                
            case '^':
                
                if (match('=')) {
                    
                    addToken(TokenType::BITWISE_XOR_ASSIGN, "^=");
                    break;
                    
                }
                
                addToken(TokenType::BITWISE_XOR, "^");
                break;
                
            case '\t':
            case ' ':
            case '\r':
                break;
                
            case '\n':
                line++;
                break;

            case '\'':
                
                collectSingleQuoteString();
                
                break;
                
            case '"':
                
                collectString();
                
                break;

            case '`':
                collectLiteralString();
                current--;
                break;
                
            default:
                
                if (isDigit()) {
                    
                    // collect number
                    collectNumber();
                    break;
                }
                
                if (isAlpha()) {
                    
                    // collect identifier
                    collectIdentifier();
                    break;
                    
                }
                
                break;
        }
        
        advance();

    }
    
    addToken(TokenType::END_OF_FILE);
    
    return tokens;
    
}

void Scanner::collectSingleQuoteString() {
    
    advance();
    
    string concat = "";
        
    while (currentCharacter() != '\'' && !eof()) {
        concat += currentCharacter();
        advance();
    }
    
    addToken(TokenType::STRING, concat);
    concat = "";

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

}

//void Scanner::collectLiteralString() {
//    
//    advance();
//    
//    string concat = "";
//    
//    bool isInterpolstioin = false;
//    string inerpolationString;
//    
//    addToken(TokenType::TEMPLATE_START);
//    
//    while (currentCharacter() != '`' && !eof()) {
//        char cc = currentCharacter();
//        
//        // check if cc is $
//        if (cc == '$' && isInterpolstioin == false) {
//            isInterpolstioin = true;
//            advance();
//            addToken(TokenType::TEMPLATE_CHUNK, concat);
//            addToken(TokenType::INTERPOLATION_START);
//            concat = "";
//            continue;
//        }
//        
//        if (cc == '{' && isInterpolstioin == true) {
//            advance();
//            continue;
//        }
//        
//        if (cc == '}' && isInterpolstioin == true) {
//            isInterpolstioin = false;
//            
//            Scanner scanner(inerpolationString);
//            vector<Token> local_tokens = scanner.getTokens();
//            
//            for(Token local_token : local_tokens) {
//                if (local_token.type == TokenType::END_OF_FILE) {
//                    continue;
//                }
//                tokens.push_back(local_token);
//            }
//            
//            addToken(TokenType::INTERPOLATION_END);
//            advance();
//            
//            inerpolationString = "";
//            continue;
//        }
//                
//        if (isInterpolstioin) {
//            inerpolationString += cc;
//        } else {
//            concat += cc;
//        }
//        
//        advance();
//    }
//    
//    addToken(TokenType::TEMPLATE_END);
//    
//    concat = "";
//
//}

void Scanner::collectLiteralString() {
    advance(); // Skip the opening `
    std::string chunk = "";
    addToken(TokenType::TEMPLATE_START);
    
    while (!eof()) {
        char c = currentCharacter();
        
        if (c == '\\') {
            advance(); // consume '\'
            if (eof()) break;

            char esc = currentCharacter();

            switch (esc) {
                // ----- Single-character escapes -----
                case 'b': chunk += '\b'; advance(); break;
                case 'f': chunk += '\f'; advance(); break;
                case 'n': chunk += '\n'; advance(); break;
                case 'r': chunk += '\r'; advance(); break;
                case 't': chunk += '\t'; advance(); break;
                case 'v': chunk += '\v'; advance(); break;
                case '0':
                    // Only \0 if not followed by digit
                    if (isdigit(peek())) {
                        // Invalid escape in template → preserve literally
                        chunk += "\\0";
                        advance();
                    } else {
                        chunk += '\0';
                        advance();
                    }
                    break;
                case '\\': chunk += '\\'; advance(); break;
                case '\'': chunk += '\''; advance(); break;
                case '\"': chunk += '\"'; advance(); break;
                case '`':  chunk += '`';  advance(); break;

                // ----- Hexadecimal escape -----
                case 'x': {
                    advance();
                    if (isHexDigit(currentCharacter()) && isHexDigit(peek())) {
                        char h1 = currentCharacter(); advance();
                        char h2 = currentCharacter(); advance();
                        int value = hexToInt(h1) * 16 + hexToInt(h2);
                        chunk += static_cast<char>(value);
                    } else {
                        // Invalid → preserve literally
                        chunk += "\\x";
                    }
                    break;
                }

                // ----- Unicode escape -----
                case 'u': {
                    advance();
                    if (currentCharacter() == '{') {
                        // \u{X...X}
                        advance();
                        std::string hexDigits = "";
                        while (!eof() && currentCharacter() != '}') {
                            if (!isHexDigit(currentCharacter())) break;
                            hexDigits += currentCharacter();
                            advance();
                        }
                        if (currentCharacter() == '}') advance(); // consume '}'

                        if (!hexDigits.empty()) {
                            int codepoint = std::stoi(hexDigits, nullptr, 16);
                            // Encode to UTF-8
                            chunk += encodeUTF8(codepoint);
                        } else {
                            chunk += "\\u{}"; // invalid
                        }
                    } else {
                        // \uXXXX form
                        std::string hexDigits = "";
                        for (int i = 0; i < 4 && isHexDigit(currentCharacter()); i++) {
                            hexDigits += currentCharacter();
                            advance();
                        }
                        if (hexDigits.size() == 4) {
                            int codepoint = std::stoi(hexDigits, nullptr, 16);
                            chunk += encodeUTF8(codepoint);
                        } else {
                            chunk += "\\u" + hexDigits; // invalid
                        }
                    }
                    break;
                }

                // ----- Line continuation -----
                case '\n': case '\r': {
                    // Skip both backslash and line terminator → no char added
                    if (esc == '\r' && peek() == '\n') advance(); // CRLF
                    advance();
                    break;
                }

                // ----- Invalid escapes -----
                default:
                    // In template literals, invalid escapes must be preserved
                    chunk += '\\';
                    chunk += esc;
                    advance();
                    break;
            }
            continue;
        }

        if (c == '`') {
            if (!chunk.empty()) {
                addToken(TokenType::TEMPLATE_CHUNK, chunk);
                chunk = "";
            }
            addToken(TokenType::TEMPLATE_END);
            // we are at the end of the template literal.
            //if (peek() != ';') {
                advance(); // Skip closing `
            //}
            return;
        }
        
        if (c == '$' && peek() == '{') {
            if (!chunk.empty()) {
                addToken(TokenType::TEMPLATE_CHUNK, chunk);
                chunk = "";
            }
            addToken(TokenType::INTERPOLATION_START);
            advance(); // skip $
            advance(); // skip {

            int braceDepth = 1;
            std::string expr = "";
            while (!eof() && braceDepth > 0) {
                char cc = currentCharacter();
                if (cc == '{') {
                    braceDepth++;
                    expr += cc;
                    advance();
                } else if (cc == '}') {
                    braceDepth--;
                    if (braceDepth == 0) {
                        advance(); // skip closing }
                        break;
                    } else {
                        expr += cc;
                        advance();
                    }
                } else if (cc == '`') {
                    // Nested template literal! Recursively scan
                    collectLiteralString();
                } else {
                    expr += cc;
                    advance();
                }
            }
            // Scan the interpolation contents as its own token stream
            if (!expr.empty()) {
                Scanner interpScanner(expr);
                std::vector<Token> interpTokens = interpScanner.getTokens();
                for (const Token& t : interpTokens) {
                    if (t.type != TokenType::END_OF_FILE) {
                        tokens.push_back(t);
                    }
                }
            }
            addToken(TokenType::INTERPOLATION_END);
            continue;
        }
        
        chunk += c;
        advance();
    }
    // Unterminated template literal: emit what we have
    if (!chunk.empty()) {
        addToken(TokenType::TEMPLATE_CHUNK, chunk);
    }
    addToken(TokenType::TEMPLATE_END);
}

void Scanner::collectNumber() {
    string num;
    char c = currentCharacter();

    // --- Hexadecimal ---
    if (c == '0' && (peek() == 'x' || peek() == 'X')) {
        num += c; advance();
        num += currentCharacter(); advance();
        while (isxdigit(currentCharacter()) && !eof()) {
            num += currentCharacter();
            advance();
        }
        addToken(TokenType::NUMBER, num);
        reverse();
        return;
    }

    // --- Binary ---
    if (c == '0' && (peek() == 'b' || peek() == 'B')) {
        num += c; advance();
        num += currentCharacter(); advance();
        while ((currentCharacter() == '0' || currentCharacter() == '1')  && !eof()) {
            num += currentCharacter();
            advance();
        }
        addToken(TokenType::NUMBER, num);
        reverse();
        return;
    }

    // --- Octal ---
    if (c == '0' && (peek() == 'o' || peek() == 'O')) {
        num += c; advance();
        num += currentCharacter(); advance();
        while (currentCharacter() >= '0' && currentCharacter() <= '7'  && !eof()) {
            num += currentCharacter();
            advance();
        }
        addToken(TokenType::NUMBER, num);
        reverse();
        return;
    }

    // --- Decimal or Float ---
    bool hasDot = false;
    while ((isDigit() || currentCharacter() == '.')  && !eof()) {
        if (currentCharacter() == '.') {
            if (hasDot) break; // second dot → stop
            hasDot = true;
        }
        num += currentCharacter();
        advance();
    }

    // --- Scientific notation ---
    if (currentCharacter() == 'e' || currentCharacter() == 'E') {
        num += currentCharacter();
        advance();
        if (currentCharacter() == '+' || currentCharacter() == '-') {
            num += currentCharacter();
            advance();
        }
        while (isDigit() && !eof()) {
            num += currentCharacter();
            advance();
        }
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
        
        std::string kw = keywords[identifier];

        if (kw == "TRUE" || kw == "FALSE") {
            addToken(TokenType::BOOLEAN, kw);
        } else if (kw == "CLASS") {
            addToken(TokenType::CLASS, kw);
        } else {
            addToken(TokenType::KEYWORD, kw);
        }

    } else {
        
        addToken(TokenType::IDENTIFIER, identifier);
        
    }
    
    reverse();

}

void Scanner::consumeMultilineComment() {
    
    while (true && !eof()) {

        if (match('*')) {
            
            if (match('/')) {
                break;
            }

            if (match('*')) {
                if (match('/')) {
                    break;
                }
            }
        }

        advance();
        
    }
    
}

void Scanner::consumeComment() {
    
    while (peek() != '\n' && !eof()) {
        advance();
    }
    
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
    token.line = line;
    
    tokens.push_back(token);

}

void Scanner::addToken(TokenType type, string lexeme) {
    
    Token token;
    token.type = type;
    token.lexeme = lexeme;
    token.line = line;
    
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
    
    char c = currentCharacter();
    
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_' || c == '$';
}

bool Scanner::isAlpha(char c) {
        
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_' || c == '$';
}

bool Scanner::isAlphaNumeric() {
    
    char c = currentCharacter();
    
    return isAlpha(c) || (c >= '0' && c <= '9');
}

bool Scanner::isAlphaNumeric(char c) {
    return isAlpha(c) || (c >= '0' && c <= '9');
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

bool Scanner::isHexDigit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

int Scanner::hexToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return 0;
}

std::string Scanner::encodeUTF8(int codepoint) {
    std::string out;
    if (codepoint <= 0x7F) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    return out;
}
