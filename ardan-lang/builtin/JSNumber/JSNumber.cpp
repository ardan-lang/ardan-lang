//
//  JSNumber.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "JSNumber.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> JSNumber::construct() {
    
    shared_ptr<JSObject> obj = make_shared<JSObject>();
    
    return obj;
    
}

//| JavaScript            | C++ equivalent                     |
//| --------------------- | ---------------------------------- |
//| `"ABC".toLowerCase()` | `std::transform` + `std::tolower`  |
//| Implicit autoboxing   | Explicit type check and conversion |
//| Returns new string    | Return new `Value` (immutable)     |

//| Concept                 | `"string"`        | `String`           | `new String("string")` |
//| ----------------------- | ----------------- | ------------------ | ---------------------- |
//| **Type**                | Primitive         | Function           | Object                 |
//| **typeof**              | `"string"`        | `"function"`       | `"object"`             |
//| **Stored as**           | immutable value   | global constructor | boxed wrapper          |
//| **When calling method** | temporarily boxed | –                  | directly accessible    |

//| Expression             | What it is                  | Type                                 | Notes                             |
//| ---------------------- | --------------------------- | ------------------------------------ | --------------------------------- |
//| `"string"`             | a **string literal**        | **primitive** (`typeof → "string"`)  | stored directly, not an object    |
//| `String`               | a **constructor function**  | **function** (`typeof → "function"`) | used to create or convert strings |
//| `new String("string")` | a **String object wrapper** | **object** (`typeof → "object"`)     | wraps a primitive string          |

Value JSNumber::call(const std::vector<Value>& args) {
    return Value();
}
