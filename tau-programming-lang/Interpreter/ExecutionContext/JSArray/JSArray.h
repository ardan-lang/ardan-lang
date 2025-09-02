//
//  JSArray.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 28/08/2025.
//

#ifndef JSArray_h
#define JSArray_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <any>
#include "JSObject.h"

using namespace std;

class JSArray : public JSObject {

    int elements_size;

public:
    void setIndex(size_t i, const Value& val);

    Value getIndex(size_t i);

    void updateLength(size_t len);
        
};

#endif /* JSArray_h */
