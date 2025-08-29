//
//  JSClass.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef JSClass_h
#define JSClass_h

#include <memory>
#include <string>
#include <vector>

class PropertyDeclaration;
class MethodDefinition;

using namespace std;

class JSClass {

public:
    string name;
    string superClass;
    unordered_map<string, unique_ptr<PropertyDeclaration>> fields;
    unordered_map<string, unique_ptr<MethodDefinition>> methods;
    
};

#endif /* JSClass_h */
