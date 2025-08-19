//
//  Statements.cpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 18/08/2025.
//

#include "Statements.hpp"
#include "StatementVisitor.hpp"

void BlockStatement::accept(StatementVisitor& visitor) {
    visitor.visit(*this);
}

void EmptyStatement::accept(StatementVisitor& visitor) {
    visitor.visit(*this);
}
