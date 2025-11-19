#include "LodManager.h"
#include "demLoader.h" // Your loader
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <BufferUtils.h>

void LODManager::operator()(wgpu::Device const& device) {   
    m_device = device;
}

void LODManager::loadLODs(const std::vector<std::string>& baseFilenames, wgpu::Buffer const& terrainUniformBuffer) {
    if (baseFilenames.empty()) {
        throw std::runtime_error("No LOD filenames provided.");
    }

    // --- 1. Load LOD 0 to get the master dimensions ---
    std::string lod0_raw = baseFilenames[0] + ".raw";
    std::string lod0_hdr = baseFilenames[0] + ".hdr";
    DEMLoader firstLoader(lod0_raw, lod0_hdr);
    const uint32_t masterWidth = firstLoader.getWidth();
    const uint32_t masterHeight = firstLoader.getHeight();
    std::cout << "Master LOD dimensions: " << masterWidth << "x" << masterHeight << std::endl;
    const uint32_t layerCount = baseFilenames.size();

    terrainUniforms uniformsData = {
        .heightScale = 1.0f, // Set as needed
        .terrainWidth = static_cast<float>(masterWidth)*30.0f,
        .terrainHeight = static_cast<float>(masterHeight)*30.0f    
    };
    // Update the terrain uniform buffer with the new data
    m_device.GetQueue().WriteBuffer(
        terrainUniformBuffer,
        0,
        &uniformsData,
        sizeof(terrainUniforms)
    );


    // --- 3. Create the GPU Texture Array resource ---
    wgpu::TextureDescriptor textureDesc = {
        usage: wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
        dimension: wgpu::TextureDimension::e2D,
        size: { masterWidth, masterHeight, layerCount },
        format: wgpu::TextureFormat::R32Float, // CRITICAL for height data
    };
    m_textureArray = m_device.CreateTexture(&textureDesc);

    // --- 3. Loop through all LODs, load, and upload to the correct slice ---
    for (uint32_t i = 0; i < layerCount; ++i) {
        std::string raw_path = baseFilenames[i] + ".raw";
        std::string hdr_path = baseFilenames[i] + ".hdr";
        
        // Load the heightmap data using your loader
        DEMLoader loader(raw_path, hdr_path);
        const Heightmap& hm = loader.getHeightmap();

        // Sanity check: ensure all upscaled textures have the same dimensions
        if (hm.width != masterWidth || hm.height != masterHeight) {
            std::cerr << "Warning: LOD " << i << " dimensions (" << hm.width << "x" << hm.height
                      << ") do not match master dimensions (" << masterWidth << "x" << masterHeight << ")" << std::endl;
        }

        // Define where to copy the data TO in the GPU texture
        wgpu::TexelCopyTextureInfo destination = {};
        destination.texture = m_textureArray;// here same as in buffercase pass the created buffer obj
        destination.origin = { 0, 0, i }; // The 'i' here selects the array layer!
    
        // Define how the data is laid out in your CPU-side vector
        wgpu::TexelCopyBufferLayout sourceLayout = {};
        sourceLayout.bytesPerRow = hm.width * sizeof(float);
        sourceLayout.rowsPerImage = hm.height;

        // Execute the upload
        wgpu::Extent3D copySize = { (uint32_t)hm.width, (uint32_t)hm.height, 1 };
        m_device.GetQueue().WriteTexture(
            &destination,
            hm.data.data(),
            hm.data.size() * sizeof(float),
            &sourceLayout,
            &copySize
        );
    }

    // --- 4. Create a Sampler and the Bind Group for the shader ---
    m_sampler = m_device.CreateSampler(); // Default sampler is fine

    // Define the layout the shader expects for this bind group
    wgpu::BindGroupLayoutEntry bglEntries[3] = {};
    bglEntries[0].binding = 0;
    bglEntries[0].visibility = wgpu::ShaderStage::Vertex; // Or Vertex, if you use it there
    bglEntries[0].texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
    bglEntries[0].texture.viewDimension = wgpu::TextureViewDimension::e2DArray; // CRITICAL

    bglEntries[1].binding = 1;
    bglEntries[1].visibility = wgpu::ShaderStage::Vertex; // Or Vertex
    bglEntries[1].sampler.type = wgpu::SamplerBindingType::NonFiltering;

    bglEntries[2].binding = 2;
    bglEntries[2].visibility = wgpu::ShaderStage::Vertex;
    bglEntries[2].buffer.type = wgpu::BufferBindingType::Uniform;

    wgpu::BindGroupLayoutDescriptor bglDesc = { .entryCount = 3, .entries = bglEntries };
    m_bindGroupLayout = m_device.CreateBindGroupLayout(&bglDesc);

    // Create the texture view for the *entire array*
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.dimension = wgpu::TextureViewDimension::e2DArray; // CRITICAL
    viewDesc.arrayLayerCount = layerCount;
    
    // Link the actual resources to the bindings
    wgpu::BindGroupEntry bgEntries[3] = {};
    bgEntries[0].binding = 0;
    bgEntries[0].textureView = m_textureArray.CreateView(&viewDesc);
    bgEntries[1].binding = 1;
    bgEntries[1].sampler = m_sampler;
    bgEntries[2].binding = 2;
    bgEntries[2].buffer = terrainUniformBuffer;
    bgEntries[2].size = sizeof(terrainUniforms);

    wgpu::BindGroupDescriptor bgDesc = { .layout = m_bindGroupLayout, .entryCount = 3, .entries = bgEntries };
    m_bindGroup = m_device.CreateBindGroup(&bgDesc);

    std::cout << "Successfully created and uploaded LOD texture array with " << layerCount << " layers." << std::endl;
}