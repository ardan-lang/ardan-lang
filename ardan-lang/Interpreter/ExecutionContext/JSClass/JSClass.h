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
    vector<string> get_static_modifiers(const string& key);
    // calling this, we don't need the modifiers because it has been set by visitClassDeclarartions
    void set(const string& key, Value value, bool perform_privacy_check);
    void check_privacy(const string& key);
    bool hasModifier(const vector<string>& mods, const string& name);

    // only called in visitClassDeclarations
    void set_var(const string& key, Value value, const vector<string> modifiers);
    void set_let(const string& key, Value value, const vector<string> modifiers);
    void set_const(const string& key, Value value, const vector<string> modifiers);

    // Compiler Use only
        
    // non-static properties
    unordered_map<string, ValueField> var_proto_props;
    unordered_map<string, ValueField> const_proto_props;

    Value get_proto_vm(const string& key);

    void set_proto_vm_var(const string& key, Value value, const vector<string> modifiers);
    void set_proto_vm_const(const string& key, Value value, const vector<string> modifiers);
    bool is_constructor_available();
    Value get_constructor();
    
    virtual std::shared_ptr<JSObject> construct() {
        return nullptr;
    }
    
    virtual ~JSClass(); // declared only
    
};

#endif /* JSClass_h */
