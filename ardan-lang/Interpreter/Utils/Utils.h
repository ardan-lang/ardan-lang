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
        else if constexpr (std::is_same_v<T, size_t>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<size_t>(arg);
        }
        else if constexpr (std::is_same_v<T, int>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<int>(arg);
        }
        else if constexpr (std::is_same_v<T, unsigned short>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<unsigned short>(arg);
        }
        else if constexpr (std::is_same_v<T, unsigned long>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<unsigned long>(arg);
        }
        
        // ******
        
        else if constexpr (std::is_same_v<T, long>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<long>(arg);
        }
        else if constexpr (std::is_same_v<T, long long>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<long long>(arg);
        }
        else if constexpr (std::is_same_v<T,short>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<short>(arg);
        }
        else if constexpr (std::is_same_v<T, unsigned int>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<unsigned int>(arg);
        }
        else if constexpr (std::is_same_v<T, unsigned long long>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<unsigned long long>(arg);
        }
        else if constexpr (std::is_same_v<T, float>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<float>(arg);
        }
        else if constexpr (std::is_same_v<T, long double>) {
            v.type = ValueType::NUMBER;
            v.numberValue = static_cast<long double>(arg);
        }
        
        // ******
        
        else if constexpr (std::is_same_v<T, char>) {
            v.type = ValueType::STRING;
            v.stringValue = arg;
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
        else if constexpr (std::is_same_v<T, std::shared_ptr<JSArray>>) {
            v.type = ValueType::ARRAY;
            v.arrayValue = arg;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<Value>>) {
            // unwrap nested Value
            v = *arg;
        }
        else if constexpr (std::is_same_v<T, Value>) {
            // unwrap nested Value
            v = arg;
        }
    }, r);
    
    return v;
    
}

string toString(const R& val) {
    if (holds_alternative<string>(val)) return get<string>(val);
    if (holds_alternative<double>(val)) return to_string(get<double>(val));
    if (holds_alternative<int>(val)) return to_string(get<int>(val));
    if (holds_alternative<size_t>(val)) return to_string(get<size_t>(val));
    if (holds_alternative<unsigned long>(val)) return to_string(get<unsigned long>(val));
    if (holds_alternative<char>(val)) return string(1, get<char>(val));
    if (holds_alternative<bool>(val)) return get<bool>(val) ? "true" : "false";
    if (holds_alternative<shared_ptr<Value>>(val)) {
        return (get<shared_ptr<Value>>(val))->toString();
   }
    if (std::holds_alternative<Value>(val)) {
        return (get<Value>(val)).toString();
   }
    return "undefined"; // monostate
}

bool isNullish(const R& value) {
    // nullish = null or undefined in JS
    if (holds_alternative<std::nullptr_t>(value)) return true;
    
    bool bool_value = toValue(value).isNull();
    return bool_value;
    
    // if you modeled "undefined" as a string or custom type, check that too
    return false;
}

bool truthy(const R& value) {
    
    //    return std::visit([](auto&& v) -> bool {
    //        using T = std::decay_t<decltype(v)>;
    //
    //        if constexpr (std::is_same_v<T, std::monostate>) {
    //            return false;  // like "undefined"
    //        }
    //        else if constexpr (std::is_same_v<T, std::nullptr_t>) {
    //            return false;  // null is falsy
    //        }
    //        else if constexpr (std::is_same_v<T, bool>) {
    //            return v;  // just return the bool
    //        }
    //        else if constexpr (std::is_arithmetic_v<T>) {
    //            return v != 0;  // all ints, floats, doubles handled here
    //        }
    //        else if constexpr (std::is_same_v<T, std::string>) {
    //            return !v.empty();  // non-empty string = truthy
    //        }
    //        else if constexpr (std::is_pointer_v<T>) {
    //            return v != nullptr;  // raw pointers
    //        }
    //        else if constexpr (std::is_same_v<T, std::shared_ptr<JSObject>> ||
    //                           std::is_same_v<T, std::shared_ptr<Value>>   ||
    //                           std::is_same_v<T, std::shared_ptr<JSClass>> ||
    //                           std::is_same_v<T, std::shared_ptr<JSArray>>) {
    //            return static_cast<bool>(v);  // check non-null
    //        }
    //        else if constexpr (std::is_same_v<T, Value>) {
    //            // recursive unwrap if needed
    //            return truthy(v);
    //        }
    //        else {
    //            return true; // default truthy (objects, etc.)
    //        }
    //    }, value);
    
//    if (holds_alternative<monostate>(value)) {
//        return false;
//    }
//    
//    if (holds_alternative<std::nullptr_t>(value)) {
//        return false;
//    }
//    
//    if (holds_alternative<double>(value)) {
//        
//    }
//    if (holds_alternative<unsigned long>(value)) {
//    }
//    if (holds_alternative<unsigned short>(value)) {
//    }
//    
//    if (holds_alternative<long>(value)) {
//    }
//    if (holds_alternative<long long>(value)) {
//        
//    }
//    
//    if (holds_alternative<short>(value)) {
//    }
//    if (holds_alternative<unsigned int>(value)) {
//        
//    }
//    if (holds_alternative<unsigned long long>(value)) {
//    }
//    if (holds_alternative<float>(value)) {}
//    if (holds_alternative<long double>(value)) {
//    }
//    if (holds_alternative<int>(value)) {
//    }
//    if (holds_alternative<char>(value)) {
//    }
//    if (holds_alternative<string>(value)) {
//    }
//    if (holds_alternative<bool>(value)) {
//        
//    }
//    if (holds_alternative<shared_ptr<JSObject>>(value)) {
//    }
//    if (holds_alternative< shared_ptr<Value>>(value)) {
//    }
//    if (holds_alternative<shared_ptr<JSClass>>(value)) {
//    }
//    if (holds_alternative< shared_ptr<JSArray>>(value)) {
//    }
//    if (holds_alternative<Value>(value)) {
//    }
    
    return toValue(value).boolean();
    
}

bool equals(const R& a, const R& b) {
    return std::visit([](auto&& lhs, auto&& rhs) -> bool {
        using L = std::decay_t<decltype(lhs)>;
        using Rhs = std::decay_t<decltype(rhs)>;

        // --- Nullish (undefined / null) ---
        if constexpr ((std::is_same_v<L, std::monostate> || std::is_same_v<L, std::nullptr_t>) ||
                      (std::is_same_v<Rhs, std::monostate> || std::is_same_v<Rhs, std::nullptr_t>)) {
            return std::is_same_v<L, Rhs>;
        }

        // --- Numbers (int, double, float, etc.) ---
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

inline void printValue(const R& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        std::cout << "nil";
    } else if (std::holds_alternative<double>(value)) {
        std::cout << std::get<double>(value);
    } else if (std::holds_alternative<unsigned long>(value)) {
        std::cout << std::get<unsigned long>(value);
    } else if (std::holds_alternative<unsigned short>(value)) {
        std::cout << std::get<unsigned short>(value);
    } else if (std::holds_alternative<int>(value)) {
        std::cout << std::get<int>(value);
    } else if (std::holds_alternative<size_t>(value)) {
        std::cout << std::get<size_t>(value);
    } else if (std::holds_alternative<std::string>(value)) {
        std::cout << std::get<std::string>(value);
    } else if (std::holds_alternative<bool>(value)) {
        std::cout << (std::get<bool>(value) ? "true" : "false");
    } else if (std::holds_alternative<shared_ptr<Value>>(value)) {

        if (get<shared_ptr<Value>>(value)->type == ValueType::ARRAY) {
            cout << get<shared_ptr<Value>>(value)->arrayValue->toString();
            return;
        }

        std::cout << (std::get<shared_ptr<Value>>(value))->toString();
    } else if (std::holds_alternative<Value>(value)) {
        
        if (get<Value>(value).type == ValueType::ARRAY) {
            cout << get<Value>(value).arrayValue->toString();
            return;
        }
        
//        NATIVE_FUNCTION,
//        FUNCTION,
//        METHOD

        
        std::cout << (std::get<Value>(value)).toString();
        
    } else if (std::holds_alternative<shared_ptr<JSArray>>(value)) {
        shared_ptr<JSArray> array = get<shared_ptr<JSArray>>(value);
        cout << array->toString();
    }
}

string read_file(const string& filename) {
    
    ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        exit(1);
    }
    
    ostringstream buffer;
    buffer << file.rdbuf();
    string source = buffer.str();
        
    file.close();
    
    return source;
    
}

#endif /* Utils_h */
