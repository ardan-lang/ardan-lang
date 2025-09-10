//
//  REPL.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/09/2025.
//

#ifndef REPL_hpp
#define REPL_hpp

#include <stdio.h>
#include <iostream>
#include <string>

#include "../Scanner/Scanner.hpp"
#include "../overloads/operators.h"
#include "../Parser/Parser.hpp"
#include "../Interpreter/Env.h"

#include "../Interpreter/Interpreter.h"

using namespace std;

class REPL {
public:
    Env* env;
    Interpreter* interpreter;
    void start_repl();
    bool evalLine(string& line, string& result);
    
};

#endif /* REPL_hpp */
