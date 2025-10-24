#include <iostream>

#include <GLFW/glfw3.h>
#if defined(__EMSCRIPTEN__)
  // #include <emscripten/emscripten.h>
  #include <emscripten/html5.h>
#endif
#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_glfw.h>
#include <vector>
#include <cstring>

#include "BufferUtils.h"
#include "Pipeline.h"
#include "Mesh.h"

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
#include <map>
#include <string>
#include <demLoader.h>

std::map<std::string, bool> keyStates;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));




wgpu::Buffer uniformBuffer;
wgpu::BindGroup uniformBindGroup;

// mesh
Mesh mesh;

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

void Render() {
  wgpu::SurfaceTexture surfaceTexture;
  surface.GetCurrentTexture(&surfaceTexture);

  // 1. Ask the camera class to compute the final view matrix from its current state.
  glm::mat4 view = camera.getViewMatrix(); 

  // 2. Create the projection matrix.
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)kWidth / (float)kHeight, 0.1f, 100.0f);

  // 3. Combine them. THIS is the data the GPU needs.
  glm::mat4 viewProjMatrix = projection * view;

  // 4. Send that single matrix to the uniform buffer.
  device.GetQueue().WriteBuffer(uniformBuffer, 0, &viewProjMatrix, sizeof(glm::mat4));


  wgpu::RenderPassColorAttachment attachment{
      .view = surfaceTexture.texture.CreateView(),
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store};

  wgpu::RenderPassDescriptor renderpass{.colorAttachmentCount = 1,
                                        .colorAttachments = &attachment};

  wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
   pass.SetBindGroup(0, uniformBindGroup);


  
  pass.SetPipeline(pipeline);
  pass.SetVertexBuffer(0, vertexBuffer, 0, sizeof(Vertex) * plane.size());
  pass.SetIndexBuffer(mesh.getIndexBuffer(), wgpu::IndexFormat::Uint32, 
  0,
  sizeof(uint32_t) * mesh.getIndexCount());
  // void DrawIndexed(
//     uint32_t indexCount,
//     uint32_t instanceCount = 1,
//     uint32_t firstIndex = 0,
//     int32_t baseVertex = 0,
//     uint32_t firstInstance = 0
// );
  pass.DrawIndexed(mesh.getIndexCount(), 1, 0, 0, 0);
  pass.End();
  wgpu::CommandBuffer commands = encoder.Finish();
  device.GetQueue().Submit(1, &commands);
}

void InitGraphics() {
  ConfigureSurface();
  mesh(device);
  // LAYOUT SETUP
  wgpu::BindGroupLayoutEntry bglEntry{};
  bglEntry.binding = 0;
  bglEntry.visibility = wgpu::ShaderStage::Vertex;
  bglEntry.buffer.type = wgpu::BufferBindingType::Uniform;
  wgpu::BindGroupLayoutDescriptor bglDesc{};
  bglDesc.entryCount = 1;
  bglDesc.entries = &bglEntry;
  wgpu::BindGroupLayout cameraBindGroupLayout = device.CreateBindGroupLayout(&bglDesc);

  // B. Create a master layout for the pipeline that includes this new "port"
  wgpu::PipelineLayoutDescriptor layoutDesc{};
  layoutDesc.bindGroupLayoutCount = 1;
  layoutDesc.bindGroupLayouts = &cameraBindGroupLayout;
  wgpu::PipelineLayout CameraPipelineLayout = device.CreatePipelineLayout(&layoutDesc);


  PipelineConfig MeshConfig{};
  MeshConfig.surfaceFormat = format;
  MeshConfig.layout = CameraPipelineLayout;
  pipeline=Pipeline(device,MeshConfig).getPipeline();


  // BUFFER SETUP
  vertexBuffer = mesh.getVertexBuffer();
  uniformBuffer = BufferUtils::createUniformBuffer(device, sizeof(glm::mat4));

  
  // BIND GROUP SETUP
  wgpu::BindGroupEntry bgEntry{};
  bgEntry.binding = 0; // Corresponds to @binding(0) in shader
  bgEntry.buffer = uniformBuffer;
  bgEntry.size = sizeof(glm::mat4);

  wgpu::BindGroupDescriptor bgDesc{};
  bgDesc.layout = cameraBindGroupLayout; // The "contract" this bind group follows
  bgDesc.entryCount = 1;
  bgDesc.entries = &bgEntry;
  uniformBindGroup = device.CreateBindGroup(&bgDesc);

}



void processInput(GLFWwindow *window) {
#if defined(__EMSCRIPTEN__)
    // Web version: Read from our state map.
    // Note: We check for both lowercase and uppercase to be safe.
    if (keyStates["w"] || keyStates["W"])
        camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    if (keyStates["s"] || keyStates["S"])
        camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (keyStates["a"] || keyStates["A"])
        camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    if (keyStates["d"] || keyStates["D"])
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    if (keyStates["d"] || keyStates["D"])
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    if (keyStates["q"] || keyStates[" "])
        camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
    if (keyStates["Shift"] || keyStates["SHIFT"])
        camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
#else
    // Native desktop version: Use polling with glfwGetKey.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
#endif
}
// This function will be called by Emscripten whenever a key is pressed down.


#if defined(__EMSCRIPTEN__)
EM_BOOL keydown_callback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
    // The keyEvent->key field gives us a string like "w", "s", "Shift", etc.
    // We store that this key is now pressed down.
    keyStates[keyEvent->key] = true;

    // Return true to "consume" the event and prevent the browser from also handling it
    // (e.g., scrolling the page when you press 's').
    return EM_TRUE;
}

// This function will be called by Emscripten whenever a key is released.
EM_BOOL keyup_callback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
    // We store that this key is now released.
    keyStates[keyEvent->key] = false;
    return EM_TRUE;
}
#endif


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}



void Start() {
  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window =
      glfwCreateWindow(kWidth, kHeight, "WebGPU window", nullptr, nullptr);

  // ADD THIS BACK
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

  InitGraphics();
  #if defined(__EMSCRIPTEN__)
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, keydown_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, keyup_callback);
  #endif

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop_arg(
      [](void* arg) {
        GLFWwindow* window = reinterpret_cast<GLFWwindow*>(arg);
        // We still need a way to pass deltaTime to processInput on the web.
        // Let's calculate it inside the loop.
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        Render();
      },
      window, 0, true);
#else
  while (!glfwWindowShouldClose(window)) {
    // ADD THIS BACK for smooth, frame-rate independent movement
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window); // ADD THIS BACK

    glfwPollEvents();
    Render();
    surface.Present();00
    instance.ProcessEvents();
  }
#endif
}

int main() {
  // try {
  //       // Use the VIRTUAL paths you specified in CMake.
  //       const std::string hdrPath = "/data/output.hdr";
  //       const std::string rawPath = "/data/output.raw";

  //       std::cout << "load DEM from vfs" << std::endl;
  //       DEMLoader loader(rawPath, hdrPath);

  //       // You can now use the loaded data...
  //       int width = loader.getWidth();
  //       int height = loader.getHeight();
  //       std::cout << "Successfully loaded DEM with dimensions: " 
  //                 << width << "x" << height << std::endl;

  //   } catch (const std::runtime_error& e) {
  //       std::cerr << "An error occurred: " << e.what() << std::endl;
  //   }
  Init();
  Start();
}