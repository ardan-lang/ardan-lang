//
//  PrintVisitor.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 18/08/2025.
//

#ifndef PrintVisitor_hpp
#define PrintVisitor_hpp

#include <stdio.h>
#include <iostream>
#include <cstring>

#include "../Statements/StatementVisitor.hpp"
#include "../Statements/Statements.hpp"
#include "../Expression/Expression.hpp"
#include "../ExpressionVisitor/ExpressionVisitor.hpp"
#include "AstPrinter/AstPrinter.h"

using namespace std;

class PrintVisitor : public StatementVisitor {
public:
    AstPrinter printer;
    void visit(BlockStatement& stmt) override {
        cout << "BlockStatement {" << endl;
        for (auto& s : stmt.statements) {
            s->accept(*this);
        }
        cout << "}" << endl;
    }

    void visit(EmptyStatement& stmt) override {
        cout << "EmptyStatement;" << endl;
    }
    
    void visit(VarStatement& stmt) override {
        cout << "VarStatement: { " << stmt.name << " = ";
        
        printer.print(stmt.init.get());
        
        cout << " } " << endl;
    }
    
    void visit(ExpressionStatement& stmt) override {
        cout << "ExpressionStatement: { ";
        printer.print(stmt.expression.get());
        cout << " } " << endl;
    }
    
};

#endif /* PrintVisitor_hpp */
