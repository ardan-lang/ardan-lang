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
#include <cstring>
#include "../Expression/Expression.hpp"

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

class VarStatement : public Statement {
public:
    VarStatement(const string name, unique_ptr<Expression> init) : name(name), init(std::move(init)) {};
    const string name;
    unique_ptr<Expression> init;
    void accept(StatementVisitor& visitor) override;
};

class ExpressionStatement : public Statement {
public:
    ExpressionStatement(unique_ptr<Expression> expr) : expression(std::move(expr)) {};
    unique_ptr<Expression> expression;
    
    void accept(StatementVisitor& visitor) override;
};

#endif /* Statements_hpp */
