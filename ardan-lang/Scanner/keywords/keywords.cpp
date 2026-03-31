//
//  keywords.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 31/03/2026.
//

#include "./keywords.h"

std::unordered_map<std::string, std::string> keywords = {
    
    // Control flow
    {"if", "IF"}, {"else", "ELSE"},
    {"switch", "SWITCH"}, {"case", "CASE"}, {"default", "DEFAULT"},
    {"for", "FOR"}, {"while", "WHILE"}, {"do", "DO"},
    {"break", "BREAK"}, {"continue", "CONTINUE"},
    {"return", "RETURN"}, {"throw", "THROW"},
    {"try", "TRY"}, {"catch", "CATCH"}, {"finally", "FINALLY"},

    // Declarations
    {"var", "VAR"}, {"let", "LET"}, {"const", "CONST"},
    {"function", "FUNCTION"}, {"class", "CLASS"}, {"extends", "EXTENDS"},
    {"import", "IMPORT"}, {"export", "EXPORT"},

    // Operators / expressions
    {"new", "NEW"}, {"delete", "DELETE"}, {"typeof", "TYPEOF"},
    {"instanceof", "INSTANCEOF"}, {"in", "IN"}, {"void", "VOID"},
    {"yield", "YIELD"}, {"await", "AWAIT"}, {"async", "ASYNC"},
    
    {"readonly", "READONLY"},

    // Literals / special
    {"true", "TRUE"}, {"false", "FALSE"}, {"null", "NULL"},
    {"this", "THIS"}, {"super", "SUPER"},

    // Reserved / future
    {"enum", "ENUM"}, {"implements", "IMPLEMENTS"}, {"interface", "INTERFACE"},
    {"package", "PACKAGE"}, {"private", "PRIVATE"}, {"protected", "PROTECTED"},
    {"public", "PUBLIC"}, {"static", "STATIC"},

    // Module-specific
    {"as", "AS"}, {"from", "FROM"}, {"of", "OF"}
};
