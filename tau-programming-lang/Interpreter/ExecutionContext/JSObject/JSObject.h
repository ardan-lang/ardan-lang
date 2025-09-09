//
//  JSObject.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef JSObject_h
#define JSObject_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>
//#include "Value.h"
//#include "JSClass.h"

#include "../Value/Value.h"
#include "../JSClass/JSClass.h"

using namespace std;

// class JSClass;

class JSObject {
    unordered_map<string, ValueField> var_properties;
    unordered_map<string, ValueField> let_properties;
    unordered_map<string, ValueField> const_properties;

    // class of this object
    shared_ptr<JSClass> js_class;

public:

    shared_ptr<JSClass> parent_class;
    shared_ptr<JSObject> parent_object;
    
    bool operator==(const JSObject& other) const;
    bool operator!=(const JSObject& other) const;
    
    void set(const string& key, const Value& val);
    void set(const string& key, const Value& val, string type, vector<string> modifiers);

    Value get(const string& key);

    void setClass(shared_ptr<JSClass> js_klass);
    
    const unordered_map<string, Value> get_all_properties();
    
    shared_ptr<JSClass> getKlass();
    
};

#endif /* JSObject_h */
