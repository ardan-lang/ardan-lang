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
#include "../R.hpp"

using namespace std;

string toString(const R& val) {
    if (holds_alternative<string>(val)) return get<string>(val);
    if (holds_alternative<double>(val)) return to_string(get<double>(val));
    if (holds_alternative<int>(val)) return to_string(get<int>(val));
    if (holds_alternative<size_t>(val)) return to_string(get<size_t>(val));
    if (holds_alternative<char>(val)) return string(1, get<char>(val));
    if (holds_alternative<bool>(val)) return get<bool>(val) ? "true" : "false";
    return "undefined"; // monostate
}

double toNumber(const R& val) {
    if (holds_alternative<double>(val)) return get<double>(val);
    if (holds_alternative<int>(val)) return static_cast<double>(get<int>(val));
    if (holds_alternative<size_t>(val)) return static_cast<double>(get<size_t>(val));
    if (holds_alternative<char>(val)) return static_cast<double>(get<char>(val));
    if (holds_alternative<bool>(val)) return get<bool>(val) ? 1.0 : 0.0;
    if (holds_alternative<string>(val)) {
        try {
            return stod(get<string>(val));
        } catch (...) {
            return nan(""); // JS: Number("abc") → NaN
        }
    }
    return nan(""); // monostate = undefined → NaN
}

bool isNullish(const R& value) {
    // nullish = null or undefined in JS
    if (holds_alternative<std::nullptr_t>(value)) return true;
    // if you modeled "undefined" as a string or custom type, check that too
    return false;
}

bool _truthy(const R& value) {
    if (holds_alternative<bool>(value)) {
        return get<bool>(value);
    }
    else if (holds_alternative<double>(value)) {
        return get<double>(value) != 0.0;
    }
    else if (holds_alternative<string>(value)) {
        return !get<string>(value).empty();
    }
    else if (holds_alternative<std::nullptr_t>(value)) {
        return false;
    }
    // fallback (objects, functions, etc. if you add them later)
    return true;
}

bool _isNullish(const R& value) {
    return holds_alternative<monostate>(value);
}

bool truthy(const R& value) {
    if (holds_alternative<bool>(value)) {
        return get<bool>(value);
    }
    else if (holds_alternative<double>(value)) {
        return get<double>(value) != 0.0;
    }
    else if (holds_alternative<int>(value)) {
        return get<int>(value) != 0;
    }
    else if (holds_alternative<size_t>(value)) {
        return get<size_t>(value) != 0;
    }
    else if (holds_alternative<char>(value)) {
        return get<char>(value) != '\0';
    }
    else if (holds_alternative<string>(value)) {
        return !get<string>(value).empty();
    }
    else if (holds_alternative<monostate>(value)) {
        return false; // undefined/null
    }
    return true; // fallback
}

#endif /* Utils_h */
