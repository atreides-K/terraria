#include "BufferUtils.h"
#include <cstring>

namespace BufferUtils{
    wgpu::Buffer createVertexBuffer(
        const wgpu::Device& device, 
        const std::vector<Vertex>& vertices){
        
        wgpu::BufferDescriptor bufferDescriptor{
            .usage=wgpu::BufferUsage::Vertex|wgpu::BufferUsage::CopyDst,
            .size=vertices.size() * sizeof(Vertex),
            .mappedAtCreation=true
        };
        wgpu::Buffer buffer=device.CreateBuffer(&bufferDescriptor);
        void* mapped=buffer.GetMappedRange();
        memcpy(mapped,vertices.data(),bufferDescriptor.size);
        buffer.Unmap();
        return buffer;
    }

    
    // UniformBinding createUniformBinding(
    //     const wgpu::Device& device,
    //     const std::vector<Vertex>& vertices
    // ) {
    //     UniformBinding binding; // Create an instance of our return struct

    //     // 1. Create the Bind Group Layout (defines the "shape")
    //     wgpu::BindGroupLayoutEntry bindingLayoutEntry = {
    //         .binding = 0, // Matches @binding(0) in the shader
    //         .visibility = wgpu::ShaderStage::Vertex,
    //         .buffer = { .type = wgpu::BufferBindingType::Uniform }
    //     };

    //     wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{
    //         .entryCount = 1,
    //         .entries = &bindingLayoutEntry
    //     };
    //     binding.bindGroupLayout = device.CreateBindGroupLayout(&bindGroupLayoutDesc);
        

    //     // 2. Create the Uniform Buffer
    //     wgpu::BufferDescriptor uniformBuffDesc{
    //         .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
    //         .size = sizeof(Uniforms)
    //     binding.buffer = device.CreateBuffer(&uniformBuffDesc);


    //     // 3. Create the Bind Group (links the buffer to the layout)
    //     wgpu::BindGroupEntry bindGroupEntry{
    //         .binding = 0,
    //         .buffer = binding.buffer,
    //         .size = sizeof(Uniforms)
    //     };

    //     wgpu::BindGroupDescriptor bindGroupDesc{
    //         .layout = binding.bindGroupLayout,
    //         .entryCount = 1,
    //         .entries = &bindGroupEntry
    //     };
    //     binding.bindGroup = device.CreateBindGroup(&bindGroupDesc);

    //     return binding;
    // }
// }
}