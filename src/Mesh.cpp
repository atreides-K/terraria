#include "Mesh.h"
#include "BufferUtils.h"
#include <webgpu/webgpu_cpp.h>

void Mesh::operator()(const wgpu::Device& device){
    std::vector<Vertex> mesh = createMesh();
    m_vertexBuffer=BufferUtils::createVertexBuffer(device,mesh);
    m_vertexCount=static_cast<uint32_t>(mesh.size());
    std::vector<uint32_t> indices=createIndices();
    m_indexCount=static_cast<uint32_t>(indices.size());
    m_indexBuffer = BufferUtils::createIndexBuffer(device,indices);
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

std::vector<Vertex> Mesh::createMesh(){
        std::vector<Vertex> mesh;
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
                Vertex v;
                // Center the grid aroundorigin
                v.position[0]=i*spacing-(N*spacing)/2.0f;
                v.position[1]=0.0f;
                v.position[2]=j*spacing-(N*spacing)/2.0f;
                mesh.push_back(v);
            }
        }
        return mesh;
    }

// std::vector<uint32_t> Mesh::createIndices(){
//     std::vector<uint32_t> indices;
//     for(int i=0;i<N-1;i++){
//         for(int j=0;j<N;j++){
//             // should form triandgle in patter 1234 -> 123 234 firm triangles 
//             indices.push_back(i*N + j);
//             indices.push_back((i+1)*N + j);            
//         }
//     }
//     return indices;
// }
std::vector<uint32_t> Mesh::createIndices(){
    std::vector<uint32_t> indices;
    for(int i=0;i<N-1;i++){
        for(int j=0;j<N-1;j++){
            // should form triandgle in patter 1234 -> 123 234 firm triangles 
            indices.push_back(i*N + j);
            indices.push_back((i+1)*N + j);
            indices.push_back(i*N + j);
            indices.push_back(i*N + j+1);
            indices.push_back((i+1)*N + j);
            indices.push_back(i*N + j+1);
            indices.push_back(i*N + j+1);
            indices.push_back((i+1)*N + j+1);
        }
    }
    return indices;
}


