//
//  StatementVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 18/08/2025.
//

#ifndef StatementVisitor_hpp
#define StatementVisitor_hpp

#include <stdio.h>

class BlockStatement;
class EmptyStatement;
class VarStatement;
class ExpressionStatement;

class StatementVisitor {
public:
    virtual void visit(BlockStatement& stmt) = 0;
    virtual void visit(EmptyStatement& stmt) = 0;
    virtual void visit(VarStatement& stmt) = 0;
    virtual void visit(ExpressionStatement& stmt) = 0;
    virtual ~StatementVisitor() = default;
};

#endif /* StatementVisitor_hpp */
