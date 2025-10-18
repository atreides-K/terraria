#include "Mesh.h"
#include "BufferUtils.h"
#include <webgpu/webgpu_cpp.h>

Mesh::Mesh(const wgpu::Device& device,const std::vector<Vertex>& vertices){
    m_vertexCount=static_cast<uint32_t>(vertices.size());
    m_vertexBuffer=BufferUtils::createVertexBuffer(device,vertices);
}

wgpu::Buffer Mesh::getVertexBuffer() const{
    return m_vertexBuffer;
}

uint32_t Mesh::getVertexCount() const{
    return m_vertexCount;
}



