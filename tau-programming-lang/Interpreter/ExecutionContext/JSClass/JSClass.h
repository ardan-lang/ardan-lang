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
#include <unordered_map>
#include "../Value/Value.h"

class PropertyDeclaration;
class MethodDefinition;

using namespace std;

class JSClass {
public:
    string name;
    shared_ptr<JSClass> superClass;
    unordered_map<string, unique_ptr<PropertyDeclaration>> fields;
    unordered_map<string, unique_ptr<MethodDefinition>> methods;
    
    // need to add var, let, const fields
    
    Value get(const string& key);
    
};

#endif /* JSClass_h */
