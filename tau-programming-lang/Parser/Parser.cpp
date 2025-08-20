//
//  Parser.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 17/08/2025.
//

#include "Parser.hpp"
#include "../Statements/Statements.hpp"
#include "../Visitor/AstPrinter/AstPrinter.h"

using namespace std;

vector<unique_ptr<Statement>> Parser::parse() {
    
    while (isEOF() == false) {
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
    
    if (currentType.type == TokenType::KEYWORD && currentType.value == "VAR") {
        return varDeclaration();
    }

    if (currentType.type == TokenType::KEYWORD && currentType.value == "LET") {
        return letDeclaration();
    }

    if (currentType.type == TokenType::KEYWORD && currentType.value == "CONST") {
        return constDeclaration();
    }

    if (currentType.type == TokenType::KEYWORD && currentType.value == "function") {
        return functionDeclaration();
    }

//    if (currentType.type == TokenType::PRINT) {
//        return printStatement();
//    }
    
    return expressionStatement();

//    advance();
//    return emptyStatement();;
    
}

unique_ptr<Expression> Parser::parseExpression() {
    return parseEquality();
}

unique_ptr<Expression> Parser::parseEquality() {

    auto expr = parseComparison();
    
    // ==, !=, ===, !==
    if (currentToken().type == TokenType::VALUE_EQUAL || currentToken().type == TokenType::INEQUALITY || currentToken().type == TokenType::REFERENCE_EQUAL || currentToken().type == TokenType::STRICT_INEQUALITY) {
        
        auto tokenType = currentToken();
        advance();
        
        auto rightExpr = parseComparison();
        
        expr = make_unique<BinaryExpression>(
                    tokenType.value,
                    std::move(expr),
                    std::move(rightExpr)
                );
        
    }
    
    return expr;
    
}

//">", ">=", "<", "<="
unique_ptr<Expression> Parser::parseComparison() {
    
    auto expr = parseTerm();
    
    Token token = currentToken();
    
    if (token.type == TokenType::GREATER_THAN || token.type == TokenType::GREATER_THAN_EQUAL || token.type == TokenType::LESS_THAN || token.type == TokenType::LESS_THAN_EQUAL) {
        
        advance();
        
        auto rightExpr = parseTerm();
        
        expr = make_unique<BinaryExpression>(token.value, std::move(expr), std::move(rightExpr));
        
    }
    
    return expr;
}

// +, -
unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();
    
    Token token = currentToken();
    
    if (token.type == TokenType::ADD || token.type == TokenType::MINUS) {
        
        advance();
        
        auto rightExpr = parseFactor();
        
        expr = make_unique<BinaryExpression>(token.value, std::move(expr), std::move(rightExpr));
        
    }

    return expr;
}

// "*", "/", "%"
unique_ptr<Expression> Parser::parseFactor() {
    
    auto expr = parsePower();
    
    Token token = currentToken();

    if (token.type == TokenType::MUL || token.type == TokenType::DIV  || token.type == TokenType::MODULI) {
        
        advance();
        
        auto rightExpr = parsePower();

        expr = make_unique<BinaryExpression>(token.value,
                                             std::move(expr),
                                             std::move(rightExpr));
        
    }

    return expr;
}

// "**"
unique_ptr<Expression> Parser::parsePower() {
    
    auto expr = parseUnary();
    
    Token token = currentToken();
    
    if (token.type == TokenType::POWER) {
        
        advance();
        
        auto rightExpr = parseUnary();
        
        expr = make_unique<BinaryExpression>(token.value, std::move(expr), std::move(rightExpr));
        
    }

    return expr;
    
}

// "!", "-"
unique_ptr<Expression> Parser::parseUnary() {
    
    auto expr = parsePrimary();
    
    Token token = currentToken();
    
    if (token.type == TokenType::LOGICAL_NOT || token.type == TokenType::MINUS) {
        
        advance();
        
        auto rightExpr = parsePrimary();
        
        expr = make_unique<UnaryExpression>(token.value, std::move(rightExpr));
        
    }

    return expr;
}

unique_ptr<Expression> Parser::parsePrimary() {
    
    Token token = currentToken();
    unique_ptr<Expression> expr;
    
    if (token.type == TokenType::NUMBER) {
        expr = make_unique<LiteralExpression>(token.value);
        advance();
    }
    
    if (token.type == TokenType::STRING) {
        expr = make_unique<LiteralExpression>(token.value);
        advance();
    }
    
    if (token.type == TokenType::KEYWORD && (token.value == "true" || token.value == "false")) {
        expr = make_unique<LiteralExpression>(token.value);
        advance();
    }
    
    if (token.type == TokenType::IDENTIFIER) {
        expr = make_unique<LiteralExpression>(token.value);
        advance();
    }
    
    if (token.type == TokenType::LEFT_PARENTHESIS) {
        auto inExpr = parseExpression();
        consume(TokenType::RIGHT_PARENTHESIS, "Expect ')' after expression.");
        expr = make_unique<GroupingExpression>(std::move(inExpr));
    }
    
    return expr;
        
}

unique_ptr<Statement> Parser::functionDeclaration() {
    return emptyStatement();
}

unique_ptr<Statement> Parser::letDeclaration() {
    
    advance(); // consume 'let'
    
    const string identifier = currentToken().value;

    consume(TokenType::IDENTIFIER, "Expect let's identifier name after the let keyword");
    
    consume(TokenType::ASSIGN, "Expect '=' after let's identifier name");

    unique_ptr<Expression> expression = parseExpression();
    
    if (currentToken().type == TokenType::EOL) {
        advance();
    }

    return make_unique<VarStatement>(identifier, std::move(expression));
    
}

unique_ptr<Statement> Parser::constDeclaration() {
    
    advance(); // consume 'const'
    
    const string identifier = currentToken().value;

    consume(TokenType::IDENTIFIER, "Expect const's identifier name after the const keyword");
    
    consume(TokenType::ASSIGN, "Expect '=' after const's identifier name");

    unique_ptr<Expression> expression = parseExpression();
    
    if (currentToken().type == TokenType::EOL) {
        advance();
    }

    return make_unique<VarStatement>(identifier, std::move(expression));
    
}

unique_ptr<Statement> Parser::varDeclaration() {

    advance(); // consume 'var'

    const string identifier = currentToken().value;

    consume(TokenType::IDENTIFIER, "Expect var's identifier name afteer the var keyword");
    
    consume(TokenType::ASSIGN, "Expect '=' after var's identifier name");

    unique_ptr<Expression> expression = parseExpression();
        
    if (currentToken().type == TokenType::EOL) {
        advance();
    }
    
    return make_unique<VarStatement>(identifier, std::move(expression));

}

unique_ptr<Statement> Parser::printStatement() {
    return emptyStatement();;

}

unique_ptr<Statement> Parser::expressionStatement() {
    
    auto expr = parseExpression();
    
    Token token = currentToken();
    
    switch (token.type) {
            // ++, --, +=, -+, etc
        case TokenType::ASSIGN:
            
            break;
            
            // property access
        case TokenType::DOT:
            break;
            
            // call();
        case TokenType::LEFT_PARENTHESIS:
            break;
            
            // ++
        case TokenType::INCREMENT:
            break;
            
            // --
        case TokenType::DECREMENT:
            break;
            
        default:
            break;
    }
    
    return make_unique<ExpressionStatement>(std::move(expr));
    
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
