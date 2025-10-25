#pragma once

#include <webgpu/webgpu_cpp.h>
#include <vector>

// We need the definition of the 'Vertex' struct.
// It's good practice to keep this in a common header.
// Assuming it's still in your BufferUtils.h
#include "BufferUtils.h"

// int x=11;
const int m=10;
const float spacing = 1.0f;



class Mesh {
public:
    // Default constructor for creating uninitialized Mesh objects
    Mesh() = default;

    // The main constructor that takes vertex data and creates the GPU buffer.
    void operator()(const wgpu::Device& device);

    // Public "getter" methods for the renderer to use.
    // The renderer needs these to issue draw commands.
    wgpu::Buffer getVertexBuffer() const;
    wgpu::Buffer getIndexBuffer() const;
    uint32_t getVertexCount() const;
    uint32_t getIndexCount() const;

    // New getter methods for the additional mesh types
    wgpu::Buffer getRfuVertexBuffer() const;
    wgpu::Buffer getRfuIndexBuffer() const;
    uint32_t getRfuVertexCount() const;
    uint32_t getRfuIndexCount() const;

    wgpu::Buffer getTrimVertexBuffer() const;
    wgpu::Buffer getTrimIndexBuffer() const;
    uint32_t getTrimVertexCount() const;
    uint32_t getTrimIndexCount() const;

private:
    // Private member variables. The Mesh object owns these resources.
    wgpu::Buffer m_vertexBuffer;
    wgpu::Buffer m_indexBuffer;
    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;

    // rfu
    wgpu::Buffer rfu_vertexBuffer;
    wgpu::Buffer rfu_indexBuffer;
    uint32_t rfu_vertexCount = 0;
    uint32_t rfu_indexCount = 0;

     // trim
    wgpu::Buffer trim_vertexBuffer;
    wgpu::Buffer trim_indexBuffer;
    uint32_t trim_vertexCount = 0;
    uint32_t trim_indexCount = 0;
    std::vector<Vertex> createMesh(const int& h=m,const int& w=m);
    std::vector<uint32_t> createIndices(const int& h=m,const int& w=m);
};               