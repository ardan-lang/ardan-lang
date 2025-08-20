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

using namespace std;

class PrintVisitor : public StatementVisitor {
public:
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
        cout << "VarStatement;" << endl;
    }
    
    void visit(ExpressionStatement& stmt) override {
        cout << "ExpressionStatement;" << endl;
    }

};

#endif /* PrintVisitor_hpp */
