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
#include <map>
#include "../Expression/Expression.hpp"
#include "StatementVisitor.hpp"
#include "../Interpreter/R.hpp"

using namespace std;

class Statement {
public:
    virtual R accept(StatementVisitor& visitor) = 0;
    virtual ~Statement() = default;
};

class EmptyStatement : public Statement {
public:
    EmptyStatement() {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitEmpty(this);
    }
    
};

class BlockStatement : public Statement {
public:
    vector<unique_ptr<Statement>> body;

    BlockStatement(vector<unique_ptr<Statement>> body)
        : body(std::move(body)) {}

    R accept(StatementVisitor& visitor) override {
        return visitor.visitBlock(this);
    }
};

class ExpressionStatement : public Statement {
public:
    unique_ptr<Expression> expression;

    ExpressionStatement(unique_ptr<Expression> expression)
        : expression(std::move(expression)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitExpression(this);
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

    R accept(StatementVisitor& visitor) {
        return visitor.visitIf(this);
    }
};

class WhileStatement : public Statement {
public:
    unique_ptr<Expression> test;
    unique_ptr<Statement> body;

    WhileStatement(unique_ptr<Expression> test,
                   unique_ptr<Statement> body)
        : test(std::move(test)), body(std::move(body)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitWhile(this);
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

    R accept(StatementVisitor& visitor) {
        return visitor.visitFor(this);
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
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitForIn(this);
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

    R accept(StatementVisitor& visitor) {
        return visitor.visitForOf(this);
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

    R accept(StatementVisitor& visitor) {
        return visitor.visitVariable(this);
    }
};

class FunctionDeclaration : public Statement {
public:
    string id;
    vector<unique_ptr<Expression>> params;
    unique_ptr<Statement> body;
    bool is_async;

    FunctionDeclaration(string id,
                        vector<unique_ptr<Expression>> params,
                        unique_ptr<Statement> body)
        : id(std::move(id)),
          params(std::move(params)),
    body(std::move(body)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitFunction(this);
    }
};

// --- ReturnStatement ---
class ReturnStatement : public Statement {
public:
    unique_ptr<Expression> argument;

    ReturnStatement(unique_ptr<Expression> argument)
        : argument(std::move(argument)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitReturn(this);
    }
};

// --- BreakStatement ---
class BreakStatement : public Statement {
public:
    string label;

    BreakStatement(string label = "")
        : label(std::move(label)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitBreak(this);
    }
};

// --- ContinueStatement ---
class ContinueStatement : public Statement {
public:
    string label;

    ContinueStatement(string label = "")
        : label(std::move(label)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitContinue(this);
    }
};

// --- ThrowStatement ---
class ThrowStatement : public Statement {
public:
    unique_ptr<Expression> argument;

    ThrowStatement(unique_ptr<Expression> argument)
        : argument(std::move(argument)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitThrow(this);
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

    R accept(StatementVisitor& visitor) {
        return visitor.visitMethodDefinition(this);
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

    R accept(StatementVisitor& visitor) {
        return visitor.visitClass(this);
    }
};

class DoWhileStatement : public Statement {
public:
    unique_ptr<Statement> body;
    unique_ptr<Expression> condition;
    DoWhileStatement(unique_ptr<Statement> body,
                     unique_ptr<Expression> condition) : body(std::move(body)), condition(std::move(condition)) {}
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitDoWhile(this);
    }

};

class SwitchCase : public Statement {
public:
    unique_ptr<Expression> test;
    
    // TODO: change this to body
    vector<unique_ptr<Statement>> consequent;
    
    unique_ptr<Statement> body;

    SwitchCase(unique_ptr<Expression> test,
               vector<unique_ptr<Statement>> consequent) : test(std::move(test)), consequent(std::move(consequent)) {}
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitSwitchCase(this);
    }

};

class SwitchStatement : public Statement {
public:
    unique_ptr<Expression> discriminant;
    vector<unique_ptr<SwitchCase>> cases;
    
    SwitchStatement(unique_ptr<Expression> discriminant,
                    vector<unique_ptr<SwitchCase>> cases) : discriminant(std::move(discriminant)), cases(std::move(cases)) {};
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitSwitch(this);
    }

};

class CatchClause : public Statement {
public:
    string param;
    unique_ptr<Statement> body;
    
    CatchClause(string param,
                unique_ptr<Statement> body) : param(param), body(std::move(body)) {}
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitCatch(this);
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
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitTry(this);
    }

};

class ArrowFunction : public Expression {
public:
    string name = "<arrow>";
    unique_ptr<Expression> parameters;
    Token token;
    
    unique_ptr<Expression> exprBody;   // expression body (x => x + 1)
    unique_ptr<Statement> stmtBody; // block body (x => { return x + 1; })
    bool is_async;
    
    // x => x + 1
    ArrowFunction(Token token,
                  unique_ptr<Expression> exprBody)
    : token(token), exprBody(std::move(exprBody)) {}
    
    // x => { return (x + 1); }
    ArrowFunction(Token token,
                  unique_ptr<Statement> stmtBody)
    : token(token), stmtBody(std::move(stmtBody)) {}

    // (x, y) => x + 1
    ArrowFunction(unique_ptr<Expression> params,
                  unique_ptr<Expression> exprBody)
        : parameters(std::move(params)), exprBody(std::move(exprBody)) {}

    // (x, y) => { return (x + 1); }
    ArrowFunction(unique_ptr<Expression> params,
                  unique_ptr<Statement> stmtBody)
        : parameters(std::move(params)), stmtBody(std::move(stmtBody)) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitArrowFunction(this);
    }
    
};

class TemplateLiteral : public Expression {
public:
    vector<unique_ptr<StringLiteral>> quasis;
    vector<unique_ptr<Expression>> expressions;

    TemplateLiteral(vector<unique_ptr<StringLiteral>> quasis,
                    vector<unique_ptr<Expression>> expressions)
        : quasis(std::move(quasis)), expressions(std::move(expressions)) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitTemplateLiteral(this);
    }


};

//class TaggedTemplateExpression : public Expression {
//public:
//    unique_ptr<Expression> tag;                // the function
//    unique_ptr<TemplateLiteral> quasi;         // the template
//
//    TaggedTemplateExpression(unique_ptr<Expression> tag,
//                             unique_ptr<TemplateLiteral> quasi)
//        : tag(std::move(tag)), quasi(std::move(quasi)) {}
//
//    R accept(ExpressionVisitor& visitor) override {
//        return visitor.visitTaggedTemplate(this);
//    }
//};

class ImportDeclaration : public Statement {
public:
    
    Token path;
    string sourceFile;
    ImportDeclaration(Token path, string sourceFile): path(path), sourceFile(sourceFile) {}
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitImportDeclaration(this);
    }
};

class FunctionExpression : public Expression {

public:
    Token token;
    string name = "<anon>";
    vector<unique_ptr<Expression>> params;
    unique_ptr<Statement> body;
    bool is_async;

    FunctionExpression(Token token, vector<unique_ptr<Expression>> params,
                        unique_ptr<Statement> body)
        : params(std::move(params)),
          body(std::move(body)), token(token) {}

    R accept(ExpressionVisitor& visitor) {
        return visitor.visitFunctionExpression(this);
    }

};

struct EnumMember {
    string name;
    unique_ptr<Expression> value;
    int computedValue;
    
    // EnumMember(const EnumMember&) = delete;
    // EnumMember& operator=(const EnumMember&) = delete;
    // EnumMember(EnumMember&&) = default;
    // EnumMember& operator=(EnumMember&&) = default;
    
};

class EnumDeclaration : public Statement {
public:
    Token token;
    string name;
    vector<EnumMember> members;
    
    EnumDeclaration(Token token, string name, vector<EnumMember> members)
    : token(token), name(name), members(std::move(members)) {}
    
    R accept(StatementVisitor& visitor) {
        return visitor.visitEnumDeclaration(this);
    }

};

//enum class InterfaceType {
//    MethodSignature,
//    PropertySignature
//};
//
//struct InterfaceMember {
//    string name;
//    InterfaceType type;
//    vector<Expression> modifiers;
//    vector<Expression> parameters;
//};
//
//class InterfaceStatement : public Statement {
//public:
//    string name;
//    Token token;
//    vector<InterfaceMember> members;
//    
//    InterfaceStatement(string name,
//                       Token token,
//                       vector<InterfaceMember> members):
//    name(name), token(token), members(std::move(members)) {}
//    
//    R accept(StatementVisitor& visitor) {
//        return visitor.visitInterfaceStatement(this);
//    }
//
//};

class ClassExpression : public Expression {
public:
    Token token;
    string name;
    unique_ptr<Expression> superClass;
    vector<unique_ptr<MethodDefinition>> body;
    vector<unique_ptr<PropertyDeclaration>> fields;

    ClassExpression(Token token, unique_ptr<Expression> superClass,
                     vector<unique_ptr<MethodDefinition>> body,
                     vector<unique_ptr<PropertyDeclaration>> fields)
        : superClass(std::move(superClass)),
          body(std::move(body)),
          fields(std::move(fields)),
          token(token) {}

    R accept(ExpressionVisitor& visitor) {
        return visitor.visitClassExpression(this);
    }

};

class UIViewExpression : public Expression {
public:
    string name;
    string viewType;
    vector<unique_ptr<Expression>> args;
    vector<unique_ptr<Statement>> children;
    vector<unique_ptr<Expression>> modifiers;
    
    UIViewExpression(string name,
                     string viewType,
                     vector<std::unique_ptr<Expression>> args,
                     vector<std::unique_ptr<Statement>> children,
                     vector<unique_ptr<Expression>> modifiers)
    :
    name(name),
    viewType(viewType),
    args(std::move(args)),
    children(std::move(children)),
    modifiers(std::move(modifiers)) {}
    
    R accept(ExpressionVisitor& visitor) {
        return visitor.visitUIExpression(this);
    }

};

// ---------interface----------

struct TypeNode {
    virtual ~TypeNode() = default;
};

// Identifier reference to a type (e.g., "User", "Promise<T>")
//struct TypeReference : public TypeNode {
//    Token name;
//    vector<unique_ptr<TypeNode>> typeArguments;
//
//    TypeReference(Token name, vector<unique_ptr<TypeNode>> typeArguments = {})
//        : name(name), typeArguments(std::move(typeArguments)) {}
//};

// Array type: string[]
struct ArrayTypeNode : public TypeNode {
    unique_ptr<TypeNode> elementType;
    ArrayTypeNode(unique_ptr<TypeNode> elementType)
        : elementType(std::move(elementType)) {}
};

// Union type: A | B
struct UnionTypeNode : public TypeNode {
    vector<unique_ptr<TypeNode>> types;
    UnionTypeNode(vector<unique_ptr<TypeNode>> types)
        : types(std::move(types)) {}
};

// Literal type: "ok" 42
struct LiteralTypeNode : public TypeNode {
    Token literal;
    LiteralTypeNode(Token literal) : literal(literal) {}
};

// Function type: (a: string) => number
struct FunctionTypeNode : public TypeNode {
    vector<pair<string, unique_ptr<TypeNode>>> parameters;
    unique_ptr<TypeNode> returnType;

    FunctionTypeNode(vector<pair<string, unique_ptr<TypeNode>>> params,
                     unique_ptr<TypeNode> returnType)
        : parameters(std::move(params)), returnType(std::move(returnType)) {}
};

// Tuple type: [string, number]
struct TupleTypeNode : public TypeNode {
    vector<unique_ptr<TypeNode>> elements;
    TupleTypeNode(vector<unique_ptr<TypeNode>> elements)
        : elements(std::move(elements)) {}
};

// Parenthesized type: (A | B)
struct ParenthesizedTypeNode : public TypeNode {
    unique_ptr<TypeNode> inner;
    ParenthesizedTypeNode(unique_ptr<TypeNode> inner)
        : inner(std::move(inner)) {}
};

// -----------------------------
// TypeReference Node
// -----------------------------
struct TypeReference {
    string name;
    vector<unique_ptr<TypeReference>> typeArguments;

    TypeReference(string name) : name(std::move(name)) {}
};

// -----------------------------
// TypeParameter (Generics)
// -----------------------------
struct TypeParameter {
    string name;
    unique_ptr<TypeReference> constraint;   // e.g. T extends SomeInterface
    // unique_ptr<TypeReference> defaultType;  // e.g. T = string

    TypeParameter(string name, unique_ptr<TypeReference> constraint)
    : name(std::move(name)), constraint(std::move(constraint))/*, defaultType(nullptr)*/ {
    }
};

// -----------------------------
// Parameter Node
// -----------------------------
struct Parameter {
    string name;
    unique_ptr<TypeReference> type;
    bool optional;

    Parameter(string name, unique_ptr<TypeReference> type, bool optional = false)
        : name(std::move(name)), type(std::move(type)), optional(optional) {}
};

// =======================================================
// INTERFACE MEMBERS
// =======================================================

// Base class
struct InterfaceMember {
    virtual ~InterfaceMember() = default;
};

// -----------------------------
// PropertySignature
// -----------------------------
struct PropertySignature : InterfaceMember {
    string name;
    unique_ptr<TypeReference> type;
    bool optional;
    bool readonly;

    PropertySignature(string name, unique_ptr<TypeReference> type,
                      bool optional = false, bool readonly = false)
        : name(std::move(name)),
          type(std::move(type)),
          optional(optional),
          readonly(readonly) {}
};

// -----------------------------
// MethodSignature
// -----------------------------
struct MethodSignature : InterfaceMember {
    string name;
    vector<Parameter> parameters;
    unique_ptr<TypeReference> returnType;
    bool optional;

    MethodSignature(string name,
                    vector<Parameter> parameters,
                    unique_ptr<TypeReference> returnType,
                    bool optional = false)
        : name(std::move(name)),
          parameters(std::move(parameters)),
          returnType(std::move(returnType)),
          optional(optional) {}
};

// -----------------------------
// CallSignature
// -----------------------------
struct CallSignature : InterfaceMember {
    vector<Parameter> parameters;
    unique_ptr<TypeReference> returnType;

    CallSignature(vector<Parameter> parameters,
                  unique_ptr<TypeReference> returnType)
        : parameters(std::move(parameters)),
          returnType(std::move(returnType)) {}
};

// -----------------------------
// ConstructSignature
// -----------------------------
struct ConstructSignature : InterfaceMember {
    vector<Parameter> parameters;
    unique_ptr<TypeReference> returnType;

    ConstructSignature(vector<Parameter> parameters,
                       unique_ptr<TypeReference> returnType)
        : parameters(std::move(parameters)),
          returnType(std::move(returnType)) {}
};

// -----------------------------
// IndexSignature
// -----------------------------
struct IndexSignature : InterfaceMember {
    Parameter key;
    unique_ptr<TypeReference> valueType;

    IndexSignature(Parameter key, unique_ptr<TypeReference> valueType)
        : key(std::move(key)), valueType(std::move(valueType)) {}
};

// =======================================================
// INTERFACE DECLARATION
// =======================================================
struct InterfaceDeclaration : Statement {
    Token token;
    string name;
    vector<unique_ptr<TypeParameter>> typeParameters;
    vector<unique_ptr<TypeReference>> baseInterfaces; // extends clause
    vector<unique_ptr<InterfaceMember>> members;

    InterfaceDeclaration(Token token,
                         string name,
                         vector<unique_ptr<TypeParameter>> typeParameters,
                         vector<unique_ptr<TypeReference>> baseInterfaces,
                         vector<unique_ptr<InterfaceMember>> members)
        : token(std::move(token)),
          name(std::move(name)),
          typeParameters(std::move(typeParameters)),
          baseInterfaces(std::move(baseInterfaces)),
          members(std::move(members)) {}

    R accept(StatementVisitor& visitor) {
        return visitor.visitInterfaceDeclaration(this);
    }
    
};

// ---------end interface------------

#endif /* Statements_hpp */
