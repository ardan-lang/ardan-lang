//
//  operators.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 17/08/2025.
//

#ifndef operators_h
#define operators_h
#include <iostream>
#include <string>

using namespace std;

ostream& operator<<(ostream& os, TokenType type) {
    switch (type) {
        case TokenType::COMMA: return os << "COMMA";
        case TokenType::SPREAD: return os << "SPREAD";
        case TokenType::DOT: return os << "DOT";
        case TokenType::SEMI_COLON: return os << "SEMI_COLON";
        case TokenType::COLON: return os << "COLON";
        case TokenType::LEFT_PARENTHESIS: return os << "LEFT_PARENTHESIS";
        case TokenType::RIGHT_PARENTHESIS: return os << "RIGHT_PARENTHESIS";
        case TokenType::LEFT_BRACKET: return os << "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return os << "RIGHT_BRACKET";
        case TokenType::LEFT_SQUARE_BRACKET: return os << "LEFT_SQUARE_BRACKET";
        case TokenType::RIGHT_SQUARE_BRACKET: return os << "RIGHT_SQUARE_BRACKET";
        case TokenType::REFERENCE_EQUAL: return os << "REFERENCE_EQUAL";
        case TokenType::VALUE_EQUAL: return os << "VALUE_EQUAL";
        case TokenType::ASSIGN: return os << "ASSIGN";
        case TokenType::DECREMENT: return os << "DECREMENT";
        case TokenType::ASSIGN_MINUS: return os << "ASSIGN_MINUS";
        case TokenType::MINUS: return os << "MINUS";
        case TokenType::EOL: return os << "EOL";
        case TokenType::POWER_ASSIGN: return os << "SQUARE_ASSIGN";
        case TokenType::POWER: return os << "SQUARE";
        case TokenType::ASSIGN_MUL: return os << "ASSIGN_MUL";
        case TokenType::MUL: return os << "MUL";
        case TokenType::ASSIGN_DIV: return os << "ASSIGN_DIV";
        case TokenType::DIV: return os << "DIV";
        case TokenType::INCREMENT: return os << "INCREMENT";
        case TokenType::ASSIGN_ADD: return os << "ASSIGN_ADD";
        case TokenType::ADD: return os << "ADD";
        case TokenType::MODULI_ASSIGN: return os << "MODULI_ASSIGN";
        case TokenType::MODULI: return os << "MODULI";
        case TokenType::BITWISE_LEFT_SHIFT_ASSIGN: return os << "BITWISE_LEFT_SHIFT_ASSIGN";
        case TokenType::BITWISE_LEFT_SHIFT: return os << "BITWISE_LEFT_SHIFT";
        case TokenType::LESS_THAN_EQUAL: return os << "LESS_THAN_ASSIGN";
        case TokenType::LESS_THAN: return os << "LESS_THAN";
        case TokenType::UNSIGNED_RIGHT_SHIFT_ASSIGN: return os << "UNSIGNED_RIGHT_SHIFT_ASSIGN";
        case TokenType::UNSIGNED_RIGHT_SHIFT: return os << "UNSIGNED_RIGHT_SHIFT";
        case TokenType::BITWISE_RIGHT_SHIFT_ASSIGN: return os << "BITWISE_RIGHT_SHIFT_ASSIGN";
        case TokenType::BITWISE_RIGHT_SHIFT: return os << "BITWISE_RIGHT_SHIFT";
        case TokenType::GREATER_THAN_EQUAL: return os << "GREATER_THAN_ASSIGN";
        case TokenType::GREATER_THAN: return os << "GREATER_THAN";
        case TokenType::LOGICAL_AND_ASSIGN: return os << "LOGICAL_AND_ASSIGN";
        case TokenType::LOGICAL_AND: return os << "LOGICAL_AND";
        case TokenType::BITWISE_AND_ASSIGN: return os << "BITWISE_AND_ASSIGN";
        case TokenType::BITWISE_AND: return os << "BITWISE_AND";
        case TokenType::LOGICAL_OR_ASSIGN: return os << "LOGICAL_OR_ASSIGN";
        case TokenType::LOGICAL_OR: return os << "LOGICAL_OR";
        case TokenType::BITWISE_OR_ASSIGN: return os << "BITWISE_OR_ASSIGN";
        case TokenType::BITWISE_OR: return os << "BITWISE_OR";
        case TokenType::STRICT_INEQUALITY: return os << "STRICT_INEQUALITY";
        case TokenType::INEQUALITY: return os << "INEQUALITY";
        case TokenType::LOGICAL_NOT: return os << "LOGICAL_NOT";
        case TokenType::NULLISH_COALESCING_ASSIGN: return os << "NULLISH_COALESCING_ASSIGN";
        case TokenType::NULLISH_COALESCING: return os << "NULLISH_COALESCING";
        case TokenType::OPTIONAL_CHAINING: return os << "OPTIONAL_CHAINING";
        case TokenType::TERNARY: return os << "TERNARY";
        case TokenType::BITWISE_NOT: return os << "BITWISE_NOT";
        case TokenType::BITWISE_XOR_ASSIGN: return os << "BITWISE_XOR_ASSIGN";
        case TokenType::BITWISE_XOR: return os << "BITWISE_XOR";
        case TokenType::STRING: return os << "STRING";
        case TokenType::NUMBER: return os << "NUMBER";
        case TokenType::IDENTIFIER: return os << "IDENTIFIER";
        case TokenType::BOOLEAN: return os << "BOOLEAN";
        case TokenType::KEYWORD: return os << "KEYWORD";
        case TokenType::END_OF_FILE: return os << "END_OF_FILE";
        default: return os << "UNKNOWN";
    }
}

#endif /* operators_h */
