//
//  ArdarFileReader.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 08/10/2025.
//

#ifndef ArdarFileReader_hpp
#define ArdarFileReader_hpp

#include <stdio.h>

#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>

#include "../VM/Module.hpp"
#include "../VM/Chunk.hpp"
#include "../VM/VMv2.hpp"
#include "../CodeGeneratorV2.hpp"
#include "../VM/Module.hpp"

class ArdarFileReader {
public:
    ArdarFileReader(const std::string& filename);
    ~ArdarFileReader();

    std::unique_ptr<Module> readModule();

private:
    std::ifstream in;

    void readMagic(const char* expected);
    uint32_t readU32();
    uint8_t readU8();
    double readDouble();
    std::string readString();
    std::vector<uint8_t> readBytes(size_t n);

};

#endif /* ArdarFileReader_hpp */
