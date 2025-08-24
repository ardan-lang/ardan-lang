//
//  Parser.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 17/08/2025.
//

#include "Parser.hpp"
#include "../overloads/operators.h"
#include "../Statements/Statements.hpp"
#include "../Visitor/AstPrinter/AstPrinter.h"
#include <stdexcept>

using namespace std;

vector<unique_ptr<Statement>> Parser::parse() {
    
    while (isAtEnd() == false) {
        statements.push_back(parseStatement());
    }
    
    return std::move(statements);
    
}

unique_ptr<Statement> Parser::parseStatement() {
    Token token = peek();

    switch (token.type) {
        case TokenType::SEMI_COLON:       return parseEmptyStatement();
        case TokenType::LEFT_BRACKET:     return parseBlockStatement();
        case TokenType::KEYWORD: {
            if (peek().lexeme == ("IF"))       return parseIfStatement();
            if (peek().lexeme == ("WHILE"))    return parseWhileStatement();
            if (peek().lexeme == ("FOR"))      return parseForStatement();
            if (peek().lexeme == ("DO"))       return parseDoWhileStatement();
            if (peek().lexeme == ("SWITCH"))   return parseSwitchStatement();
            if (peek().lexeme == ("TRY"))      return parseTryStatement();
            if (peek().lexeme == ("THROW"))    return parseThrowStatement();
            if (peek().lexeme == ("RETURN"))   return parseReturnStatement();
            if (peek().lexeme == ("BREAK"))    return parseBreakStatement();
            if (peek().lexeme == ("CONTINUE")) return parseContinueStatement();
            if (peek().lexeme == ("VAR") ||
                peek().lexeme == ("LET") ||
                peek().lexeme == ("CONST"))    return parseVariableStatement();
            if (peek().lexeme == ("FUNCTION")) return parseFunctionDeclaration();
            else return parseExpressionStatement();
        }
        case TokenType::CLASS:
            return parseClassDeclaration();
        default:
            return parseExpressionStatement();
    }
}

unique_ptr<Statement> Parser::parseEmptyStatement() {
    consume(TokenType::SEMI_COLON, "Expected ';'");
    return make_unique<EmptyStatement>();
}

// ---------------------
// Block Statement
// ---------------------
unique_ptr<Statement> Parser::parseBlockStatement() {
    consume(TokenType::LEFT_BRACKET, "Expected '{'");
    vector<unique_ptr<Statement>> body;
    while (!check(TokenType::RIGHT_BRACKET) && !isAtEnd()) {
        body.push_back(parseStatement());
    }
    consume(TokenType::RIGHT_BRACKET, "Expected '}'");
    return make_unique<BlockStatement>(std::move(body));
}

unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMI_COLON, "Expected ';' after expression");
    return make_unique<ExpressionStatement>(std::move(expr));
}

// ---------------------
// If Statement
// ---------------------
unique_ptr<Statement> Parser::parseIfStatement() {
    consumeKeyword("IF");
    consume(TokenType::LEFT_PARENTHESIS, "Expected '(' after 'if'");
    auto test = parseExpression();
    consume(TokenType::RIGHT_PARENTHESIS, "Expected ')'");
    auto consequent = parseStatement();
    unique_ptr<Statement> alternate = nullptr;
    if (matchKeyword("ELSE")) {
        alternate = parseStatement();
    }
    return make_unique<IfStatement>(std::move(test), std::move(consequent), std::move(alternate));
}

// ---------------------
// While Statement
// ---------------------
unique_ptr<Statement> Parser::parseWhileStatement() {
    consumeKeyword("WHILE");
    consume(TokenType::LEFT_PARENTHESIS, "Expected '(' after 'while'");
    auto test = parseExpression();
    consume(TokenType::RIGHT_PARENTHESIS, "Expected ')'");
    auto body = parseStatement();
    return make_unique<WhileStatement>(std::move(test), std::move(body));
}

// ---------------------
// For Statement
// ---------------------
unique_ptr<Statement> Parser::parseForStatement() {
    consumeKeyword("FOR");
    consume(TokenType::LEFT_PARENTHESIS, "Expected '(' after 'for'");

    unique_ptr<Statement> init = nullptr;
    if (!check(TokenType::SEMI_COLON)) {
        if (checkKeyword("VAR") || checkKeyword("LET") || checkKeyword("CONST")) {
            init = parseVariableStatement();
        } else {
            init = parseExpressionStatement();
        }
    } else {
        consume(TokenType::SEMI_COLON, "Expected ';'");
    }

    unique_ptr<Expression> test = nullptr;
    if (!check(TokenType::SEMI_COLON)) {
        test = parseExpression();
    }
    consume(TokenType::SEMI_COLON, "Expected ';'");

    unique_ptr<Expression> update = nullptr;
    if (!check(TokenType::RIGHT_PARENTHESIS)) {
        update = parseExpression();
    }
    consume(TokenType::RIGHT_PARENTHESIS, "Expected ')'");

    auto body = parseStatement();

    return make_unique<ForStatement>(std::move(init), std::move(test), std::move(update), std::move(body));
}

// ---------------------
// Variable Statement
// ---------------------
unique_ptr<Statement> Parser::parseVariableStatement() {
    Token keyword = advance(); // var | let | const
    vector<VariableDeclarator> declarations;

    do {
        auto id = consume(TokenType::IDENTIFIER, "Expected variable name");
        unique_ptr<Expression> init = nullptr;
        if (match(TokenType::ASSIGN)) {
            init = parseAssignment();
        }
        declarations.push_back(VariableDeclarator{id.lexeme, std::move(init)});
    } while (match(TokenType::COMMA));

    consume(TokenType::SEMI_COLON, "Expected ';' after variable declaration");
    return make_unique<VariableStatement>(keyword.lexeme, std::move(declarations));
}

// ---------------------
// Function Declaration
// ---------------------
unique_ptr<Statement> Parser::parseFunctionDeclaration() {
    consumeKeyword("FUNCTION");
    auto id = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LEFT_PARENTHESIS, "Expected '('");
    vector<unique_ptr<Expression>> params;
    if (!check(TokenType::RIGHT_PARENTHESIS)) {
        do {
            params.push_back(parseAssignment());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PARENTHESIS, "Expected ')'");
    unique_ptr<Statement> body = parseBlockStatement();
    return make_unique<FunctionDeclaration>(id.lexeme, std::move(params), std::move(body));
}

// ---------------------
// Class Declaration
// ---------------------
unique_ptr<Statement> Parser::parseClassDeclaration() {
    consume(TokenType::CLASS, "Expect 'class'.");

    string id = "";
    if (check(TokenType::IDENTIFIER)) {
        id = advance().lexeme;
    }

    unique_ptr<Expression> superClass = nullptr;
    if (match(TokenType::EXTENDS)) {
        superClass = parseExpression();
    }

    consume(TokenType::LEFT_BRACKET, "Expect '{' before class body.");
    vector<unique_ptr<MethodDefinition>> methods;
    vector<unique_ptr<PropertyDeclaration>> fields;

    while (!check(TokenType::RIGHT_BRACKET) && !isAtEnd()) {
        parseClassMember(methods, fields);
    }

    consume(TokenType::RIGHT_BRACKET, "Expect '}' after class body.");

    return make_unique<ClassDeclaration>(
        id,
        std::move(superClass),
        std::move(methods),
        std::move(fields)
    );
}

vector<unique_ptr<Expression>> Parser::parseClassModifiers() {
    vector<unique_ptr<Expression>> modifiers;
    while (check(TokenType::KEYWORD)) {
        string kw = peek().lexeme;
        if (kw == "PRIVATE" || kw == "PUBLIC" || kw == "PROTECTED" || kw == "STATIC") {
            modifiers.push_back(parseExpression());
        } else {
            break;
        }
    }
    return modifiers;
}

void Parser::parseClassMember(
    vector<unique_ptr<MethodDefinition>>& methods,
    vector<unique_ptr<PropertyDeclaration>>& fields
) {
    vector<unique_ptr<Expression>> modifiers = parseClassModifiers();

    if (checkKeyword("VAR") || checkKeyword("CONST") || checkKeyword("LET")) {
        // Class field
        unique_ptr<Statement> property = parseVariableStatement();
        fields.push_back(make_unique<PropertyDeclaration>(
            std::move(modifiers),
            std::move(property)
        ));
    } else {
        // Class method
        const string name = consume(TokenType::IDENTIFIER, "Expect method name.").lexeme;
        vector<unique_ptr<Expression>> params = parseParameterList();
        unique_ptr<Statement> body = parseBlockStatement();

        methods.push_back(make_unique<MethodDefinition>(
            name,
            std::move(params),
            std::move(body),
            std::move(modifiers)
        ));
    }
}

vector<unique_ptr<Expression>> Parser::parseParameterList() {
    vector<unique_ptr<Expression>> params;

    consume(TokenType::LEFT_PARENTHESIS, "Expect '(' before parameter list.");
    if (!check(TokenType::RIGHT_PARENTHESIS)) {
        do {
            params.push_back(parseAssignment());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PARENTHESIS, "Expect ')' after parameters.");

    return params;
}

// ---------------------
// Continue Statement
// ---------------------
unique_ptr<Statement> Parser::parseContinueStatement() {
    consumeKeyword("CONTINUE");
    string label = "";
    if (check(TokenType::IDENTIFIER)) {
        label = advance().lexeme;
    }
    consume(TokenType::SEMI_COLON, "Expect ';' after continue.");
    return make_unique<ContinueStatement>(label);
}

// ---------------------
// Do-While Statement
// ---------------------
unique_ptr<Statement> Parser::parseDoWhileStatement() {
    consumeKeyword("DO");
    auto body = parseStatement();
    consumeKeyword("WHILE", "Expect 'while' after do-while body.");
    consume(TokenType::LEFT_PARENTHESIS, "Expect '(' after while.");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PARENTHESIS, "Expect ')' after condition.");
    consume(TokenType::SEMI_COLON, "Expect ';' after do-while.");
    return make_unique<DoWhileStatement>(std::move(body), std::move(condition));
}

// ---------------------
// Switch Statement
// ---------------------
unique_ptr<Statement> Parser::parseSwitchStatement() {
    consumeKeyword("SWITCH", "Expect 'switch'.");
    consume(TokenType::LEFT_PARENTHESIS, "Expect '(' after 'switch'.");
    auto discriminant = parseExpression();
    consume(TokenType::RIGHT_PARENTHESIS,
            "Expect ')' after switch discriminant.");
    consume(TokenType::LEFT_BRACKET, "Expect '{' before switch body.");

    vector<unique_ptr<SwitchCase>> cases;
    while (!check(TokenType::RIGHT_BRACKET) && !isAtEnd()) {
        if (matchKeyword("CASE")) {
            auto test = parseExpression();
            consume(TokenType::COLON, "Expect ':' after case.");
            vector<unique_ptr<Statement>> consequent;
            while (!checkKeyword("CASE") && !checkKeyword("DEFAULT") && !check(TokenType::RIGHT_BRACKET)) {
                consequent.push_back(parseStatement());
            }
            cases.push_back(make_unique<SwitchCase>(std::move(test), std::move(consequent)));
        } else if (matchKeyword("DEFAULT")) {
            consume(TokenType::COLON, "Expect ':' after default.");
            vector<unique_ptr<Statement>> consequent;
            while (!checkKeyword("CASE") && !check(TokenType::RIGHT_BRACKET)) {
                consequent.push_back(parseStatement());
            }
            cases.push_back(make_unique<SwitchCase>(nullptr, std::move(consequent)));
        } else {
            throw error(peek(), "Unexpected token in switch.");
        }
    }

    consume(TokenType::RIGHT_BRACKET, "Expect '}' after switch cases.");
    return make_unique<SwitchStatement>(std::move(discriminant), std::move(cases));
}

// ---------------------
// Return Statement
// ---------------------
unique_ptr<Statement> Parser::parseReturnStatement() {
    consumeKeyword("RETURN");
    unique_ptr<Expression> value = nullptr;
    if (!check(TokenType::SEMI_COLON)) {
        value = parseExpression();
    }
    consume(TokenType::SEMI_COLON, "Expect ';' after return.");
    return make_unique<ReturnStatement>(std::move(value));
}

// ---------------------
// Throw Statement
// ---------------------
unique_ptr<Statement> Parser::parseThrowStatement() {
    consumeKeyword("THROW", "Expect 'throw'.");
    auto expr = parseExpression();
    consume(TokenType::SEMI_COLON, "Expect ';' after throw.");
    return make_unique<ThrowStatement>(std::move(expr));
}

// ---------------------
// Break Statement
// ---------------------
unique_ptr<Statement> Parser::parseBreakStatement() {
    consumeKeyword("BREAK");
    string label = "";
    if (check(TokenType::IDENTIFIER)) {
        label = advance().lexeme;
    }
    consume(TokenType::SEMI_COLON, "Expect ';' after break.");
    return make_unique<BreakStatement>(label);
}

// ---------------------
// Try Statement
// ---------------------
unique_ptr<Statement> Parser::parseTryStatement() {
    consumeKeyword("TRY", "Expect 'try'.");
    auto block = parseBlockStatement();

    unique_ptr<CatchClause> handler = nullptr;
    if (matchKeyword("CATCH")) {
        consume(TokenType::LEFT_PARENTHESIS, "Expect '(' after catch.");
        string param = consume(TokenType::IDENTIFIER, "Expect identifier in catch.").lexeme;
        consume(TokenType::RIGHT_PARENTHESIS, "Expect ')' after catch param.");
        auto body = parseBlockStatement();
        handler = make_unique<CatchClause>(param, std::move(body));
    }

    unique_ptr<Statement> finalizer = nullptr;
    if (matchKeyword("FINALLY")) {
        finalizer = parseBlockStatement();
    }

    return make_unique<TryStatement>(std::move(block), std::move(handler), std::move(finalizer));
}

// ───────────── Helpers ─────────────

// Try to match a single token type
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

// Try to match one of several token types
bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

// Ensure the next token matches `type` or throw
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error("Parse error: expected " + message);
}

// Check if current token is of given type without consuming
bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

// Advance one token and return the previous one
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

// Look at current token
Token Parser::peek() {
    return tokens[current];
}

// Look at most recently consumed token
Token Parser::previous() {
    return tokens[current - 1];
}

// EOF check
bool Parser::isAtEnd() {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::checkKeyword(const string& keyword) {
    if (isAtEnd()) return false;
    return peek().lexeme == keyword;
}

bool Parser::matchKeyword(const string& keyword) {
    if (checkKeyword(keyword)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consumeKeyword(const string& keyword) {
    if (checkKeyword(keyword)) return advance();
    throw std::runtime_error("Parse error: expected keyword '" + keyword + "'");
}

Token Parser::consumeKeyword(const string& keyword, const string& message) {
    if (checkKeyword(keyword)) return advance();
    throw std::runtime_error(message);
}

