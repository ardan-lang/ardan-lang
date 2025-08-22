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
#include "../Scanner/Token/TokenType.h"
#include "../Statements/Statements.hpp"

class Parser {
    vector<Token> tokens;
    int current = 0;

public:
    Parser(vector<Token> tokens) : tokens(tokens) {};
    vector<unique_ptr<Statement>> parse();
    vector<unique_ptr<Statement>> statements;

private:
    
    // Statements declarations
    unique_ptr<Statement> parseStatement();
    unique_ptr<Statement> parseEmptyStatement();
    unique_ptr<Statement> parseBlockStatement();
    unique_ptr<Statement> parseIfStatement();
    unique_ptr<Statement> parseWhileStatement();
    unique_ptr<Statement> parseForStatement();
    // unique_ptr<Statement> parseDoWhileStatement();
    // unique_ptr<Statement> parseSwitchStatement();
    // unique_ptr<Statement> parseTryStatement();
    // unique_ptr<Statement> parseThrowStatement();
    // unique_ptr<Statement> parseReturnStatement();
    
    // unique_ptr<Statement> parseBreakStatement();
    // unique_ptr<Statement> parseContinueStatement();
    unique_ptr<Statement> parseVariableStatement();
    unique_ptr<Statement> parseFunctionDeclaration();
    // unique_ptr<Statement> parseClassDeclaration();
    unique_ptr<Statement> parseExpressionStatement();

    // ───────────── Helpers (match, consume, check, etc.) ─────────────

    bool match(TokenType type);
    bool match(initializer_list<TokenType> types);
    Token consume(TokenType type, const string& message);

    Token consumeKeyword(const string& keyword);
    bool checkKeyword(const string& keyword);
    bool matchKeyword(const string& keyword);

    bool check(TokenType type);
    Token advance();
    Token peek();
    Token previous();
    bool isAtEnd();

    unique_ptr<Expression> parseExpression() {
        return parseComma();
    }

    // ───────────── Lowest precedence → Highest ─────────────
    
    unique_ptr<Expression> parseComma() {
        vector<unique_ptr<Expression>> exprs;
        exprs.push_back(parseAssignment());

        while (match(TokenType::COMMA)) {
            exprs.push_back(parseAssignment());
        }

        if (exprs.size() == 1) {
            return std::move(exprs[0]); // just a single expression, no need for sequence
        }

        return make_unique<SequenceExpression>(std::move(exprs));
    }

    unique_ptr<Expression> parseAssignment() {
        auto expr = parseConditional();

        if (match({
            TokenType::ASSIGN,
            TokenType::ASSIGN_ADD, TokenType::ASSIGN_MINUS,
            TokenType::ASSIGN_MUL, TokenType::ASSIGN_DIV,
            TokenType::POWER_ASSIGN,
            TokenType::MODULI_ASSIGN,
            TokenType::BITWISE_LEFT_SHIFT_ASSIGN,
            TokenType::BITWISE_RIGHT_SHIFT_ASSIGN,
            TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN,
            TokenType::BITWISE_AND_ASSIGN,
            TokenType::BITWISE_OR_ASSIGN,
            TokenType::BITWISE_XOR_ASSIGN,
            TokenType::LOGICAL_AND_ASSIGN,
            TokenType::LOGICAL_OR_ASSIGN,
            TokenType::NULLISH_COALESCING_ASSIGN
        })) {
            Token op = previous();
            auto right = parseAssignment();
            // expr = make_unique<AssignmentExpression>(op, std::move(expr), std::move(right));
            expr = make_unique<BinaryExpression>(op,
                                                 std::move(expr),
                                                 std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseConditional() {
        auto expr = parseNullish();
        if (match(TokenType::TERNARY)) {
            auto thenExpr = parseExpression();
            consume(TokenType::COLON, "Expected ':' in conditional expression");
            auto elseExpr = parseExpression();
            expr = make_unique<ConditionalExpression>(std::move(expr), std::move(thenExpr), std::move(elseExpr));
        }
        return expr;
    }

    unique_ptr<Expression> parseNullish() {
        auto expr = parseLogicalOr();
        while (match(TokenType::NULLISH_COALESCING)) {
            Token op = previous();
            auto right = parseLogicalOr();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseLogicalOr() {
        auto expr = parseLogicalAnd();
        while (match(TokenType::LOGICAL_OR)) {
            Token op = previous();
            auto right = parseLogicalAnd();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseLogicalAnd() {
        auto expr = parseBitwiseOr();
        while (match(TokenType::LOGICAL_AND)) {
            Token op = previous();
            auto right = parseBitwiseOr();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseBitwiseOr() {
        auto expr = parseBitwiseXor();
        while (match(TokenType::BITWISE_OR)) {
            Token op = previous();
            auto right = parseBitwiseXor();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseBitwiseXor() {
        auto expr = parseBitwiseAnd();
        while (match(TokenType::BITWISE_XOR)) {
            Token op = previous();
            auto right = parseBitwiseAnd();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseBitwiseAnd() {
        auto expr = parseEquality();
        while (match(TokenType::BITWISE_AND)) {
            Token op = previous();
            auto right = parseEquality();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseEquality() {
        auto expr = parseRelational();
        while (match({
            TokenType::VALUE_EQUAL, TokenType::REFERENCE_EQUAL,
            TokenType::INEQUALITY, TokenType::STRICT_INEQUALITY
        })) {
            Token op = previous();
            auto right = parseRelational();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseRelational() {
        auto expr = parseShift();
        while (match({
            TokenType::LESS_THAN, TokenType::LESS_THAN_EQUAL,
            TokenType::GREATER_THAN, TokenType::GREATER_THAN_EQUAL
        })) {
            Token op = previous();
            auto right = parseShift();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseShift() {
        auto expr = parseAdditive();
        while (match({
            TokenType::BITWISE_LEFT_SHIFT,
            TokenType::BITWISE_RIGHT_SHIFT,
            TokenType::UNSIGNED_RIGHT_SHIFT
        })) {
            Token op = previous();
            auto right = parseAdditive();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseAdditive() {
        auto expr = parseMultiplicative();
        while (match({ TokenType::ADD, TokenType::MINUS })) {
            Token op = previous();
            auto right = parseMultiplicative();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseMultiplicative() {
        auto expr = parseExponentiation();
        while (match({ TokenType::MUL, TokenType::DIV, TokenType::MODULI })) {
            Token op = previous();
            auto right = parseExponentiation();
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseExponentiation() {
        auto expr = parseUnary();
        if (match(TokenType::POWER)) {
            Token op = previous();
            auto right = parseExponentiation(); // ** is right-associative
            expr = make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    unique_ptr<Expression> parseUnary() {
        if (match({
            TokenType::LOGICAL_NOT, TokenType::BITWISE_NOT,
            TokenType::INCREMENT, TokenType::DECREMENT,
            TokenType::ADD, TokenType::MINUS
        })) {
            Token op = previous();
            auto right = parseUnary();
            return make_unique<UnaryExpression>(op, std::move(right));
        }
        return parseUpdate();
    }

    unique_ptr<Expression> parseUpdate() {
        auto expr = parseLeftHandSide();
        if (match({ TokenType::INCREMENT, TokenType::DECREMENT })) {
            Token op = previous();
            return make_unique<UpdateExpression>(op, std::move(expr), /*prefix=*/false);
        }
        return expr;
    }

    // ───────────── LHS: calls, member access, new, super, etc. ─────────────

    unique_ptr<Expression> parseLeftHandSide() {
        auto expr = parseNewMember();

        while (true) {
            if (match(TokenType::LEFT_PARENTHESIS)) {
                vector<unique_ptr<Expression>> args;
                if (!check(TokenType::RIGHT_PARENTHESIS)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::RIGHT_PARENTHESIS, "Expected ')' after arguments");
                expr = make_unique<CallExpression>(std::move(expr), std::move(args));
            } else if (match(TokenType::DOT)) {
                Token name = consume(TokenType::IDENTIFIER, "Expected property name after '.'");
                expr = make_unique<MemberExpression>(std::move(expr), name, false);
            } else if (match(TokenType::LEFT_SQUARE_BRACKET)) {
                auto property = parseExpression();
                consume(TokenType::RIGHT_SQUARE_BRACKET, "Expected ']' after computed property");
                expr = make_unique<MemberExpression>(std::move(expr), std::move(property), true);
            } else {
                break;
            }
        }

        return expr;
    }

    unique_ptr<Expression> parseNewMember() {
        if (match(TokenType::KEYWORD) && previous().lexeme == "new") {
            auto ctor = parseNewMember();
            vector<unique_ptr<Expression>> args;
            if (match(TokenType::LEFT_PARENTHESIS)) {
                if (!check(TokenType::RIGHT_PARENTHESIS)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::RIGHT_PARENTHESIS, "Expected ')' after constructor args");
            }
            return make_unique<NewExpression>(std::move(ctor), std::move(args));
        }
        return parsePrimary();
    }

    // ───────────── Primary expressions ─────────────

    unique_ptr<Expression> parsePrimary() {
        if (match(TokenType::BOOLEAN)) {
            if (previous().lexeme == "TRUE") {
                return make_unique<TrueKeyword>();
            }
            
            if (previous().lexeme == "FALSE") {
                return make_unique<FalseKeyword>();
            }
        }
        if (match(TokenType::NUMBER)) return make_unique<NumericLiteral>(previous().lexeme);
        if (match(TokenType::STRING)) return make_unique<StringLiteral>(previous().lexeme);
        if (match(TokenType::IDENTIFIER)) return make_unique<IdentifierExpression>(previous());
        if (match(TokenType::KEYWORD)) {
            auto kw = previous();
            if (kw.lexeme == "this") return make_unique<ThisExpression>();
            if (kw.lexeme == "super") return make_unique<SuperExpression>();
        }
        if (match(TokenType::LEFT_PARENTHESIS)) {
            auto expr = parseExpression();
            consume(TokenType::RIGHT_PARENTHESIS, "Expected ')'");
            return expr;
        }
        if (match(TokenType::LEFT_SQUARE_BRACKET)) {
            vector<unique_ptr<Expression>> elements;
            if (!check(TokenType::RIGHT_SQUARE_BRACKET)) {
                do {
                    elements.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_SQUARE_BRACKET, "Expected ']'");
            return make_unique<ArrayLiteralExpression>(std::move(elements));
        }
        if (match(TokenType::LEFT_BRACKET)) { // { object literal }
            vector<pair<Token, unique_ptr<Expression>>> props;
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    Token key = consume(TokenType::IDENTIFIER, "Expected property key");
                    consume(TokenType::COLON, "Expected ':' after property key");
                    auto value = parseExpression();
                    props.push_back({ key, std::move(value) });
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_BRACKET, "Expected '}'");
            return make_unique<ObjectLiteralExpression>(std::move(props));
        }
        
        cout << peek().lexeme << endl;

        throw error(peek(), "Unexpected token in primary expression");
    }
    
    [[noreturn]] runtime_error error(const Token& token, const string& message) {
        string where = token.type == TokenType::END_OF_FILE
        ? " at end"
        : " at '" + token.lexeme + "'";
        // return runtime_error("[line " + to_string(token.line) + "] Error" + where + ": " + message);
        return runtime_error(message);
    }

};

#endif /* Parser_hpp */
