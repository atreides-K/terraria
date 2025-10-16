#include <iostream>

#include <GLFW/glfw3.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_glfw.h>
#include <vector>
#include <cstring>

#include "BufferUtils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

wgpu::Instance instance;
wgpu::Adapter adapter;
wgpu::Device device;
wgpu::RenderPipeline pipeline;

wgpu::Surface surface;
wgpu::TextureFormat format;
const uint32_t kWidth = 1000;
const uint32_t kHeight = 800;


// Camera 

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));




wgpu::Buffer uniformBuffer;
wgpu::BindGroup uniformBindGroup;

// NEW: Mouse input state
float lastX = kWidth / 2.0f;
float lastY = kHeight / 2.0f;
bool firstMouse = true;

// NEW: Timing for smooth movement
float deltaTime = 0.0f;
float lastFrame = 0.0f;



std::vector<Vertex> plane = {
  {{-0.8f, -1.0f, 0.0f}},
  {{ 1.0f, -1.0f, 0.0f}},
  {{-1.0f,  1.0f, 0.0f}},
  {{ 0.8f, 0.9f, 0.0f}},
  
};
wgpu::Buffer vertexBuffer;
UniformBinding uniformBinding;





void ConfigureSurface() {
  wgpu::SurfaceCapabilities capabilities;
  surface.GetCapabilities(adapter, &capabilities);
  format = capabilities.formats[0];
  
  if (format == wgpu::TextureFormat::Undefined) {
    std::cerr << "No valid surface format!" << std::endl;
    exit(1);
  }
  wgpu::SurfaceConfiguration config{.device = device,
                                    .format = format,
                                    .width = kWidth,
                                    .height = kHeight,
                                    .presentMode = wgpu::PresentMode::Fifo};
  surface.Configure(&config);
}

void Init() {
  static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instanceDesc{.requiredFeatureCount = 1,
                                        .requiredFeatures = &kTimedWaitAny};
  instance = wgpu::CreateInstance(&instanceDesc);

  wgpu::Future f1 = instance.RequestAdapter(
      nullptr, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestAdapterStatus status, wgpu::Adapter a,
         wgpu::StringView message) {
        if (status != wgpu::RequestAdapterStatus::Success) {
          std::cout << "RequestAdapter: " << message << "\n";
          exit(0);
        }
        adapter = std::move(a);
      });
  instance.WaitAny(f1, UINT64_MAX);

  wgpu::DeviceDescriptor desc{};
  desc.SetUncapturedErrorCallback([](const wgpu::Device&,
                                     wgpu::ErrorType errorType,
                                     wgpu::StringView message) {
    std::cout << "Error: " << errorType << " - message: " << message << "\n";
  });

  wgpu::Future f2 = adapter.RequestDevice(
      &desc, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestDeviceStatus status, wgpu::Device d,
         wgpu::StringView message) {
        if (status != wgpu::RequestDeviceStatus::Success) {
          std::cout << "RequestDevice: " << message << "\n";
          exit(0);
        }
        device = std::move(d);
      });
  instance.WaitAny(f2, UINT64_MAX);
}

// const char shaderCode[] = R"(
//     // // This struct MUST match the memory layout of your C++ struct/matrix
//     // struct MVP_Matrix {
//     //     matrix : mat4x4f
//     // };

//     // This tells the shader to expect a uniform buffer at slot 0
//     // of the resource binding group 0.
//     @group(0) @binding(0) var<uniform> mvp: MVP_Matrix;

//     struct VertexInput{
//       @location(0) position : vec3f
//     };

//     @vertex fn vertexMain(input:VertexInput) ->
//       @builtin(position) vec4f {
//         // Apply the transformation matrix to the vertex position
//         return vec4f(input.position, 1.0);
//     }

//     @fragment fn fragmentMain() -> @location(0) vec4f {
//         return vec4f(1, 0, 0, 1);
//     }
// )";
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

void CreateRenderPipeline() {
  wgpu::ShaderSourceWGSL wgsl{{.code = shaderCode}};

  wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain = &wgsl};
  wgpu::ShaderModule shaderModule =
      device.CreateShaderModule(&shaderModuleDescriptor);
  
  wgpu::VertexAttribute vertexAttrib{
    .format = wgpu::VertexFormat::Float32x3,
    .offset = 0,
    .shaderLocation = 0
  };
  wgpu::VertexBufferLayout vertexLayout{
    .arrayStride = sizeof(Vertex),
    .attributeCount = 1,
    .attributes = &vertexAttrib
  };

  wgpu::VertexState vertexState{
      .module = shaderModule,
      .entryPoint = "vertexMain",
      .bufferCount = 1,
      .buffers = &vertexLayout};
    
  wgpu::ColorTargetState colorTargetState{.format = format};
    
  wgpu::FragmentState fragmentState{
      .module = shaderModule, 
      .targetCount = 1, 
      .targets = &colorTargetState
    };
  wgpu::PrimitiveState primitiveState{
    .topology = wgpu::PrimitiveTopology::TriangleStrip};
  wgpu::RenderPipelineDescriptor descriptor{.vertex = vertexState,
    .primitive = primitiveState,
    .fragment = &fragmentState
                                          };
  pipeline = device.CreateRenderPipeline(&descriptor);
}



void Render() {
  wgpu::SurfaceTexture surfaceTexture;
  surface.GetCurrentTexture(&surfaceTexture);

  wgpu::RenderPassColorAttachment attachment{
      .view = surfaceTexture.texture.CreateView(),
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store};

  wgpu::RenderPassDescriptor renderpass{.colorAttachmentCount = 1,
                                        .colorAttachments = &attachment};

  wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
  pass.SetPipeline(pipeline);
  pass.SetVertexBuffer(0, vertexBuffer, 0, sizeof(Vertex) * plane.size());
  pass.Draw(4);
  pass.End();
  wgpu::CommandBuffer commands = encoder.Finish();
  device.GetQueue().Submit(1, &commands);
}

void InitGraphics() {
  ConfigureSurface();
  CreateRenderPipeline();
  vertexBuffer = BufferUtils::createVertexBuffer(device, plane);
  // uniformBinding = BufferUtils::createUniformBinding(device,plane);
}

void Start() {
  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window =
      glfwCreateWindow(kWidth, kHeight, "WebGPU window", nullptr, nullptr);
  surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

  InitGraphics();

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop(Render, 0, false);
#else
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    Render();
    surface.Present();
    instance.ProcessEvents();
  }
#endif
}

int main() {
  Init();
  Start();
}