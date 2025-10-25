#include "ShaderLoader.h"

// Utility function to load a shader file into a std::string
std::string loadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::string msg = "Failed to open shader file: ";
        msg += filename;
        throw std::runtime_error(msg);
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the entire file stream into the stringstream
    return buffer.str();
}