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

bool equals(const R& a, const R& b) {
    return visit([](auto&& lhs, auto&& rhs) -> bool {
        using L = decay_t<decltype(lhs)>;
        using Rhs = decay_t<decltype(rhs)>;

        // Null / empty cases
        if constexpr (is_same_v<L, monostate> || is_same_v<L, nullptr_t> ||
                      is_same_v<Rhs, monostate> || is_same_v<Rhs, nullptr_t>) {
            return is_same_v<L, Rhs>; // only equal if *both* are null-like
        }
        // Numeric types (int, double, size_t, char treated as numeric)
        else if constexpr (is_arithmetic_v<L> && is_arithmetic_v<Rhs>) {
            return static_cast<long double>(lhs) == static_cast<long double>(rhs);
        }
        // String strict comparison
        else if constexpr (is_same_v<L, string> && is_same_v<Rhs, string>) {
            return lhs == rhs;
        }
        // Mixed string <-> numeric (optional feature)
        else if constexpr (is_same_v<L, string> && is_arithmetic_v<Rhs>) {
            try {
                return stold(lhs) == static_cast<long double>(rhs);
            } catch (...) { return false; }
        }
        else if constexpr (is_arithmetic_v<L> && is_same_v<Rhs, string>) {
            try {
                return static_cast<long double>(lhs) == stold(rhs);
            } catch (...) { return false; }
        }
        // Fallback strict compare
        else {
            return lhs == rhs;
        }
    }, a, b);
}

#endif /* Utils_h */
