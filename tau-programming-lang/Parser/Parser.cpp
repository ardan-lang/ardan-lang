//
//  Parser.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 17/08/2025.
//

#include "Parser.hpp"
#include "../Statements/Statements.hpp"

using namespace std;

vector<unique_ptr<Statement>> Parser::parse() {
    while (!isEOF()) {
        statements.push_back(declaration());
    }
    return std::move(statements);
}

unique_ptr<Statement> Parser::declaration() {

    const Token& currentType = currentToken();

    if (currentType.type == TokenType::SEMI_COLON) {
        return emptyStatement();
    }
    
    if (currentType.type == TokenType::LEFT_BRACKET) {
        return blockStatement();
    }
    
//    if (currentType.type == TokenType::VAR) {
//        return varDeclaration();
//    }
//    if (currentType.type == TokenType::PRINT) {
//        return printStatement();
//    }
//    
//    return expressionStatement();

    advance();
    return emptyStatement();;
    
}

unique_ptr<Statement> Parser::varDeclaration() {
    return emptyStatement();;

}

unique_ptr<Statement> Parser::printStatement() {
    return emptyStatement();;

}

unique_ptr<Statement> Parser::expressionStatement() {
    return emptyStatement();;
}

unique_ptr<Statement> Parser::emptyStatement() {
    advance(); // consume ';'
    return make_unique<EmptyStatement>();
}

unique_ptr<Statement> Parser::blockStatement() {
    advance(); // consume '{'
    auto block = make_unique<BlockStatement>();
    
    while (currentToken().type != TokenType::RIGHT_BRACKET && !isEOF()) {
        block->statements.push_back(declaration());
    }
    
    consume(TokenType::RIGHT_BRACKET, "Expect '}' after block.");
    return block;
}

void Parser::advance() {
    
    if(isEOF()) {
        return;
    }
    
    index++;
}

void Parser::consume(TokenType type, const std::string& message) {
    
    if (currentToken().type == type) {
        advance();
        return;
    }
    
    throw std::runtime_error(message);
    
}

const Token& Parser::currentToken() {
    return tokens[index];
}

bool Parser::isEOF() {

    if (currentToken().type == TokenType::END_OF_FILE) {
        return true;
    }
    
    return false;
    
}
