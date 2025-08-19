//
//  Statements.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 18/08/2025.
//
// Statements.hpp
#ifndef Statements_hpp
#define Statements_hpp

#include <iostream>
#include <memory>
#include <vector>
// #include "StatementVisitor.hpp"

using namespace std;

// Forward declare visitor
class StatementVisitor;

class Statement {
public:
    virtual void accept(StatementVisitor& visitor) = 0;
    virtual ~Statement() = default;
};

class BlockStatement : public Statement {
public:
    vector<unique_ptr<Statement>> statements;

    void accept(StatementVisitor& visitor) override;
};

class EmptyStatement : public Statement {
public:
    void accept(StatementVisitor& visitor) override;
};

#endif /* Statements_hpp */
