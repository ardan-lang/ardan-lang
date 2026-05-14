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

#include "engines/Cascade/VM/VM.hpp"
#include "engines/Cascade/CodeGenerator.hpp"
#include "engines/Cascade/VM/Module.hpp"

#include "engines/Nova/TurboVM.hpp"
#include "engines/Nova/TurboCodeGenerator.hpp"
#include "engines/Nova/TurboModule.hpp"

class ArdarFileReader {
public:
    ArdarFileReader(const std::string& filename);
    ~ArdarFileReader();

    std::unique_ptr<Module> readModule();
    std::unique_ptr<TurboModule> readTurboModule(const std::string& filename);

private:
    std::ifstream in;

    void readMagic(const char* expected);
    uint32_t readU32();
    uint8_t readU8();
    double readDouble();
    std::string readString();
    std::vector<uint8_t> readBytes(size_t n);

    std::string readString(std::istream& in);
    double readDouble(std::istream& in);
    uint8_t readU8(std::istream& in);
    uint32_t readU32(std::istream& in);

};

#endif /* ArdarFileReader_hpp */
