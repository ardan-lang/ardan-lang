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
#include "StatementVisitor.hpp"

using namespace std;

class Statement {
public:
    virtual void accept(StatementVisitor& visitor) = 0;
    virtual ~Statement() = default;
};

class BlockStatement : public Statement {
public:
    vector<unique_ptr<Statement>> body;

    BlockStatement(vector<unique_ptr<Statement>> body)
        : body(std::move(body)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitBlock(this);
    }
};

class EmptyStatement : public Statement {
public:
    EmptyStatement() {}
    void accept(StatementVisitor& visitor) override {
        visitor.visitEmpty(this);
    }
    
};

class ExpressionStatement : public Statement {
public:
    unique_ptr<Expression> expression;

    ExpressionStatement(unique_ptr<Expression> expression)
        : expression(std::move(expression)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitExpression(this);
    }
};

class IfStatement : public Statement {
public:
    unique_ptr<Expression> test;
    unique_ptr<Statement> consequent;
    unique_ptr<Statement> alternate; // may be null

    IfStatement(unique_ptr<Expression> test,
                unique_ptr<Statement> consequent,
                unique_ptr<Statement> alternate)
        : test(std::move(test)),
          consequent(std::move(consequent)),
          alternate(std::move(alternate)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitIf(this);
    }
};

class WhileStatement : public Statement {
public:
    unique_ptr<Expression> test;
    unique_ptr<Statement> body;

    WhileStatement(unique_ptr<Expression> test,
                   unique_ptr<Statement> body)
        : test(std::move(test)), body(std::move(body)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitWhile(this);
    }
};

class ForStatement : public Statement {
public:
    unique_ptr<Statement> init;       // may be null
    unique_ptr<Expression> test;      // may be null
    unique_ptr<Expression> update;    // may be null
    unique_ptr<Statement> body;

    ForStatement(unique_ptr<Statement> init,
                 unique_ptr<Expression> test,
                 unique_ptr<Expression> update,
                 unique_ptr<Statement> body)
        : init(std::move(init)),
          test(std::move(test)),
          update(std::move(update)),
          body(std::move(body)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitFor(this);
    }
};

struct VariableDeclarator {
    string id;
    unique_ptr<Expression> init; // may be null
};

class VariableStatement : public Statement {
public:
    string kind; // "var", "let", or "const"
    vector<VariableDeclarator> declarations;

    VariableStatement(string kind, vector<VariableDeclarator> declarations)
        : kind(std::move(kind)), declarations(std::move(declarations)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitVariable(this);
    }
};

class FunctionDeclaration : public Statement {
public:
    string id;
    vector<string> params;
    unique_ptr<Statement> body;

    FunctionDeclaration(string id,
                        vector<string> params,
                        unique_ptr<Statement> body)
        : id(std::move(id)),
          params(std::move(params)),
          body(std::move(body)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitFunction(this);
    }
};

// --- ReturnStatement ---
class ReturnStatement : public Statement {
public:
    unique_ptr<Expression> argument;

    ReturnStatement(unique_ptr<Expression> argument)
        : argument(std::move(argument)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitReturn(this);
    }
};

// --- BreakStatement ---
class BreakStatement : public Statement {
public:
    string label;

    BreakStatement(string label = "")
        : label(std::move(label)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitBreak(this);
    }
};

// --- ContinueStatement ---
class ContinueStatement : public Statement {
public:
    string label;

    ContinueStatement(string label = "")
        : label(std::move(label)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitContinue(this);
    }
};

// --- ThrowStatement ---
class ThrowStatement : public Statement {
public:
    unique_ptr<Expression> argument;

    ThrowStatement(unique_ptr<Expression> argument)
        : argument(std::move(argument)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitThrow(this);
    }
};

// --- TryCatchStatement ---
class TryCatchStatement : public Statement {
public:
    unique_ptr<BlockStatement> tryBlock;
    string catchParam; // identifier inside catch
    unique_ptr<BlockStatement> catchBlock;
    unique_ptr<BlockStatement> finallyBlock;

    TryCatchStatement(unique_ptr<BlockStatement> tryBlock,
                      string catchParam,
                      unique_ptr<BlockStatement> catchBlock,
                      unique_ptr<BlockStatement> finallyBlock)
        : tryBlock(std::move(tryBlock)),
          catchParam(std::move(catchParam)),
          catchBlock(std::move(catchBlock)),
          finallyBlock(std::move(finallyBlock)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitTryCatch(this);
    }
};

#endif /* Statements_hpp */
