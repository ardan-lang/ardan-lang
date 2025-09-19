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
#include "../overloads/operators.h"

class Parser {
    vector<Token> tokens;
    int current = 0;

public:
    Parser(vector<Token> tokens) : tokens(tokens) {};
    vector<unique_ptr<Statement>> parse();
    vector<unique_ptr<Statement>> statements;
    string sourceFile;

private:
    
    // Statements declarations
    unique_ptr<Statement> parseStatement();
    unique_ptr<Statement> parseEmptyStatement();
    unique_ptr<Statement> parseBlockStatement();
    unique_ptr<Statement> parseIfStatement();
    unique_ptr<Statement> parseWhileStatement();
    
    unique_ptr<Statement> parseForStatement();
    unique_ptr<Statement> parseTraditionalForStatement(unique_ptr<Statement>& init);
    unique_ptr<Statement> parseForInStatement(unique_ptr<Statement>& init);
    unique_ptr<Statement> parseForOfStatement(unique_ptr<Statement>& init);
    unique_ptr<Statement> parseForVariableStatement();

    unique_ptr<Statement> parseDoWhileStatement();
    unique_ptr<Statement> parseSwitchStatement();
    unique_ptr<Statement> parseTryStatement();
    unique_ptr<Statement> parseThrowStatement();
    unique_ptr<Statement> parseReturnStatement();
    
    unique_ptr<Statement> parseBreakStatement();
    unique_ptr<Statement> parseContinueStatement();
    unique_ptr<Statement> parseVariableStatement();
    unique_ptr<Statement> parseFunctionDeclaration();
    unique_ptr<Statement> parseClassDeclaration();
    unique_ptr<Statement> parseImportDeclaration();

    vector<unique_ptr<Expression>> parseClassModifiers();
    void parseClassMember(vector<unique_ptr<MethodDefinition>>& methods, vector<unique_ptr<PropertyDeclaration>>& fields);

    vector<unique_ptr<Expression>> parseParameterList();
    unique_ptr<Statement> parseExpressionStatement();
    unique_ptr<Statement> parseAsyncStatement();
    
    // ───────────── Helpers ─────────────

    bool match(TokenType type);
    bool match(initializer_list<TokenType> types);
    Token consume(TokenType type, const string& message);

    Token consumeKeyword(const string& keyword);
    Token consumeKeyword(const string& keyword, const string& message);
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

    // ───────────── LHS: calls, member access, new, super ─────────────

    unique_ptr<Expression> parseLeftHandSide() {
        auto expr = parseAwaitExpression();

        while (true) {
            if (match(TokenType::LEFT_PARENTHESIS)) {
                vector<unique_ptr<Expression>> args;
                if (!check(TokenType::RIGHT_PARENTHESIS)) {
                    do {
                        args.push_back(parseAssignment());
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
    
    unique_ptr<Expression> parseAwaitExpression() {
        
        if (matchKeyword("AWAIT")) {
            return make_unique<AwaitExpression>(parseAsyncExpression());
        }
        
        return parseAsyncExpression();
        
    }
    
    unique_ptr<Expression> parseAsyncExpression() {
        
        if (matchKeyword("ASYNC")) {
            
            auto expr = parseFunctionExpression();
            
            if (ArrowFunction* arrow_expr = dynamic_cast<ArrowFunction*>(expr.get())) {
                arrow_expr->is_async = true;
            }
            
            if (auto function_expr = dynamic_cast<FunctionExpression*>(expr.get())) {
                function_expr->is_async = true;
            }
            
            return expr;
            
        }
        
        return parseFunctionExpression();

    }
    
    unique_ptr<Expression> parseFunctionExpression() {
        
        if (matchKeyword("FUNCTION")) {
            
            bool is_func_stmt = false;
            
            Token token = previous();
            Token current_token = peek();
            
            if (current_token.type == TokenType::IDENTIFIER) {
                is_func_stmt = true;
                consume(TokenType::IDENTIFIER, "Expected identifier after function.");
            }
            
            bool seenRest = false;
            
            consume(TokenType::LEFT_PARENTHESIS, "Expected '('");
            vector<unique_ptr<Expression>> params;
            if (!check(TokenType::RIGHT_PARENTHESIS)) {
                do {
                    if (match(TokenType::SPREAD)) {
                        // Only allow one rest parameter, and it must be last
                        if (seenRest) error(peek(), "Only one rest parameter allowed");
                        seenRest = true;
                        auto restArg = consume(TokenType::IDENTIFIER, "Expected rest parameter name");
                        // Store as a special "rest" parameter
                        params.push_back(make_unique<RestParameter>(restArg));
                        break; // Rest parameter must be last in the list
                    } else {
                        params.push_back(parseAssignment());
                    }
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_PARENTHESIS, "Expected ')'");
            unique_ptr<Statement> body = parseBlockStatement();
            
            return make_unique<FunctionExpression>(is_func_stmt ? current_token : token,
                                                   std::move(params),
                                                   std::move(body));
        }
        
        return parseClassExpression();
    }
    
    unique_ptr<Expression> parseClassExpression() {
        
        if (matchKeyword("CLASS")) {
            Token token = previous();

            unique_ptr<Expression> superClass = nullptr;
            if (matchKeyword("EXTENDS")) {
                superClass = parseExpression();
            }

            consume(TokenType::LEFT_BRACKET, "Expect '{' before class body.");
            vector<unique_ptr<MethodDefinition>> methods;
            vector<unique_ptr<PropertyDeclaration>> fields;

            while (!check(TokenType::RIGHT_BRACKET) && !isAtEnd()) {
                parseClassMember(methods, fields);
            }

            consume(TokenType::RIGHT_BRACKET, "Expect '}' after class body.");

            return make_unique<ClassExpression>(
                                                 token,
                std::move(superClass),
                std::move(methods),
                std::move(fields)
            );
        }
        
        return parseNewMember();
        
    }

    unique_ptr<Expression> parseNewMember() {
        if (matchKeyword("NEW")) {
            Token token = previous();
            auto ctor = parseNewMember();
            vector<unique_ptr<Expression>> args;
            if (match(TokenType::LEFT_PARENTHESIS)) {
                if (!check(TokenType::RIGHT_PARENTHESIS)) {
                    do {
                                                
                        args.push_back(parseAssignment());
                        
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::RIGHT_PARENTHESIS,
                        "Expected ')' after constructor args at line: " + to_string(peek().line));
            }
            return make_unique<NewExpression>(token, std::move(ctor), std::move(args));
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
        if (match(TokenType::IDENTIFIER))  {
            
            Token token_previous = previous();
            
            if (token_previous.lexeme == "undefined") return make_unique<NullKeyword>(token_previous);

            // add support for single param arrow function
            // x => x; x => {};
            if (match(TokenType::ARROW)) {
                
                if (check(TokenType::LEFT_BRACKET)) {
                    auto block = parseBlockStatement();
                    // x => {}
                    return make_unique<ArrowFunction>(make_unique<IdentifierExpression>(std::move(token_previous)), std::move(block));
                }

                auto expr = parseAssignment();
                // x => x
                return make_unique<ArrowFunction>(make_unique<IdentifierExpression>(std::move(token_previous)), std::move(expr));

            }

            return make_unique<IdentifierExpression>(std::move(token_previous));
            
        }
        if (match(TokenType::KEYWORD)) {
            auto kw = previous();
            if (kw.lexeme == "THIS") return make_unique<ThisExpression>();
            if (kw.lexeme == "SUPER") return make_unique<SuperExpression>();
            if (kw.lexeme == "PRIVATE") return make_unique<PrivateKeyword>();
            if (kw.lexeme == "PUBLIC") return make_unique<PublicKeyword>();
            if (kw.lexeme == "PROTECTED") return make_unique<ProtectedKeyword>();
            if (kw.lexeme == "STATIC") return make_unique<StaticKeyword>();
            if (kw.lexeme == "NULL") return make_unique<NullKeyword>(kw);
        }
        if (match(TokenType::LEFT_PARENTHESIS)) {
            
            unique_ptr<Expression> expr = nullptr;
            
            if (peek().type == TokenType::RIGHT_PARENTHESIS) {
                consume(TokenType::RIGHT_PARENTHESIS,
                        "Expected ')' at line: " + to_string(peek().line));
            } else {
                expr = parseExpression();
                consume(TokenType::RIGHT_PARENTHESIS,
                        "Expected ')' at line: " + to_string(peek().line));
            }
            
            // add support for arrow function
            if (match(TokenType::ARROW)) {
                
                // (x) => {}
                if (check(TokenType::LEFT_BRACKET)) {

                    // statement body
                    auto block = parseBlockStatement();

                    return make_unique<ArrowFunction>(std::move(expr), std::move(block));
                    
                }
                
                auto exprBody = parseAssignment();
                
                // (x) => x
                return make_unique<ArrowFunction>(std::move(expr), std::move(exprBody));
                
            }
            
            return expr;
            
        }
        if (match(TokenType::LEFT_SQUARE_BRACKET)) {
            Token token = previous();
            vector<unique_ptr<Expression>> elements;
            if (!check(TokenType::RIGHT_SQUARE_BRACKET)) {
                do {
                    elements.push_back(parseAssignment());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_SQUARE_BRACKET,
                    "Expected ']' at line: " + to_string(peek().line));
            return make_unique<ArrayLiteralExpression>(token, std::move(elements));
        }
        if (match(TokenType::LEFT_BRACKET)) { // { object literal }
            Token token = previous();
            vector<pair<Token, unique_ptr<Expression>>> props;
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    
                    Token key;
                    
                    // key in object-literal can be a string
                    if (peek().type == TokenType::STRING) {
                        key = consume(TokenType::STRING,
                                      "Expected property key at line: " + to_string(peek().line));
                    } else if (peek().type == TokenType::IDENTIFIER) {
                        key = consume(TokenType::IDENTIFIER,
                                      "Expected property key at line: " + to_string(peek().line));
                    } else {
                        throw runtime_error("Expected property key at line: " + to_string(peek().line));
                    }
                    
                    consume(TokenType::COLON,
                            "Expected ':' after property key at line: " + to_string(peek().line));
                    auto value = parseAssignment();
                    props.push_back({ key, std::move(value) });
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_BRACKET,
                    "Expected '}' at line: " + to_string(peek().line));
            return make_unique<ObjectLiteralExpression>(token, std::move(props));
        }
        
        if (match(TokenType::TEMPLATE_START)) {
            return parseTemplateLiteral();
        }
        
        if (match(TokenType::SPREAD)) {
            Token name = peek();
            advance();
            return make_unique<RestParameter>(name);
        }

        throw error(peek(), "Unexpected token in primary expression");
        
    }
    
    unique_ptr<TemplateLiteral> parseTemplateLiteral() {
        // consume(TokenType::TEMPLATE_START, "Expected template start");

        vector<unique_ptr<StringLiteral>> quasis;
        vector<unique_ptr<Expression>> expressions;

        // Always expect at least one quasi
        string currentChunk = "";

        while (!check(TokenType::TEMPLATE_END) && !isAtEnd()) {
            if (match(TokenType::TEMPLATE_CHUNK)) {
                currentChunk += previous().lexeme;
            }
            else if (match(TokenType::INTERPOLATION_START)) {
                // Push current quasi
                quasis.push_back(make_unique<StringLiteral>(currentChunk));
                currentChunk = "";

                // Parse inside ${ ... }
                auto expr = parseExpression();
                expressions.push_back(std::move(expr));

                consume(TokenType::INTERPOLATION_END, "Unterminated interpolation");

                // Ensure even if empty, a new quasi exists after interpolation
                // quasis.push_back(make_unique<StringLiteral>(""));
                // Ensure a quasi slot exists after interpolation
                if (check(TokenType::INTERPOLATION_START) || check(TokenType::TEMPLATE_END)) {
                    // consecutive ${...} or immediate end → must insert ""
                    quasis.push_back(make_unique<StringLiteral>(""));
                } else {
                    // let the next chunk fill naturally
                    currentChunk = "";
                }

            }
            else {
                advance(); // recover
            }
        }

        consume(TokenType::TEMPLATE_END, "Unterminated template literal");

        // If no quasis were pushed, still push the final chunk
        if (quasis.empty()) {
            quasis.push_back(make_unique<StringLiteral>(currentChunk));
        } else {
            // Last quasi may already be pushed (back-to-back interpolations)
            if (!currentChunk.empty()) {
                // replace the last "" if it's empty
                if (quasis.back()->text == "") {
                    quasis.back() = make_unique<StringLiteral>(currentChunk);
                } else {
                    quasis.push_back(make_unique<StringLiteral>(currentChunk));
                }
            }
        }

        Token ccp = peek();

        return make_unique<TemplateLiteral>(std::move(quasis), std::move(expressions));
    }
    
//    if (match(TOKEN_IDENTIFIER) && check(TEMPLATE_START)) {
//            auto tag = new Identifier(previous().lexeme);
//            auto literal = parseTemplateLiteral();
//            return new TaggedTemplateExpression(tag, literal);
//        }
    
    [[noreturn]] runtime_error error(const Token& token, const string& message) {
        string where = token.type == TokenType::END_OF_FILE
        ? " at end"
        : " at '" + token.lexeme + "'";
        // return runtime_error("[line " + to_string(token.line) + "] Error" + where + ": " + message);
        return runtime_error(message);
    }

};

#endif /* Parser_hpp */

//TaggedTemplateExpression {
//    tag: Identifier("tag"),
//    quasi: TemplateLiteral {
//        quasis: ["hello ", ""],
//        expressions: [ Identifier("world") ]
//    }
//}
