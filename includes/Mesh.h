#pragma once

#include <webgpu/webgpu_cpp.h>
#include <vector>

// We need the definition of the 'Vertex' struct.
// It's good practice to keep this in a common header.
// Assuming it's still in your BufferUtils.h
#include "BufferUtils.h"

const int N=11;
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

    

private:
    // Private member variables. The Mesh object owns these resources.
    wgpu::Buffer m_vertexBuffer;
    wgpu::Buffer m_indexBuffer;
    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;
    std::vector<Vertex> createMesh();
    std::vector<uint32_t> createIndices();
};