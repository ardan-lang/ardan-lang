//
//  ArdarFileReader.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 08/10/2025.
//

#include "ArdarFileReader.hpp"

ArdarFileReader::ArdarFileReader(const std::string& filename)
    : in(filename, std::ios::binary)
{
    if (!in.is_open()) throw std::runtime_error("Could not open ardar file");
}

ArdarFileReader::~ArdarFileReader() {
    if (in.is_open()) in.close();
}

std::unique_ptr<Module> ArdarFileReader::readModule() {
    
    readMagic("ARDAR");
    
    uint32_t version = readU32();
    
    uint32_t entryChunkIndex = readU32();

    auto module_ = make_unique<Module>();
    module_->version = version;
    module_->entryChunkIndex = entryChunkIndex;

    // Chunks
    uint32_t numChunks = readU32();
    for (uint32_t i = 0; i < numChunks; ++i) {
        auto chunk = std::make_unique<Chunk>();
        uint32_t codeSize = readU32();
        chunk->code = readBytes(codeSize);

        uint32_t constantsSize = readU32();
        for (uint32_t ci = 0; ci < constantsSize; ++ci) {
            Value v;
            v.type = static_cast<ValueType>(readU8());
            switch (v.type) {
                case ValueType::NUMBER:
                    v.numberValue = readDouble();
                    break;
                case ValueType::STRING:
                    v.stringValue = readString();
                    break;
                case ValueType::FUNCTION_REF: {
                    auto fn = make_shared<FunctionObject>();
                    fn->chunkIndex = readU32();
                    fn->arity = readU32();
                    fn->name = readString();
                    fn->upvalues_size = readU32();
                    v.fnRef = fn;
                    break;
                }
                default:
                    // handle others as appropriate
                    break;
            }
            chunk->constants.push_back(v);
        }

        chunk->arity = readU32();
        chunk->name = readString();
        chunk->maxLocals = readU32();
        module_->chunks.push_back(std::move(chunk));
    }

    // Constants
    uint32_t numConstants = readU32();
    for (uint32_t i = 0; i < numConstants; ++i) {
        Value v;
        v.type = static_cast<ValueType>(readU8());
        switch (v.type) {
            case ValueType::NUMBER:
                v.numberValue = readDouble();
                break;
            case ValueType::STRING:
                v.stringValue = readString();
                break;
            case ValueType::FUNCTION_REF: {
                auto fn = make_shared<FunctionObject>();
                fn->chunkIndex = readU32();
                fn->arity = readU32();
                fn->name = readString();
                fn->upvalues_size = readU32();
                v.fnRef = fn;
                break;
            }
            default:
                // handle others as appropriate
                break;
        }
        module_->constants.push_back(v);
    }

    return module_;
}


std::unique_ptr<TurboModule> ArdarFileReader::readTurboModule(const std::string& filename) {

    // Read and check magic
    char magic[11];
    in.read(magic, 11);
    if (std::string(magic, 11) != "ARDAR-TURBO")
        throw std::runtime_error("File is not a valid TURBO ARDAR file");

    auto module_ = std::make_unique<TurboModule>();
    module_->version = readU32(in);
    module_->entryChunkIndex = readU32(in);

    // Chunks
    uint32_t numChunks = readU32(in);
    for (uint32_t i = 0; i < numChunks; ++i) {
        auto chunk = std::make_shared<TurboChunk>();

        // Code
        uint32_t codeSize = readU32(in);
        chunk->code.reserve(codeSize);
        for (uint32_t j = 0; j < codeSize; ++j) {
            TurboOpCode op = static_cast<TurboOpCode>(readU8(in));
            uint8_t a = readU8(in);
            uint8_t b = readU8(in);
            uint8_t c = readU8(in);
            chunk->code.push_back({op, a, b, c});
        }

        // Constants
        uint32_t numConsts = readU32(in);
        for (uint32_t k = 0; k < numConsts; ++k) {
            Value val;
            val.type = static_cast<ValueType>(readU8(in));
            switch (val.type) {
                case ValueType::NUMBER:
                    val.numberValue = readDouble(in);
                    break;
                case ValueType::STRING:
                    val.stringValue = readString(in);
                    break;
                case ValueType::FUNCTION_REF: {
                    val.fnRef = std::make_shared<FunctionObject>();
                    val.fnRef->chunkIndex = readU32(in);
                    val.fnRef->arity = readU32(in);
                    val.fnRef->name = readString(in);
                    val.fnRef->upvalues_size = readU32(in);
                    break;
                }
                default:
                    break;
            }
            chunk->constants.push_back(val);
        }

        chunk->arity = readU32(in);
        chunk->name = readString(in);
        chunk->maxLocals = readU32(in);

        module_->chunks.push_back(chunk);
    }

    // Global constants
    uint32_t numModuleConsts = readU32(in);
    for (uint32_t k = 0; k < numModuleConsts; ++k) {
        Value val;
        val.type = static_cast<ValueType>(readU8(in));
        switch (val.type) {
            case ValueType::NUMBER:
                val.numberValue = readDouble(in);
                break;
            case ValueType::STRING:
                val.stringValue = readString(in);
                break;
            case ValueType::FUNCTION_REF: {
                val.fnRef = make_shared<FunctionObject>();
                val.fnRef->chunkIndex = readU32(in);
                val.fnRef->arity = readU32(in);
                val.fnRef->name = readString(in);
                val.fnRef->upvalues_size = readU32(in);
                break;
            }
            default:
                break;
        }
        module_->constants.push_back(val);
    }
    return module_;
}

uint32_t ArdarFileReader::readU32(std::istream& in) {
    uint32_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(uint32_t));
    return v;
}

uint8_t ArdarFileReader::readU8(std::istream& in) {
    uint8_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(uint8_t));
    return v;
}

double ArdarFileReader::readDouble(std::istream& in) {
    double v;
    in.read(reinterpret_cast<char*>(&v), sizeof(double));
    return v;
}

std::string ArdarFileReader::readString(std::istream& in) {
    uint32_t len = readU32(in);
    std::string s(len, '\0');
    if(len) in.read(&s[0], len);
    return s;
}

void ArdarFileReader::readMagic(const char* expected) {
    char magic[6] = {0}; // 5 bytes + null-terminator
    in.read(magic, 5);
    if (std::strncmp(magic, expected, 5) != 0) {
        throw std::runtime_error("Bad magic number: not an .adar file");
    }
}

uint32_t ArdarFileReader::readU32() {
    uint32_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(uint32_t));
    return v;
}

uint8_t ArdarFileReader::readU8() {
    uint8_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(uint8_t));
    return v;
}

double ArdarFileReader::readDouble() {
    double d;
    in.read(reinterpret_cast<char*>(&d), sizeof(double));
    return d;
}

std::string ArdarFileReader::readString() {
    uint32_t len = readU32();
    std::string s(len, '\0');
    if (len > 0)
        in.read(&s[0], len);
    return s;
}

std::vector<uint8_t> ArdarFileReader::readBytes(size_t n) {
    std::vector<uint8_t> data(n);
    if (n > 0)
        in.read(reinterpret_cast<char*>(data.data()), n);
    return data;
}
