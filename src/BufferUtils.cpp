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

    wgpu::Buffer createIndexBuffer(
        const wgpu::Device& device, 
        const std::vector<uint32_t>& indices){
        
        wgpu::BufferDescriptor bufferDescriptor{
            .usage=wgpu::BufferUsage::Index|wgpu::BufferUsage::CopyDst,
            .size=indices.size() * sizeof(uint32_t),
            .mappedAtCreation=true
        };
        wgpu::Buffer buffer=device.CreateBuffer(&bufferDescriptor);
        void* mapped=buffer.GetMappedRange();
        memcpy(mapped,indices.data(),bufferDescriptor.size);
        buffer.Unmap();
        return buffer;
    }
    wgpu::Buffer createUniformBuffer(
        const wgpu::Device& device,
         uint64_t size
    ) {
        wgpu::Buffer buffer; // Create an instance of our return struct

        // Create the Uniform Buffer
        wgpu::BufferDescriptor uniformBuffDesc{
            .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
            .size =  size,
        };
        buffer = device.CreateBuffer(&uniformBuffDesc);

        return buffer;
    }

}