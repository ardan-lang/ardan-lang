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

class ForInStatement : public Statement {
public:
    unique_ptr<Statement> init;
    unique_ptr<Expression> object;
    unique_ptr<Statement> body;

    ForInStatement(unique_ptr<Statement> init,
                 unique_ptr<Expression> object,
                 unique_ptr<Statement> body)
        : init(std::move(init)),
          object(std::move(object)),
          body(std::move(body)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitForIn(this);
    }
};

class ForOfStatement : public Statement {
public:
    std::unique_ptr<Statement> left; // variable declaration or expression
    std::unique_ptr<Expression> right; // iterable expression
    std::unique_ptr<Statement> body;

    ForOfStatement(std::unique_ptr<Statement> left,
                   std::unique_ptr<Expression> right,
                   std::unique_ptr<Statement> body)
        : left(std::move(left)),
          right(std::move(right)),
          body(std::move(body)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitForOf(this);
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
    vector<unique_ptr<Expression>> params;
    unique_ptr<Statement> body;

    FunctionDeclaration(string id,
                        vector<unique_ptr<Expression>> params,
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

class MethodDefinition : public Statement {
public:
    const string name;
    vector<unique_ptr<Expression>> params;
    unique_ptr<Statement> methodBody;
    vector<unique_ptr<Expression>> modifiers;

    MethodDefinition(const string name,
                     vector<unique_ptr<Expression>> params,
                     unique_ptr<Statement> methodBody,
                     vector<unique_ptr<Expression>> modifiers)
        : name((name)),
          params(std::move(params)),
          methodBody(std::move(methodBody)),
          modifiers(std::move(modifiers)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitMethodDefinition(this);
    }
};

class PropertyDeclaration {
public:
    vector<unique_ptr<Expression>> modifiers;
    unique_ptr<Statement> property;
    PropertyDeclaration(
                        vector<unique_ptr<Expression>> modifiers,
                        unique_ptr<Statement> property
                        ) :
    modifiers(std::move(modifiers)),
    property(std::move(property)) {}
};

class ClassDeclaration : public Statement {
public:
    const string id;
    unique_ptr<Expression> superClass;
    vector<unique_ptr<MethodDefinition>> body;
    vector<unique_ptr<PropertyDeclaration>> fields;

    ClassDeclaration(const string id,
                     unique_ptr<Expression> superClass,
                     vector<unique_ptr<MethodDefinition>> body,
                     vector<unique_ptr<PropertyDeclaration>> fields)
        : id((id)),
          superClass(std::move(superClass)),
          body(std::move(body)),
          fields(std::move(fields)) {}

    void accept(StatementVisitor& visitor) override {
        visitor.visitClass(this);
    }
};

class DoWhileStatement : public Statement {
public:
    unique_ptr<Statement> body;
    unique_ptr<Expression> condition;
    DoWhileStatement(unique_ptr<Statement> body,
                     unique_ptr<Expression> condition) : body(std::move(body)), condition(std::move(condition)) {}
    
    void accept(StatementVisitor& visitor) override {
        visitor.visitDoWhile(this);
    }

};

class SwitchCase : public Statement {
public:
    unique_ptr<Expression> test;
    vector<unique_ptr<Statement>> consequent;
    
    SwitchCase(unique_ptr<Expression> test,
               vector<unique_ptr<Statement>> consequent) : test(std::move(test)), consequent(std::move(consequent)) {}
    
    void accept(StatementVisitor& visitor) override {
        visitor.visitSwitchCase(this);
    }

};

class SwitchStatement : public Statement {
public:
    unique_ptr<Expression> discriminant;
    vector<unique_ptr<SwitchCase>> cases;
    SwitchStatement(unique_ptr<Expression> discriminant,
                    vector<unique_ptr<SwitchCase>> cases) : discriminant(std::move(discriminant)), cases(std::move(cases)) {};
    
    void accept(StatementVisitor& visitor) override {
        visitor.visitSwitch(this);
    }

};

class CatchClause : public Statement {
public:
    string param;
    unique_ptr<Statement> body;
    
    CatchClause(string param,
                unique_ptr<Statement> body) : param(param), body(std::move(body)) {}
    
    void accept(StatementVisitor& visitor) override {
        visitor.visitCatch(this);
    }

};

class TryStatement : public Statement {
public:
    unique_ptr<Statement> block;
    unique_ptr<CatchClause> handler;
    unique_ptr<Statement> finalizer;
    
    TryStatement(unique_ptr<Statement> block,
                 unique_ptr<CatchClause> handler,
                 unique_ptr<Statement> finalizer) : block(std::move(block)), handler(std::move(handler)), finalizer(std::move(finalizer)) {}
    
      void accept(StatementVisitor& visitor) override {
          visitor.visitTry(this);
      }

};

#endif /* Statements_hpp */
