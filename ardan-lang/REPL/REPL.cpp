//
//  REPL.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 10/09/2025.
//

#include "REPL.hpp"

void REPL::start_repl() {

    env = new Env();
    interpreter = new Interpreter(env);
    
    string help = R"(Commands:
        :help       Show this message
        :quit       Exit REPL
        :clear      Clear all
    )";

    cout << "Simple Ardan REPL. Type :help\n";
    string line;
    
    while (true) {
        
        cout << ">>> " << flush;
        
        if (!std::getline(cin, line)) break;
        if (line.empty()) continue;
        
        string result;

        if (line == "clear" || line == ":cls" || line == ":clear") {
            #ifdef _WIN32
                system("cls");   // Windows
            #else
                system("clear"); // Linux / macOS
            #endif
                continue; // don't exit REPL, just clear screen
        }
        if (line == "quit" || line == ":q" || line == ":quit") break;
        if (line == "help" || line == ":h" || line == ":help") {
            cout << help << endl;
            continue;
        }
        if (line == ":env") {
            env->debugPrint(); // dump environment variables
            continue;
        }

        bool ok = evalLine(line, result);
        
        if (!ok) {
            cout << "Internal error\n";
            continue;
        }
        
        if (!result.empty()) cout << result << "\n";
        
    }
    
    cout << "Bye!\n";
    
}


bool REPL::evalLine(string& line, string& result) {
    
    Scanner scanner(line);
    
    auto tokens = scanner.getTokens();

    Parser parser(tokens);
    auto ast = parser.parse();

    interpreter->execute(std::move(ast));
    
    return true;
}
