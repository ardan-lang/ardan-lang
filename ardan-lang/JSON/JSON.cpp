//
//  JSON.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 11/09/2025.
//

#include "JSON.hpp"

// Simple function to trim whitespace and quotes
std::string JSON::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\"");
    size_t end   = s.find_last_not_of(" \t\n\r\"");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Very basic JSON object reader (flat key-value pairs only)
std::map<std::string, std::string> JSON::readJson(const std::string& filename) {
    std::ifstream file(filename);
    std::map<std::string, std::string> result;

    if (!file.is_open()) {
        std::cerr << "Could not open " << filename << "\n";
        return result;
    }

    std::string line, json;
    while (std::getline(file, line)) {
        json += line;
    }

    // Remove braces { }
    json.erase(std::remove(json.begin(), json.end(), '{'), json.end());
    json.erase(std::remove(json.begin(), json.end(), '}'), json.end());

    std::stringstream ss(json);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        size_t colon = pair.find(':');
        if (colon != std::string::npos) {
            std::string key = trim(pair.substr(0, colon));
            std::string value = trim(pair.substr(colon + 1));
            result[key] = value;
        }
    }

    return result;
}

//int main() {
//    auto data = readJson("main.json");
//
//    if (data.empty()) {
//        std::cerr << "No data found in main.json\n";
//        return 1;
//    }
//
//    // Print everything
//    std::cout << "main.json contents:\n";
//    for (const auto& [key, value] : data) {
//        std::cout << key << " : " << value << "\n";
//    }
//
//    // Example: access specific keys
//    if (data.count("name")) {
//        std::cout << "Project name: " << data["name"] << "\n";
//    }
//    if (data.count("version")) {
//        std::cout << "Version: " << data["version"] << "\n";
//    }
//
//    return 0;
//}
