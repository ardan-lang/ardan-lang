//
//  WriteArdarFile.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 08/10/2025.
//

#ifndef WriteArdarFile_hpp
#define WriteArdarFile_hpp

#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "../VM/VM.hpp"
#include "../CodeGenerator.hpp"
#include "../VM/Module.hpp"

#include "../Turbo/TurboVM.hpp"
#include "../Turbo/TurboCodeGenerator.hpp"
#include "../Turbo/TurboModule.hpp"

class WriteArdarFile {
public:
    WriteArdarFile(const std::string& filename,
                   const Module* module_,
                   uint32_t entryChunkIndex,
                   uint32_t version = 1);
    
    WriteArdarFile(const std::string& filename,
                   const TurboModule* turboModule,
                   uint32_t entryChunkIndex,
                   uint32_t version = 2);
    
    ~WriteArdarFile();

    void writing();
    void writingTurbo(const TurboModule* turboModule);

private:
    void writeMagic(const char* magic);
    void writeU32(uint32_t v);
    void writeU8(uint8_t v);
    void writeDouble(double v);
    void writeString(const std::string& s);
    void writeBytes(const std::vector<uint8_t>& data);

    std::ofstream out;
    const Module* module_;
    const TurboModule* turboModule;
    uint32_t version;
    uint32_t entryChunkIndex;
};

#endif /* WriteArdarFile_hpp */
