#include "BuildingManager.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm> // For std::reverse
#include <cmath>     // For std::cos in projection

// --- Third-party header-only libraries ---
// These are found by CMake via add_subdirectory(libs/...)
#include <nlohmann/json.hpp>

#include <mapbox/earcut.hpp>
#include <Pipeline.h>

#include <ShaderLoader.h>

// --- Helper Structures and Constants for Projection ---

// A simple 2D point structure required by the earcut triangulation library
using Point = std::array<double, 2>;

// Constants for the WGS84 ellipsoid (Earth's shape) used in projection
constexpr double EARTH_RADIUS_METERS = 6378137.0;
constexpr double PI = 3.14159265358979323846;

// Represents a projected coordinate in your local meter-based system
struct LocalCoords {
    float x;
    float z;
};

/**
 * @brief Projects a geographic coordinate to a local Cartesian coordinate.
 *
 * Uses an Equirectangular projection, which is a good approximation for small areas.
 * @param lon Longitude of the point to project.
 * @param lat Latitude of the point to project.
 * @param originLon Longitude of the local system's origin (0,0).
 * @param originLat Latitude of the local system's origin (0,0).
 * @param metersPerDegLon Pre-calculated scale factor for longitude.
 * @param metersPerDegLat Pre-calculated scale factor for latitude.
 * @return The projected {x, z} coordinates in meters.
 */
LocalCoords projectToLocal(double lon, double lat, double originLon, double originLat, double metersPerDegLon, double metersPerDegLat) {
    double deltaLon = lon - originLon;
    double deltaLat = lat - originLat;

    // Note: Latitude is mapped to the Z-axis. Depending on your camera setup
    // (e.g., if Z is "into the screen"), you might need to use -deltaLat.
    return {
        static_cast<float>(deltaLon * metersPerDegLon),
        static_cast<float>(deltaLat * metersPerDegLat)
    };
}


// --- Class Implementation ---

void BuildingManager::initialize(wgpu::Device const& device) {
    m_device = device;
}

void BuildingManager::loadBuildings(const std::string& geojsonPath, double originLon, double originLat, float defaultBuildingHeight) {
    if (!m_device) {
        throw std::runtime_error("BuildingManager has not been initialized. Call initialize() first.");
    }

    // --- 1. SETUP COORDINATE PROJECTION ---
    // Calculate the crucial scaling factors: how many meters one degree represents at our origin's latitude.
    double latRad = originLat * PI / 180.0;
    const double metersPerDegreeLon = (PI * EARTH_RADIUS_METERS * std::cos(latRad)) / (180.0 );
    const double metersPerDegreeLat = (2.0 * PI * EARTH_RADIUS_METERS) / (360.0 );

    std::cout << "Building projection initialized:" << std::endl;
    std::cout << " - Origin: " << originLon << ", " << originLat << std::endl;
    std::cout << " - Meters/Deg Lon: " << metersPerDegreeLon << std::endl;
    std::cout << " - Meters/Deg Lat: " << metersPerDegreeLat << std::endl;

    // --- 2. PARSE GEOJSON AND PROJECT FOOTPRINTS ---
    std::ifstream geojsonFile(geojsonPath);
    if (!geojsonFile.is_open()) {
        throw std::runtime_error("Failed to open GeoJSON file: " + geojsonPath);
    }
    
    nlohmann::json data = nlohmann::json::parse(geojsonFile);
    std::vector<std::vector<Point>> buildingFootprints; // Stores the *projected* footprints in meters

    if (data.contains("features")) {
        // const int maxBuildingsToProcess = INT_MAX;
        int buildingsProcessed = 0;
        for (const auto& feature : data["features"]) {
            const auto& geometry = feature["geometry"];
            if (geometry["type"] == "Polygon") {
                std::vector<Point> projectedFootprint;
                //     if (buildingsProcessed >= maxBuildingsToProcess) {
                //     break; // Exit the loop
                // }
                buildingsProcessed++;
                std::cout << "Processing a building footprint..." << std::endl;
                // For a polygon, the first array of coordinates is the outer ring. We ignore inner holes for simplicity.
                for (const auto& coord : geometry["coordinates"][0]) {
                    double lon = coord[0].get<double>();
                    double lat = coord[1].get<double>();

                    // Project the geographic coordinate into our local meter-based system
                    LocalCoords local = projectToLocal(lon, lat, originLon, originLat, metersPerDegreeLon, metersPerDegreeLat);
                    projectedFootprint.push_back({ local.x, local.z });
                    std::cout << "  - Projected Point: [" << local.x << ", " << local.z << "]" << std::endl;
                }

                // GeoJSON polygons often repeat the first point at the end; remove it for triangulation.
                if (projectedFootprint.size() > 1 && projectedFootprint.front() == projectedFootprint.back()) {
                    projectedFootprint.pop_back();
                }
                buildingFootprints.push_back(projectedFootprint);
            }
        }
    }
    std::cout << "Parsed and projected " << buildingFootprints.size() << " building footprints." << std::endl;
    if (buildingFootprints.empty()) {
        std::cerr << "Warning: No building footprints were loaded from GeoJSON." << std::endl;
        return;
    }

    // --- 3. GENERATE 3D MESH DATA (VERTICES AND INDICES) ---
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    const int maxBuildingsToProcess = 5;
    int buildingsProcessed = 0;

    // float w = 10.0f; // Width
    // float h = 20.0f; // Height

    // // Base Vertices (Y = 0)
    // vertices.push_back({{ 0.0f, 0.0f, 0.0f }}); // 0: Bottom-Left
    // vertices.push_back({{    w, 0.0f, 0.0f }}); // 1: Bottom-Right
    // vertices.push_back({{    w, 0.0f,    w }}); // 2: Top-Right
    // vertices.push_back({{ 0.0f, 0.0f,    w }}); // 3: Top-Left

    // // Top Vertices (Y = Height)
    // vertices.push_back({{ 0.0f,    h, 0.0f }}); // 4: Bottom-Left Top
    // vertices.push_back({{    w,    h, 0.0f }}); // 5: Bottom-Right Top
    // vertices.push_back({{    w,    h,    w }}); // 6: Top-Right Top
    // vertices.push_back({{ 0.0f,    h,    w }}); // 7: Top-Left Top

    // // 2. Define Indices (12 Triangles total)

    // // -- Wall 1 (Front) --
    // indices.push_back(0); indices.push_back(1); indices.push_back(5);
    // indices.push_back(5); indices.push_back(4); indices.push_back(0);

    // // -- Wall 2 (Right) --
    // indices.push_back(1); indices.push_back(2); indices.push_back(6);
    // indices.push_back(6); indices.push_back(5); indices.push_back(1);

    // // -- Wall 3 (Back) --
    // indices.push_back(2); indices.push_back(3); indices.push_back(7);
    // indices.push_back(7); indices.push_back(6); indices.push_back(2);

    // // -- Wall 4 (Left) --
    // indices.push_back(3); indices.push_back(0); indices.push_back(4);
    // indices.push_back(4); indices.push_back(7); indices.push_back(3);

    // // -- Roof (Top) --
    // indices.push_back(4); indices.push_back(5); indices.push_back(7);
    // indices.push_back(5); indices.push_back(6); indices.push_back(7);

    // // -- Floor (Optional, usually hidden) --
    // indices.push_back(0); indices.push_back(3); indices.push_back(1);
    // indices.push_back(3); indices.push_back(2); indices.push_back(1);

    // std::cout << "Generated Dummy Static Building." << std::endl;


    for (const auto& footprint : buildingFootprints) {
    //      if (buildingsProcessed >= maxBuildingsToProcess) {
    //     break; // Exit the loop
    // }
    buildingsProcessed++;
        if (footprint.size() < 3) continue; // Skip invalid polygons

        uint32_t baseIndex = static_cast<uint32_t>(vertices.size());
        uint32_t numFootprintPoints = static_cast<uint32_t>(footprint.size());

        // Create bottom and top vertices from the projected footprint
        for (const auto& p : footprint) {
            vertices.push_back({{(float)p[0], 0.0f, (float)p[1]}}); // Bottom vertex (Y=0)
        }
        for (const auto& p : footprint) {
            vertices.push_back({{(float)p[0], defaultBuildingHeight, (float)p[1]}}); // Top vertex
        }

        // Generate indices for the side walls by creating two triangles per wall segment
        for (uint32_t i = 0; i < numFootprintPoints; ++i) {
            uint32_t next_i = (i + 1) % numFootprintPoints; // Wrap around for the last wall

            uint32_t bl = baseIndex + i;
            uint32_t br = baseIndex + next_i;
            uint32_t tl = bl + numFootprintPoints;
            uint32_t tr = br + numFootprintPoints;

            indices.push_back(bl); indices.push_back(br); indices.push_back(tl);
            indices.push_back(tl); indices.push_back(br); indices.push_back(tr);
        }

        // Triangulate the top and bottom faces using the earcut library
        std::vector<std::vector<Point>> polygon = { footprint };
        std::vector<uint32_t> faceIndices = mapbox::earcut<uint32_t>(polygon);

        // Add indices for the bottom face
        for (uint32_t idx : faceIndices) {
            indices.push_back(baseIndex + idx);
        }

        // Add indices for the top face (in reverse order for correct outward-facing normals)
        std::reverse(faceIndices.begin(), faceIndices.end());
        for (uint32_t idx : faceIndices) {
            indices.push_back(baseIndex + numFootprintPoints + idx);
        }
    }

    m_indexCount = static_cast<uint32_t>(indices.size());
    if (m_indexCount == 0) {
        std::cerr << "Warning: Mesh generation resulted in 0 indices." << std::endl;
        return;
    }

    // Debug: print the generated vertex positions (first 3 floats of each Vertex assumed to be XYZ)
    // std::cout << "Dumping vertices (" << vertices.size() << "):" << std::endl;
    // for (size_t i = 0; i < vertices.size(); ++i) {
    //     const float* pos = reinterpret_cast<const float*>(&vertices[i]);
    //     std::cout << "  [" << i << "] " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
    // }

    // --- 4. CREATE AND UPLOAD GPU BUFFERS ---
    wgpu::BufferDescriptor vertexDesc = {};
    vertexDesc.size = vertices.size() * sizeof(Vertex);
    vertexDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    m_vertexBuffer = m_device.CreateBuffer(&vertexDesc);
    m_device.GetQueue().WriteBuffer(m_vertexBuffer, 0, vertices.data(), vertexDesc.size);

    wgpu::BufferDescriptor indexDesc = {};
    indexDesc.size = indices.size() * sizeof(uint32_t);
    indexDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
    m_indexBuffer = m_device.CreateBuffer(&indexDesc);
    m_device.GetQueue().WriteBuffer(m_indexBuffer, 0, indices.data(), indexDesc.size);

    m_isReady = true;
    std::cout << "Successfully uploaded building meshes to GPU." << std::endl;
    std::cout << " - Vertex count: " << vertices.size() << std::endl;
    std::cout << " - Index count: " << m_indexCount << std::endl;
}


wgpu::RenderPipeline BuildingManager::createBuildingPipeline(const wgpu::TextureFormat& format, const wgpu::BindGroupLayout& cameraBindGroupLayout) {
    std::string shader="shaders/buildings.wgsl";
    std::string shaderCode;
        try {
            shaderCode = loadShaderSource(shader);
        } catch (const std::runtime_error& e) {
            std::cerr << "Fatal Error: " << e.what() << std::endl;
            // Handle error, maybe exit or throw an exception.
            exit(1);
        }

    // Create the shader module
    wgpu::ShaderSourceWGSL wgsl{{.code = shaderCode.c_str()}};
    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain = &wgsl};
    wgpu::ShaderModule shaderModule = m_device.CreateShaderModule(&shaderModuleDescriptor);

    // Create the vertex state
    VertexPipelineLayoutData layoutData;
    wgpu::VertexState vertexState = createVertexPipelineLayout(shaderModule, layoutData);
    

    wgpu::ColorTargetState colorTargetState{.format = format};
    
     wgpu::DepthStencilState depthStencilState = {
    .format = wgpu::TextureFormat::Depth24Plus, // Must match the texture created in main loop
    .depthWriteEnabled = true,                  // "Write my distance to the buffer"
    .depthCompare = wgpu::CompareFunction::Less,// "Only draw me if I am closer than what's already there"
    };

    wgpu::FragmentState fragmentState = {
        .module = shaderModule,
        .entryPoint = "fragmentMain",
        .targetCount = 1,
        .targets = &colorTargetState
    };

    
    wgpu::PrimitiveState primitiveState = {
        .topology = wgpu::PrimitiveTopology::TriangleList,
        .stripIndexFormat = wgpu::IndexFormat::Undefined,
        .frontFace = wgpu::FrontFace::CW,
        .cullMode = wgpu::CullMode::None
    };
    
    wgpu::BindGroupLayoutEntry bindGroupEntry = {
        .binding = 0,
        .visibility = wgpu::ShaderStage::Vertex
    };
    bindGroupEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    
    wgpu::BindGroupLayoutDescriptor bindGroupDesc = {
        .entryCount = 1,
        .entries = &bindGroupEntry
    };

    wgpu::BindGroupLayout bindGroupLayout = cameraBindGroupLayout;

    wgpu::PipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = &bindGroupLayout;
    
    wgpu::PipelineLayout pipelineLayout= m_device.CreatePipelineLayout(&layoutDesc);
    
    
    wgpu::RenderPipelineDescriptor pipelineDescriptor = {
        .layout = pipelineLayout,
        .vertex = vertexState,
        .primitive = primitiveState,
        .depthStencil = &depthStencilState,
        .fragment = &fragmentState
    };
    

    // Create the render pipeline
    return m_device.CreateRenderPipeline(&pipelineDescriptor);
}



wgpu::VertexState BuildingManager::createVertexPipelineLayout(
    const wgpu::ShaderModule& shaderModule,
    VertexPipelineLayoutData& layoutData
) {
    // --- Mesh vertex buffer (location 0) ---
    layoutData.vertexAttributes[0] = {
        .format = wgpu::VertexFormat::Float32x3, // position
        .offset = 0,
        .shaderLocation = 0
    };

    // FIX: Reordered initializers to match declaration order
    layoutData.vertexBufferLayout = {
        .stepMode = wgpu::VertexStepMode::Vertex,
        .arrayStride = sizeof(Vertex), 
        .attributeCount = 1,
        .attributes = layoutData.vertexAttributes
    };


    // --- VertexState with two buffers ---
    static wgpu::VertexBufferLayout buffers[1] = { layoutData.vertexBufferLayout };

    wgpu::VertexState vertexState{
        .module = shaderModule,
        .entryPoint = "vertexMain",
        .bufferCount = 1,
        .buffers = buffers
    };

    return vertexState;
}