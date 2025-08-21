//
//  Token.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 13/08/2025.
//

#ifndef Token_hpp
#define Token_hpp

#include <stdio.h>
#include <iostream>
#include "TokenType.h"

using namespace std;

class Token {

public:
    TokenType type;
    string lexeme;
    
};

#endif /* Token_hpp */
