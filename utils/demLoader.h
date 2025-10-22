#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <unordered_map>

struct Heightmap {
    int width;
    int height;
    std::vector<float> data;
};


class DEMLoader {
    public:
        DEMLoader(const std::string& rawFile,const std::string& hdrFile);

        std::vector<std::vector<float>> getElevationData() const;

        Heightmap loadENVI(const std::string& rawFile);

        void parseENVIHeader(const std::string& hdrFile);

        int getWidth() const;
        
        int getHeight() const;
    private:
        std::vector<std::vector<float>> elevationData;
        std::unordered_map<std::string, std::string> DEMMetadata;
        int width;
        int height;
};