#include "Pipeline.h"


Pipeline::Pipeline(
    const wgpu::Device& device,
    const PipelineConfig& config,
    const std::string& shaderCode
){

    VertexPipelineLayoutData layoutData;

    wgpu::ShaderSourceWGSL wgsl{{.code = shaderCode.c_str()}};
    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain = &wgsl};
    wgpu::ShaderModule shaderModule =
        device.CreateShaderModule(&shaderModuleDescriptor);
    

    // creates the layout
    wgpu::VertexState vertexState=createVertexPipelineLayout(shaderModule,layoutData);


    wgpu::ColorTargetState colorTargetState{.format = config.surfaceFormat}; // Use the passed-in format
    
    wgpu::FragmentState fragmentState{
        .module = shaderModule, 
        .targetCount = 1, 
        .targets = &colorTargetState
    };

    wgpu::PrimitiveState primitiveState{
        .topology = config.topology,
        // .stripIndexFormat = config.indexFormat,
        
    };
    


    wgpu::RenderPipelineDescriptor descriptor{
        .layout = config.layout,
        .vertex = vertexState,
        .primitive = primitiveState,
        .fragment = &fragmentState
    };
    m_pipeline = device.CreateRenderPipeline(&descriptor);

}

wgpu::VertexState Pipeline::createVertexPipelineLayout(const wgpu::ShaderModule& shaderModule, VertexPipelineLayoutData& layoutData) {

    layoutData.vertexAttributes[0] = {
        .format = wgpu::VertexFormat::Float32x3,
        .offset = 0,
        .shaderLocation = 0
    };

    layoutData.vertexBufferLayout = {
        .arrayStride = sizeof(Vertex),
        .attributeCount = 1,
        .attributes = layoutData.vertexAttributes
    };

    wgpu::VertexState vertexState{
        .module = shaderModule,
        .entryPoint = "vertexMain",
        .bufferCount = 1,
        .buffers = &layoutData.vertexBufferLayout
    };

    return vertexState;     
}

wgpu::RenderPipeline Pipeline::getPipeline() const{
    return m_pipeline;
     
}