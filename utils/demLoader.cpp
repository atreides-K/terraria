
#include<demLoader.h>
#include <cstring>



DEMLoader::DEMLoader(const std::string& rawFile,const std::string& hdrFile) {
    parseENVIHeader(hdrFile);
    Heightmap hm = loadENVI(rawFile);
}
Heightmap DEMLoader::loadENVI(const std::string& rawFile) {
    Heightmap hm;
    hm.width = getWidth();
    hm.height = getHeight();
    std::cout<<"Width: " << hm.width << ", Height: " << hm.height << std::endl;
    hm.data.resize(hm.width * hm.height);

    std::ifstream file(rawFile, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open RAW file: " + rawFile);
    }

    file.read(reinterpret_cast<char*>(hm.data.data()), hm.width * hm.height * sizeof(float));
    file.close();

    return hm;
}

void DEMLoader::parseENVIHeader(const std::string& hdrFile) {
    
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
        // skip comments and empty lines
        if (line.empty() || line[0] == ';') continue;

        auto eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            // clean up spaces
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

    return ;
}

int DEMLoader::getWidth() const {
    return width;
} 

int DEMLoader::getHeight() const {
    return height;
}
