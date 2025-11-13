#pragma once

#include <webgpu/webgpu_cpp.h>
#include <string>
#include <vector>

class LODManager {
public:
    // Constructor needs the device to create GPU resources
    LODManager()=default;

    void operator()(wgpu::Device const& device);
    // Main function to load a series of LOD files into a texture array
    void loadLODs(const std::vector<std::string>& baseFilenames, wgpu::Buffer const& terrainUniformBuffer);

    // Getters for the main render loop to use
    wgpu::BindGroup getBindGroup() const { return m_bindGroup; }
    // need to send bglayout fr lod as well fr the pipeline layout 
    wgpu::BindGroupLayout getBindGroupLayout() const { return m_bindGroupLayout; }

private:
    wgpu::Device m_device;
    wgpu::Texture m_textureArray = nullptr;
    wgpu::Sampler m_sampler = nullptr;
    wgpu::BindGroup m_bindGroup = nullptr;
    wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
};