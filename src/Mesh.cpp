#include "Mesh.h"
#include "BufferUtils.h"
#include <webgpu/webgpu_cpp.h>
#include <iostream>

void Mesh::operator()(const wgpu::Device& device){
    std::vector<Vertex> mesh = createSmallMesh();
    m_vertexBuffer=BufferUtils::createVertexBuffer(device,mesh);
    m_vertexCount=static_cast<uint32_t>(mesh.size());
    std::vector<uint32_t> indices=createIndices();
    m_indexCount=static_cast<uint32_t>(indices.size());
    m_indexBuffer = BufferUtils::createIndexBuffer(device,indices);

    std::vector<Vertex> rfu = createMesh(m*4-1, m*4-1);
    rfu_vertexBuffer=BufferUtils::createVertexBuffer(device,rfu);
    rfu_vertexCount=static_cast<uint32_t>(rfu.size());
    indices=createIndices(m*4-1,m*4-1);
    rfu_indexCount=static_cast<uint32_t>(indices.size());
    rfu_indexBuffer = BufferUtils::createIndexBuffer(device,indices);

    std::vector<Vertex> trim = createLMesh(m*2+1);
    trim_vertexBuffer=BufferUtils::createVertexBuffer(device,trim);
    trim_vertexCount=static_cast<uint32_t>(trim.size());
    indices=createLIndices_wf(m*2+1,2);
    trim_indexCount=static_cast<uint32_t>(indices.size());
    trim_indexBuffer = BufferUtils::createIndexBuffer(device,indices);
    
    std::vector<Vertex> trimH = createSmallMesh(2,m*2+1);
    trimH_vertexBuffer=BufferUtils::createVertexBuffer(device,trimH);
    trimH_vertexCount=static_cast<uint32_t>(trimH.size());
    indices=createIndices(m*2+1,2);
    trimH_indexCount=static_cast<uint32_t>(indices.size());
    trimH_indexBuffer = BufferUtils::createIndexBuffer(device,indices);
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

uint32_t Mesh::getTrimHVertexCount() const{
    return trimH_vertexCount;
}
uint32_t Mesh::getTrimHIndexCount() const{
    return trimH_indexCount;
}
wgpu::Buffer Mesh::getTrimHVertexBuffer() const{
    return trimH_vertexBuffer;
}
wgpu::Buffer Mesh::getTrimHIndexBuffer() const{
    return trimH_indexBuffer;
}

std::vector<Vertex> Mesh::createMesh(const int& h,const int& w){
    std::cout<<"Creating mesh with height: "<<h<<" and width: "<<w<<std::endl;
        std::vector<Vertex> mesh;
        for(int i=0;i<w;i++){
            for(int j=0;j<h;j++){
                Vertex v;
                // Center the grid aroundorigin
                v.position[0]=i;
                v.position[1]=0.0f;
                v.position[2]=j;
                mesh.push_back(v);
                // std::cout<<"Vertex "<<(i*h + j)<<" : ("<<v.position[0]<<","<<v.position[1]<<","<<v.position[2]<<")"<<std::endl;
            }
        }
        return mesh;
    }

std::vector<Vertex> Mesh::createSmallMesh(const int& h,const int& w){
    std::cout<<"Creating mesh with height: "<<h<<" and width: "<<w<<std::endl;
        std::vector<Vertex> mesh;
        for(int i=0;i<h;i++){
            for(int j=0;j<w;j++){
                Vertex v;

                // top to bottom square mesh
                v.position[0]=-i;
                v.position[1]=0.0f;
                v.position[2]=j;
                mesh.push_back(v);
                // std::cout<<"Vertex "<<(i*h + j)<<" : ("<<v.position[0]<<","<<v.position[1]<<","<<v.position[2]<<")"<<std::endl;
            }
        }
        return mesh;
    }

std::vector<Vertex> Mesh::createLMesh(const int& m){
    std::cout<<"Creating L mesh with size: "<<m<<std::endl;
        std::vector<Vertex> mesh;
        for(int i=0;i<m;i++){
            for(int j=-1;j<1;j++){
                Vertex v;

                
                v.position[0]=i;
                v.position[1]=0.0f;
                v.position[2]=j;
                mesh.push_back(v);
               
                std::cout<<"Vertex "<<(i*2 + (j+1))<<" : ("<<v.position[0]<<","<<v.position[1]<<","<<v.position[2]<<")"<<std::endl;
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

std::vector<uint32_t> Mesh::createLIndices_wf(const int& h,const int& w){
    std::vector<uint32_t> indices;
    int i=0;
    int j=0;
    std::cout<<"Creating L Indices with height: "<<h<<" and width: "<<w<<std::endl;
    for(i=0;i<h*2-2;i+=2){
        // indices.push_back(i);
        // indices.push_back(i + 1);
        indices.push_back(i );
        indices.push_back(i + 2);
        indices.push_back(i);
        indices.push_back(i + 3);
        indices.push_back(i +1);
        indices.push_back(i + 3);
        indices.push_back(i +2);
        indices.push_back(i + 3);

    }
    return indices;
}


