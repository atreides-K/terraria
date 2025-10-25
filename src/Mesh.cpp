#include "Mesh.h"
#include "BufferUtils.h"
#include <webgpu/webgpu_cpp.h>

void Mesh::operator()(const wgpu::Device& device){
    std::vector<Vertex> mesh = createMesh(10,3);
    m_vertexBuffer=BufferUtils::createVertexBuffer(device,mesh);
    m_vertexCount=static_cast<uint32_t>(mesh.size());
    std::vector<uint32_t> indices=createIndices(10,3);
    m_indexCount=static_cast<uint32_t>(indices.size());
    m_indexBuffer = BufferUtils::createIndexBuffer(device,indices);

    // std::vector<Vertex> rfu = createMesh(m,3);
    // rfu_vertexBuffer=BufferUtils::createVertexBuffer(device,rfu);
    // rfu_vertexCount=static_cast<uint32_t>(rfu.size());
    // indices=createIndices(m,3);
    // rfu_indexCount=static_cast<uint32_t>(indices.size());
    // rfu_indexBuffer = BufferUtils::createIndexBuffer(device,indices);

    // std::vector<Vertex> trim = createMesh(m,2);
    // trim_vertexBuffer=BufferUtils::createVertexBuffer(device,trim);
    // trim_vertexCount=static_cast<uint32_t>(trim.size());
    // indices=createIndices(m,2);
    // trim_indexCount=static_cast<uint32_t>(indices.size());
    // trim_indexBuffer = BufferUtils::createIndexBuffer(device,indices);
}

wgpu::Buffer Mesh::getVertexBuffer() const{
    return m_vertexBuffer;
}

uint32_t Mesh::getVertexCount() const{
    return m_vertexCount;
}
wgpu::Buffer Mesh::getIndexBuffer() const{
    return m_indexBuffer;
}
uint32_t Mesh::getIndexCount() const{
    return m_indexCount;
}

wgpu::Buffer Mesh::getRfuVertexBuffer() const{
    return rfu_vertexBuffer;
}

wgpu::Buffer Mesh::getRfuIndexBuffer() const{
    return rfu_indexBuffer;
}

uint32_t Mesh::getRfuVertexCount() const{
    return rfu_vertexCount;
}

uint32_t Mesh::getRfuIndexCount() const{
    return rfu_indexCount;
}

wgpu::Buffer Mesh::getTrimVertexBuffer() const{
    return trim_vertexBuffer;
}

wgpu::Buffer Mesh::getTrimIndexBuffer() const{
    return trim_indexBuffer;
}

uint32_t Mesh::getTrimVertexCount() const{
    return trim_vertexCount;
}

uint32_t Mesh::getTrimIndexCount() const{
    return trim_indexCount;
}

std::vector<Vertex> Mesh::createMesh(const int& h,const int& w){
        std::vector<Vertex> mesh;
        for(int i=0;i<w;i++){
            for(int j=0;j<h;j++){
                Vertex v;
                // Center the grid aroundorigin
                v.position[0]=i*spacing-(w*spacing)/2.0f;
                v.position[1]=0.0f;
                v.position[2]=j*spacing-(h*spacing)/2.0f;
                mesh.push_back(v);
            }
        }
        return mesh;
    }

// std::vector<uint32_t> Mesh::createIndices(const int& h,const int& w){
//     std::vector<uint32_t> indices;
//     for(int i=0;i<w-1;i++){
//         for(int j=0;j<h-1;j++){
//             // should form triandgle in patter 1234 -> 123 234 firm triangles 
//             indices.push_back(i*h + j);
//             indices.push_back((i+1)*h + j);
//         }
//     }
//     return indices;
// }
std::vector<uint32_t> Mesh::createIndices(const int& h,const int& w){
    std::vector<uint32_t> indices;
    for(int i=0;i<w-1;i++){
        for(int j=0;j<h-1;j++){
            // should form triandgle in patter 1234 -> 123 234 firm triangles 
            indices.push_back(i*h + j);
            indices.push_back((i+1)*h + j);
            indices.push_back(i*h + j);
            indices.push_back(i*h + j+1);
            indices.push_back((i+1)*h + j);
            indices.push_back(i*h + j+1);
            indices.push_back(i*h + j+1);
            indices.push_back((i+1)*h + j+1);
        }
    }
    return indices;
}


