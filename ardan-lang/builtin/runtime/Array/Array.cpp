//
//  Array.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 27/10/2025.
//

#include "Array.hpp"
#include "../../Statements/Statements.hpp"

shared_ptr<JSObject> Array::construct() {
    
    auto arr = make_shared<JSArray>();
    
    arr->set_builtin_value("constructor", Value::native([this, arr](const std::vector<Value>& args) -> Value {
        
        arr->set("value", args[0]);
        
        return Value();

    }));
    
    return arr;
    
}

//| Form             | Behavior          | Returns     |
//| ---------------- | ----------------- | ----------- |
//| `Array()`        | empty array       | `[]`        |
//| `Array(3)`       | length 3 (holes)  | `[ , , , ]` |
//| `Array(1, 2, 3)` | normal array      | `[1, 2, 3]` |
//| `new Array()`    | same as `Array()` | `[]`        |

//| Expression           | What it does                                    | Notes                                     |
//| -------------------- | ----------------------------------------------- | ----------------------------------------- |
//| `new Array()`        | creates an **empty array**                      | same as `[]`                              |
//| `new Array(3)`       | creates an array of **length 3**, *empty slots* | `[ <3 empty items> ]`                     |
//| `new Array(1, 2, 3)` | creates an array with 3 elements                | `[1, 2, 3]`                               |
//| `Array()`            | behaves exactly the same as `new Array()`       | the `new` keyword is optional for `Array` |

Value Array::call(const std::vector<Value>& args) {
    // it will return a string
    
    auto arr = make_shared<JSArray>();

    if (args.size() == 0) {
        
        return Value::array(arr);
        
    } else if (args.size() == 1) {
        
        int arg_len = args[0].numberValue;
        
        for (int i = 0; i < arg_len; i++) {
            arr->push({ Value::str("") });
        }
        
    } else {
        
        for (auto arg : args) {
            arr->push({ arg });
        }

    }

    return Value::array(arr);
    
}
