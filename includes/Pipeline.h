#pragma once
#include <webgpu/webgpu_cpp.h>
#include <string> 
#include "Mesh.h" 

struct VertexPipelineLayoutData {
    wgpu::VertexAttribute vertexAttributes[1]; // Use an array
    wgpu::VertexBufferLayout vertexBufferLayout;
    wgpu::VertexState vertexState;
    // You could also add FragmentState, BindGroupLayouts, etc., here.
};

struct PipelineConfig{
    wgpu::TextureFormat surfaceFormat;
    wgpu::PipelineLayout layout;
    std::string shaderCode;
    wgpu::PrimitiveTopology topology=wgpu::PrimitiveTopology::LineList;
    wgpu::IndexFormat indexFormat=wgpu::IndexFormat::Uint32;
};


class Pipeline
{
    public:
        Pipeline(const wgpu::Device& device,
                 const PipelineConfig& config,
                 const std::string& shaderCode
                );
        
        wgpu::RenderPipeline getPipeline() const;

        wgpu::VertexState createVertexPipelineLayout(const wgpu::ShaderModule& shaderModule, VertexPipelineLayoutData& layoutData);
        

    private:
        wgpu::RenderPipeline m_pipeline;
};
