//
//  Utils.h
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 27/08/2025.
//

#ifndef Utils_h
#define Utils_h

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <stdexcept>

#include <cstring>
#include <fstream>
#include <sstream>

#include "../R.hpp"
#include "../ExecutionContext/JSObject/JSObject.h"

using namespace std;

const string CONST = "CONST";
const string LET = "LET";
const string VAR = "VAR";
const string PRIVATE = "private";
const string PROTECTED = "protected";
const string PUBLIC = "public";
const string CONSTRUCTOR = "constructor";

Value toValue(const R& r);
string toString(const R& val);
bool isNullish(const R& value);
bool truthy(const R& value);
bool equals(const R& a, const R& b);
string read_file(const string& filename);
void printValue(const R& value);

#endif /* Utils_h */
