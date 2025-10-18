#pragma once
#include <webgpu/webgpu_cpp.h>
#include "Mesh.h" 
class Pipeline
{
    public:
        Pipeline(const wgpu::Device& device,
                 const wgpu::TextureFormat& surfaceFormat);
        
        wgpu::RenderPipeline getPipeline() const;

    private:
        wgpu::RenderPipeline m_pipeline;
};