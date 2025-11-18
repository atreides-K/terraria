#pragma once

#include <webgpu/webgpu_cpp.h>
#include <string>
#include <vector>

/**
 * @class BuildingManager
 * @brief Manages loading, processing, and rendering of 3D building data.
 *
 * This class handles:
 * 1. Reading building footprint data from a GeoJSON file.
 * 2. Projecting geographic coordinates (lon/lat) to a local metric coordinate system.
 * 3. Generating 3D meshes by extruding the footprints.
 * 4. Uploading the resulting vertex and index data to GPU buffers for rendering.
 */
class BuildingManager {
public:
    BuildingManager() = default;

    /**
     * @struct Vertex
     * @brief Defines the data layout for a single vertex.
     *
     * This must match the vertex buffer layout expected by the render pipeline.
     */
    struct Vertex {
        float position[3]; // X, Y, Z
        // Future additions could include:
        // float normal[3];
        // float uv[2];
    };

    /**
     * @brief Initializes the manager with a WebGPU device.
     * @param device The active wgpu::Device. Must be called before loadBuildings.
     */
    void initialize(wgpu::Device const& device);

    /**
     * @brief Loads building data from a GeoJSON file, generates meshes, and uploads to the GPU.
     * @param geojsonPath Path to the .geojson file.
     * @param originLon The longitude of the origin (0,0) of your local coordinate system (e.g., your DEM's corner).
     * @param originLat The latitude of the origin (0,0) of your local coordinate system.
     * @param defaultBuildingHeight The height in meters to extrude the building footprints.
     */
    void loadBuildings(
        const std::string& geojsonPath,
        double originLon,
        double originLat,
        float defaultBuildingHeight = 15.0f
    );

    // --- Accessors for the Render Loop ---

    wgpu::Buffer getVertexBuffer() const { return m_vertexBuffer; }
    wgpu::Buffer getIndexBuffer() const { return m_indexBuffer; }
    uint32_t getIndexCount() const { return m_indexCount; }
    bool isReady() const { return m_isReady; }

private:
    wgpu::Device m_device;
    wgpu::Buffer m_vertexBuffer;
    wgpu::Buffer m_indexBuffer;
    uint32_t m_indexCount = 0;
    bool m_isReady = false;
};