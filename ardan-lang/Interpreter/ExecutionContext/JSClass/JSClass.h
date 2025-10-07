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

    unordered_map<string, ValueField> var_static_fields;
    unordered_map<string, ValueField> let_static_fields;
    unordered_map<string, ValueField> const_static_fields;
    bool is_native = false;

    // need to add var, let, const fields
    
    Value get(const string& key, bool perform_privacy_check);
    // calling this, we don't need the modifiers because it has been set by visitClassDeclarartions
    void set(const string& key, Value value, bool perform_privacy_check);
    void check_privacy(const string& key);
    bool hasModifier(const vector<string>& mods, const string& name);

    // only called in visitClassDeclarartions
    void set_var(const string& key, Value value, const vector<string> modifiers);
    void set_let(const string& key, Value value, const vector<string> modifiers);
    void set_const(const string& key, Value value, const vector<string> modifiers);

    // simple maps for static and prototype methods
    unordered_map<string, Value> staticProps;
    unordered_map<string, Value> protoProps;
    Value get_vm(const string& key);
    void set_static_vm(const string& key, Value value);
    void set_proto_vm(const string& key, Value value);

    virtual std::shared_ptr<JSObject> construct() {
        return nullptr;
    }
};

#endif /* JSClass_h */
