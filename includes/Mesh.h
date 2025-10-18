#pragma once

#include <webgpu/webgpu_cpp.h>
#include <vector>

// We need the definition of the 'Vertex' struct.
// It's good practice to keep this in a common header.
// Assuming it's still in your BufferUtils.h
#include "BufferUtils.h"

class Mesh {
public:
    // Default constructor for creating uninitialized Mesh objects
    Mesh() = default;

    // The main constructor that takes vertex data and creates the GPU buffer.
    Mesh(const wgpu::Device& device, const std::vector<Vertex>& vertices);

    // Public "getter" methods for the renderer to use.
    // The renderer needs these to issue draw commands.
    wgpu::Buffer getVertexBuffer() const;
    uint32_t getVertexCount() const;

    

private:
    // Private member variables. The Mesh object owns these resources.
    wgpu::Buffer m_vertexBuffer;
    uint32_t m_vertexCount = 0;
};