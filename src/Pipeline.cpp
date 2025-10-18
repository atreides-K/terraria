#include "Pipeline.h"

const char shaderCode[] = R"(
    struct VertexInput{
      @location(0) position : vec3f
    };
    @vertex fn vertexMain(input:VertexInput) ->
      @builtin(position) vec4f {
        return vec4f(input.position, 1.0);
    }
    @fragment fn fragmentMain() -> @location(0) vec4f {
      return vec4f(0.3, 0.8, 0.4, 1.0); 
    }
)";

Pipeline::Pipeline(const wgpu::Device& device,
const wgpu::TextureFormat& surfaceFormat
){
    wgpu::ShaderSourceWGSL wgsl{{.code = shaderCode}};

    
    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain = &wgsl};
    wgpu::ShaderModule shaderModule =
        device.CreateShaderModule(&shaderModuleDescriptor);
    

    wgpu::VertexAttribute vertexAttribute{
        .format=wgpu::VertexFormat::Float32x3,
        .offset=0,
        .shaderLocation=0
    };

    wgpu::VertexBufferLayout vertexLayout{
        .arrayStride=sizeof(Vertex),
        .attributeCount=1,
        .attributes=&vertexAttribute
    };

    wgpu::VertexState vertexState{
        .module = shaderModule,
        .entryPoint = "vertexMain",
        .bufferCount = 1,
        .buffers = &vertexLayout
    };

    wgpu::ColorTargetState colorTargetState{.format = surfaceFormat}; // Use the passed-in format
    
    wgpu::FragmentState fragmentState{
        .module = shaderModule, 
        .targetCount = 1, 
        .targets = &colorTargetState
    };

    wgpu::PrimitiveState primitiveState{
        .topology = wgpu::PrimitiveTopology::TriangleStrip
    };



    wgpu::RenderPipelineDescriptor descriptor{
        .vertex = vertexState,
        .primitive = primitiveState,
        .fragment = &fragmentState
    };
    m_pipeline = device.CreateRenderPipeline(&descriptor);

}

wgpu::RenderPipeline Pipeline::getPipeline() const{
    return m_pipeline;
}