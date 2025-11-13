#include "demLoader.h"
#include <cstring> // For strerror
#include <cerrno>  // For errno

DEMLoader::DEMLoader(const std::string& rawFile, const std::string& hdrFile) {
    parseENVIHeader(hdrFile);
    loadENVI(rawFile); // Now loads data into the m_heightmap member
}

// This function now modifies the class member 'm_heightmap' directly
void DEMLoader::loadENVI(const std::string& rawFile) {
    m_heightmap.width = getWidth();
    m_heightmap.height = getHeight();
    std::cout << "Loading " << rawFile << " (" << m_heightmap.width << "x" << m_heightmap.height << ")" << std::endl;
    m_heightmap.data.resize(m_heightmap.width * m_heightmap.height);

    std::ifstream file(rawFile, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open RAW file: " + rawFile);
    }

    file.read(reinterpret_cast<char*>(m_heightmap.data.data()), m_heightmap.width * m_heightmap.height * sizeof(float));
    file.close();
}

void DEMLoader::parseENVIHeader(const std::string& hdrFile) {
    // ... your parsing code is perfect, no changes needed ...
    std::ifstream file(hdrFile);
    if (!file.is_open()) {
        int errnum = errno;
        const char* errstr = errnum ? std::strerror(errnum) : "unknown error";
        std::cerr << "DEMLoader::parseENVIHeader - Failed to open .hdr file: '"
                << hdrFile << "' (errno=" << errnum << " : " << errstr << ")\n";
        throw std::runtime_error(std::string("Cannot open .hdr file: ") + hdrFile + " (" + errstr + ")");
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';') continue;
        auto eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            auto trim = [](std::string s) {
                s.erase(0, s.find_first_not_of(" \t"));
                s.erase(s.find_last_not_of(" \t\r\n") + 1);
                return s;
            };
            DEMMetadata[trim(key)] = trim(value);
        }
    }
    width=std::stoi(DEMMetadata["samples"]);
    height=std::stoi(DEMMetadata["lines"]);
}

int DEMLoader::getWidth() const { return width; }
int DEMLoader::getHeight() const { return height; }

// The new getter
const Heightmap& DEMLoader::getHeightmap() const { return m_heightmap; }