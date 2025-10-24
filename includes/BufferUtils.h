#pragma once

#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include <vector>

// lets defines camera
struct Uniforms{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct UniformBinding{
    wgpu::Buffer buffer;
    wgpu::BindGroup bindGroup;
    wgpu::BindGroupLayout bindGroupLayout;
};

// for vertex buffer
struct Vertex{
    float position[3];
};

namespace BufferUtils {
    wgpu::Buffer createVertexBuffer(
        const wgpu::Device& device,
        const std::vector<Vertex>& vertices
        );

    wgpu::Buffer createIndexBuffer(
        const wgpu::Device& device,
        const std::vector<uint32_t>& indices
    );

    wgpu::Buffer createUniformBuffer(
        const wgpu::Device& device,
         uint64_t size
        );
    
}  

