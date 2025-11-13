#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

// A simple struct to hold the raw heightmap data on the CPU
struct Heightmap {
    int width = 0;
    int height = 0;
    std::vector<float> data;
};

class DEMLoader {
public:
    DEMLoader(const std::string& rawFile, const std::string& hdrFile);

    int getWidth() const;
    int getHeight() const;
    const Heightmap& getHeightmap() const; // <-- Public getter for the data

private:
    void parseENVIHeader(const std::string& hdrFile);
    void loadENVI(const std::string& rawFile); // <-- Changed to void

    int width = 0;
    int height = 0;
    std::map<std::string, std::string> DEMMetadata;
    Heightmap m_heightmap; // <-- Stores the loaded data
};