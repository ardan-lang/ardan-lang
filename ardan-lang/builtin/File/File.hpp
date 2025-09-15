//
//  File.hpp
//  tau-programming-lang
//
//  Created by Chidume Nnamdi on 02/09/2025.
//

#ifndef File_hpp
#define File_hpp

#include <stdio.h>
#include <fstream>
#include <sstream>

#include "../../Interpreter/R.hpp"

class File : public JSObject {
//    const fs = require('fs');

    // Reading a file asynchronously
//    fs.readFile('example.txt', 'utf8', (err, data) => {
//        if (err) throw err;
//        console.log('File contents:', data);
//    });

    // Writing to a file asynchronously
//    fs.writeFile('output.txt', 'Hello, Node.js!', (err) => {
//        if (err) throw err;
//        console.log('File written successfully.');
//    });
    
public:
    File() {
        
        set_builtin_value("readFileSync", Value::native([](const std::vector<Value>& args) {

            if (args.size() < 1) {
                throw std::runtime_error("readFile expects 1 argument (path)");
            }
            
            // std::string path = args[0].toString();
            
            std::string path = std::filesystem::absolute(args[0].toString()).string();
            Value callback = args[1];

            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file) {
                
                vector<Value> cbArgs = {
                    Value("Could not open file: " + path)
                };
                
                callback.functionValue(cbArgs);
                
                return Value::nullVal();
                
                // throw std::runtime_error("Could not open file: " + path);
            }
            
            std::ostringstream ss;
            ss << file.rdbuf();
                        
            vector<Value> cbArgs = {
                Value(),
                Value(ss.str())
            };
            
            callback.functionValue(cbArgs);
            return Value::nullVal();
            
        }));
        
        set_builtin_value("writeFileSync", Value::native([](const std::vector<Value>& args) {
            
            if (args.size() < 2) {
                throw std::runtime_error("writeFile expects 2 arguments (path, data)");
            }
            // std::string path = args[0].toString();
            std::string path = std::filesystem::absolute(args[0].toString()).string();

            std::string data = args[1].toString();
            Value callback = args[1];

            std::ofstream file(path, std::ios::out | std::ios::binary);
            if (!file) {
                
                vector<Value> cbArgs = {
                    Value("Could not write file: " + path),
                };

                callback.functionValue(cbArgs);
                
                return Value::nullVal();
                
                // throw std::runtime_error("Could not write file: " + path);
                
            }
            
            file << data;
            file.close();
            
            vector<Value> cbArgs = {};

            callback.functionValue(cbArgs);

            return Value::nullVal();
            
        }));

    }
};

#endif /* File_hpp */
