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

// This function is in Pipeline.cpp, which now includes "DataTypes.h"

wgpu::VertexState Pipeline::createVertexPipelineLayout(
    const wgpu::ShaderModule& shaderModule,
    VertexPipelineLayoutData& layoutData
) {
    // --- Mesh vertex buffer (location 0) ---
    layoutData.vertexAttributes[0] = {
        .format = wgpu::VertexFormat::Float32x3, // position
        .offset = 0,
        .shaderLocation = 0
    };

    // FIX: Reordered initializers to match declaration order
    layoutData.vertexBufferLayout = {
        .arrayStride = sizeof(Vertex), 
        .stepMode = wgpu::VertexStepMode::Vertex,
        .attributeCount = 1,
        .attributes = layoutData.vertexAttributes
    };

    // --- Instance buffer (locations 1,2,3) ---
    // Note: The 'static' issue I mentioned before still exists, but this will compile.
    static wgpu::VertexAttribute instanceAttrs[4];

    // These lines now work because "DataTypes.h" is included
    instanceAttrs[0] = { .format = wgpu::VertexFormat::Float32x2, .offset = offsetof(Instance, offset), .shaderLocation = 1 };
    instanceAttrs[1] = { .format = wgpu::VertexFormat::Float32,   .offset = offsetof(Instance, scale),  .shaderLocation = 2 };
    instanceAttrs[2] = { .format = wgpu::VertexFormat::Uint32,    .offset = offsetof(Instance, level),  .shaderLocation = 3 };
    instanceAttrs[3] = { .format = wgpu::VertexFormat::Float32x4, .offset = offsetof(Instance, color),  .shaderLocation = 4 };
    
    static wgpu::VertexBufferLayout instanceLayout = {};
    instanceLayout.arrayStride = sizeof(Instance); // This now works
    instanceLayout.stepMode = wgpu::VertexStepMode::Instance;
    instanceLayout.attributeCount = 4;
    instanceLayout.attributes = instanceAttrs;

    // --- VertexState with two buffers ---
    static wgpu::VertexBufferLayout buffers[2] = { layoutData.vertexBufferLayout, instanceLayout };

    wgpu::VertexState vertexState{
        .module = shaderModule,
        .entryPoint = "vertexMain",
        .bufferCount = 2,
        .buffers = buffers
    };

    return vertexState;
}

wgpu::RenderPipeline Pipeline::getPipeline() const{
    return m_pipeline;
     
}