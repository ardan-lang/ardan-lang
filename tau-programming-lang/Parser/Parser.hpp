//
//  Parser.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 17/08/2025.
//

#ifndef Parser_hpp
#define Parser_hpp

#include <stdio.h>
#include <iostream>
#include "../Scanner/Token/Token.hpp"
#include "../Statements/Statements.hpp"

class Parser {

public:
    Parser(vector<Token>& tokens): tokens(tokens) {};
    vector<unique_ptr<Statement>> parse();
    const Token& currentToken();
    
private:
    const vector<Token>& tokens;
    vector<unique_ptr<Statement>> statements;
    int index = 0;
    bool isEOF();
    
    unique_ptr<Statement> declaration();
    unique_ptr<Statement> blockStatement();
    unique_ptr<Statement> emptyStatement();
    unique_ptr<Statement> varDeclaration();
    unique_ptr<Statement> printStatement();
    unique_ptr<Statement> expressionStatement();
    
    void advance();
    void consume(TokenType type, const std::string& message);
    
};

#endif /* Parser_hpp */
