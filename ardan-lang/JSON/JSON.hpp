//
//  JSON.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/09/2025.
//

#ifndef JSON_hpp
#define JSON_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>

class JSON {
public:
    std::string trim(const std::string& s);
    std::map<std::string, std::string> readJson(const std::string& filename);

};

#endif /* JSON_hpp */
