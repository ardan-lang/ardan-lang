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
#include "../ExecutionContext/JSObject.h"

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
    return std::visit([](auto&& lhs, auto&& rhs) -> bool {
        using L = std::decay_t<decltype(lhs)>;
        using Rhs = std::decay_t<decltype(rhs)>;

        // --- Nullish (undefined / null) ---
        if constexpr ((std::is_same_v<L, std::monostate> || std::is_same_v<L, std::nullptr_t>) ||
                      (std::is_same_v<Rhs, std::monostate> || std::is_same_v<Rhs, std::nullptr_t>)) {
            return std::is_same_v<L, Rhs>; // only equal if *both* are null-like
        }

        // --- Numbers (int, double, size_t, char) ---
        else if constexpr (std::is_arithmetic_v<L> && std::is_arithmetic_v<Rhs>) {
            return static_cast<long double>(lhs) == static_cast<long double>(rhs);
        }

        // --- String strict comparison ---
        else if constexpr (std::is_same_v<L, std::string> && std::is_same_v<Rhs, std::string>) {
            return lhs == rhs;
        }

        // --- String <-> Number coercion (like JS `==`) ---
        else if constexpr (std::is_same_v<L, std::string> && std::is_arithmetic_v<Rhs>) {
            try { return std::stold(lhs) == static_cast<long double>(rhs); }
            catch (...) { return false; }
        }
        else if constexpr (std::is_arithmetic_v<L> && std::is_same_v<Rhs, std::string>) {
            try { return static_cast<long double>(lhs) == std::stold(rhs); }
            catch (...) { return false; }
        }

        // --- Reference equality for objects ---
        else if constexpr (std::is_same_v<L, std::shared_ptr<JSObject>> &&
                           std::is_same_v<Rhs, std::shared_ptr<JSObject>>) {
            return lhs == rhs; // true only if both point to same object
        }

        // --- Fallback strict comparison (bool, char, etc.) ---
        else if constexpr (std::equality_comparable_with<L, Rhs>) {
            return lhs == rhs;
        }

        // --- Otherwise not comparable ---
        else {
            return false;
        }
    }, a, b);
}

inline Value toValue(const R& r) {
    Value v;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
            v.type = ValueType::UNDEFINED;
        }
        else if constexpr (std::is_same_v<T, std::nullptr_t>) {
            v.type = ValueType::NULLTYPE;
        }
        else if constexpr (std::is_same_v<T, double>) {
            v.type = ValueType::NUMBER;
            v.numberValue = arg;
        }
        else if constexpr (std::is_same_v<T, size_t> ||
                           std::is_same_v<T, int>   ||
                           std::is_same_v<T, char>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<double>(arg);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            v.type = ValueType::STRING;
            v.stringValue = arg;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            v.type = ValueType::BOOLEAN;
            v.boolValue = arg;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<JSObject>>) {
            v.type = ValueType::OBJECT;
            v.objectValue = arg;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<Value>>) {
            // unwrap nested Value
            v = *arg;
        }
    }, r);

    return v;
}

inline void printValue(const R& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        std::cout << "nil";
    } else if (std::holds_alternative<double>(value)) {
        std::cout << std::get<double>(value);
    } else if (std::holds_alternative<std::string>(value)) {
        std::cout << std::get<std::string>(value);
    } else if (std::holds_alternative<bool>(value)) {
        std::cout << (std::get<bool>(value) ? "true" : "false");
    } else if (std::holds_alternative<shared_ptr<Value>>(value)) {
        std::cout << (std::get<shared_ptr<Value>>(value))->toString();
    }
}

#endif /* Utils_h */
