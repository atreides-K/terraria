#include "Pipeline.h"

const char shaderCode[] = R"(
    // This struct must match the data you send from the CPU
    struct CameraUniforms {
        view_projection_matrix: mat4x4<f32>,
    };

    // Tell the shader where to find this uniform data
    @group(0) @binding(0)
    var<uniform> camera: CameraUniforms;

    struct VertexInput{
      @location(0) position : vec3f
    };

    @vertex fn vertexMain(input:VertexInput) ->
      @builtin(position) vec4f {
        // Use the matrix to transform the vertex position
        return camera.view_projection_matrix * vec4f(input.position, 1.0);
    }

    @fragment fn fragmentMain() -> @location(0) vec4f {
      return vec4f(0.3, 0.8, 0.4, 1.0); 
    }
)";



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