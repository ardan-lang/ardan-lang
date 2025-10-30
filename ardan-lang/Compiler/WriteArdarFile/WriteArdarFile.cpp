//
//  WriteArdarFile.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 08/10/2025.
//

#include "WriteArdarFile.hpp"

WriteArdarFile::WriteArdarFile(const std::string& filename,
                               const Module* module_,
                               uint32_t entryChunkIndex,
                               uint32_t version)
: out(filename, std::ios::binary), module_(module_), version(version), entryChunkIndex(entryChunkIndex)
{
    if (!out.is_open()) {
            throw std::runtime_error("Failed to open file for writing");
        }
}

WriteArdarFile::WriteArdarFile(const std::string& filename,
                               const TurboModule* module_,
                               uint32_t entryChunkIndex,
                               uint32_t version)
: out(filename, std::ios::binary), turboModule(module_), version(version), entryChunkIndex(entryChunkIndex)
{
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }
}

WriteArdarFile::~WriteArdarFile() {
    if (out.is_open()) out.close();
}

void WriteArdarFile::writing() {
    writeMagic("ARDAR");
    writeU32(version);
    
    writeU32(entryChunkIndex);

    // Write chunks
    writeU32(static_cast<uint32_t>(module_->chunks.size()));
    for (const auto& chunkPtr : module_->chunks) {
        const auto& chunk = *chunkPtr;

        writeU32(static_cast<uint32_t>(chunk.code.size()));
        writeBytes(chunk.code);
        
        writeU32(static_cast<uint32_t>(chunk.constants.size()));
        for (const auto& v : chunk.constants) {
            writeU8(static_cast<uint8_t>(v.type));
            switch (v.type) {
                case ValueType::NUMBER:
                    writeDouble(v.numberValue);
                    break;
                case ValueType::BOOLEAN:
                    writeDouble(v.boolValue);
                    break;
                case ValueType::STRING:
                    writeString(v.stringValue);
                    break;
                case ValueType::FUNCTION_REF:
                    writeU32(v.fnRef->chunkIndex);
                    writeU32(v.fnRef->arity);
                    writeString(v.fnRef->name);
                    writeU32(v.fnRef->upvalues_size);
                    
                    break;
                default:
                    break;
            }
        }
        
        writeU32(static_cast<uint32_t>(chunk.arity));
        writeString(chunk.name);
        writeU32(static_cast<uint32_t>(chunk.maxLocals));
    }

    // Write constants
    writeU32(static_cast<uint32_t>(module_->constants.size()));
    for (const auto& v : module_->constants) {
        writeU8(static_cast<uint8_t>(v.type));
        switch (v.type) {
        case ValueType::NUMBER:
            writeDouble(v.numberValue);
            break;
        case ValueType::STRING:
            writeString(v.stringValue);
            break;
        case ValueType::BOOLEAN:
            writeDouble(v.boolValue);
            break;
        case ValueType::FUNCTION_REF:
            writeU32(v.fnRef->chunkIndex);
            writeU32(v.fnRef->arity);
            writeString(v.fnRef->name);
            writeU32(v.fnRef->upvalues_size);

            break;
        default:
            break;
        }
    }
    out.flush();
}

void WriteArdarFile::writingTurbo(const TurboModule* turboModule) {
    writeMagic("ARDAR-TURBO"); // 11 bytes, or adjust to fit your needs (and change writeMagic if needed)
    
    writeU32(turboModule->version);
    writeU32(turboModule->entryChunkIndex);
    
    // Write chunks
    writeU32(static_cast<uint32_t>(turboModule->chunks.size()));
    for (const auto& chunkPtr : turboModule->chunks) {
        const auto& chunk = *chunkPtr;
        
        // Write code size (number of instructions)
        writeU32(static_cast<uint32_t>(chunk.code.size()));
        
        // Write each instruction as 4 bytes
        for (const auto& instr : chunk.code) {
            writeU8(static_cast<uint8_t>(instr.op));
            writeU8(instr.a);
            writeU8(instr.b);
            writeU8(instr.c);
        }
        
        // Write constants
        writeU32(static_cast<uint32_t>(chunk.constants.size()));
        for (const auto& v : chunk.constants) {
            writeU8(static_cast<uint8_t>(v.type));
            switch (v.type) {
                case ValueType::NUMBER:
                    writeDouble(v.numberValue);
                    break;
                case ValueType::BOOLEAN:
                    writeDouble(v.boolValue);
                    break;
                case ValueType::STRING:
                    writeString(v.stringValue);
                    break;
                case ValueType::FUNCTION_REF:
                    writeU32(v.fnRef->chunkIndex);
                    writeU32(v.fnRef->arity);
                    writeString(v.fnRef->name);
                    writeU32(v.fnRef->upvalues_size);
                    break;
                default:
                    break;
            }
        }
        
        // Metadata for function/script
        writeU32(chunk.arity);
        writeString(chunk.name);
        writeU32(chunk.maxLocals);
    }
    
    // Write global constants pool
    writeU32(static_cast<uint32_t>(turboModule->constants.size()));
    for (const auto& v : turboModule->constants) {
        writeU8(static_cast<uint8_t>(v.type));
        switch (v.type) {
            case ValueType::NUMBER:
                writeDouble(v.numberValue);
                break;
            case ValueType::STRING:
                writeString(v.stringValue);
                break;
            case ValueType::BOOLEAN:
                writeDouble(v.boolValue);
                break;
            case ValueType::FUNCTION_REF:
                writeU32(v.fnRef->chunkIndex);
                writeU32(v.fnRef->arity);
                writeString(v.fnRef->name);
                writeU32(v.fnRef->upvalues_size);
                break;
            default:
                break;
        }
    }
    out.flush();
}

void WriteArdarFile::writeMagic(const char* magic) {
    out.write(magic, /*5*/ strlen(magic) ); // "ARDAR" is 5 bytes
}

void WriteArdarFile::writeU32(uint32_t v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(uint32_t));
}

void WriteArdarFile::writeU8(uint8_t v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
}

void WriteArdarFile::writeDouble(double v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(double));
}

void WriteArdarFile::writeString(const std::string& s) {
    writeU32(static_cast<uint32_t>(s.size()));
    out.write(s.data(), s.size());
}

void WriteArdarFile::writeBytes(const std::vector<uint8_t>& data) {
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
}
