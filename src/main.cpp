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
const uint32_t kWidth = 1920;
const uint32_t kHeight = 1080;


// Camera 
#include <map>
#include <string>
#include <demLoader.h>


// shader
#include <ShaderLoader.h>

std::map<std::string, bool> keyStates;
Camera camera(glm::vec3(0.0f, 30.0f, 0.0f));




wgpu::Buffer uniformBuffer;
wgpu::BindGroup uniformBindGroup;
wgpu::Buffer rfuInstanceBuffer;
wgpu::Buffer rfuHInstanceBuffer;
wgpu::Buffer LInstanceBuffer;
wgpu::Buffer LHInstanceBuffer;


// mesh
Mesh mesh;

// NEW: Mouse input state
float lastX = kWidth / 2.0f;
float lastY = kHeight / 2.0f;
bool firstMouse = true;

// NEW: Timing for smooth movement
float deltaTime = 0.0f;
float lastFrame = 0.0f;




wgpu::Buffer vertexBuffer;
UniformBinding uniformBinding;

wgpu::Buffer instanceBuffer = nullptr;

    // 2. Instance data
    struct InstanceData {
        glm::vec2 offset;
        float scale;
        uint32_t level;
        glm::vec4 color;
    };
    std::vector<InstanceData> instances;
    std::vector<InstanceData> LHInstances;
    std::vector<InstanceData> LInstances;
    std::vector<InstanceData> rfuInstances;
    std::vector<InstanceData> rfuHInstances;

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

    // 1. Update camera uniform buffer
    glm::mat4 view = camera.getViewMatrix(); 
    glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                      (float)kWidth / (float)kHeight,
                                      0.1f, 100.0f);
    glm::mat4 viewProj = proj * view;
    device.GetQueue().WriteBuffer(uniformBuffer, 0, &viewProj, sizeof(glm::mat4));

    
    // 2. Setup render pass
    wgpu::RenderPassColorAttachment attachment{
        .view = surfaceTexture.texture.CreateView(),
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = {0.1, 0.1, 0.15, 1.0}
    };
    wgpu::RenderPassDescriptor passDesc{.colorAttachmentCount = 1, .colorAttachments = &attachment};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);

    // Set pipeline and bind groups once for the entire pass
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, uniformBindGroup);
    
    // --- DRAW 1: The main instanced mesh ---
    pass.SetVertexBuffer(0, mesh.getVertexBuffer());
    pass.SetIndexBuffer(mesh.getIndexBuffer(), wgpu::IndexFormat::Uint32);
    pass.SetVertexBuffer(1, instanceBuffer);
    pass.DrawIndexed(mesh.getIndexCount(), instances.size(), 0, 0, 0);

    // --- DRAW 2: The single "RFU" instance ---
    // Update the data for the single instance
   
    // Bind its geometry and single-instance buffer
    pass.SetVertexBuffer(0, mesh.getRfuVertexBuffer());
    pass.SetIndexBuffer(mesh.getRfuIndexBuffer(), wgpu::IndexFormat::Uint32);
    pass.SetVertexBuffer(1, rfuInstanceBuffer);
    pass.DrawIndexed(mesh.getRfuIndexCount(), rfuInstances.size(), 0, 0, 0); 

    pass.SetVertexBuffer(0, mesh.getRfuHVertexBuffer());
    pass.SetIndexBuffer(mesh.getRfuHIndexBuffer(), wgpu::IndexFormat::Uint32);
    pass.SetVertexBuffer(1, rfuHInstanceBuffer);
    pass.DrawIndexed(mesh.getRfuHIndexCount(), rfuHInstances.size(), 0, 0, 0); 

  
    // Draw L-shaped trim mesh
    pass.SetVertexBuffer(0, mesh.getTrimVertexBuffer());
    pass.SetIndexBuffer(mesh.getTrimIndexBuffer(), wgpu::IndexFormat::Uint32);
    pass.SetVertexBuffer(1, LInstanceBuffer);
    pass.DrawIndexed(mesh.getTrimIndexCount(), LInstances.size(), 0, 0, 0); // Draw exactly one instance

    // device.GetQueue().WriteBuffer(LHInstanceBuffer, 0, LHInstances.data(), sizeof(InstanceData) * LHInstances.size());
    // Draw L-shaped trim mesh
    pass.SetVertexBuffer(0, mesh.getTrimHVertexBuffer());
    pass.SetIndexBuffer(mesh.getTrimHIndexBuffer(), wgpu::IndexFormat::Uint32);
    pass.SetVertexBuffer(1, LHInstanceBuffer);
    pass.DrawIndexed(mesh.getTrimHIndexCount(), LHInstances.size(), 0, 0, 0); // Draw exactly one instance

    // End the pass and submit
    pass.End();
    wgpu::CommandBuffer cmd = encoder.Finish();
    device.GetQueue().Submit(1, &cmd);
}

void InitGraphics() {
  ConfigureSurface();
  mesh(device);

  std::string shaderCode;
    try {
        shaderCode = loadShaderSource("data/mesh.wgsl");
    } catch (const std::runtime_error& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        // Handle error, maybe exit or throw an exception.
        exit(1);
    }
    
    
  // LAYOUT SETUP
  wgpu::BindGroupLayoutEntry bglEntry{
      .binding = 0,
      .visibility = wgpu::ShaderStage::Vertex,
      .buffer.type = wgpu::BufferBindingType::Uniform,
  };
 
  wgpu::BindGroupLayoutDescriptor bglDesc{
      .entryCount = 1,
      .entries = &bglEntry
  };
  wgpu::BindGroupLayout cameraBindGroupLayout = device.CreateBindGroupLayout(&bglDesc);

  // wgpu::BindGroupLayoutEntry mDataBglEntry{
  //     .binding = 0,
  //     .visibility = wgpu::ShaderStage::Vertex,
  //     .buffer.type = wgpu::BufferBindingType::Uniform,
  // };
  // wgpu::BindGroupLayoutDescriptor mDataDesc{
  //     .entryCount = 1,
  //     .entries = &mDataBglEntry
  // };
  // wgpu::BindGroupLayout mDataGroupLayout = device.CreateBindGroupLayout(&mDataDesc);

  std::vector<wgpu::BindGroupLayout> bindGroupLayouts = {cameraBindGroupLayout};

  wgpu::PipelineLayoutDescriptor layoutDesc{
      .bindGroupLayoutCount = static_cast<uint32_t>(bindGroupLayouts.size()),
      .bindGroupLayouts = bindGroupLayouts.data()
  };
  wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&layoutDesc);


  PipelineConfig MeshConfig{};
  MeshConfig.surfaceFormat = format;
  MeshConfig.layout = pipelineLayout;
  try {
      pipeline = Pipeline(device, MeshConfig, shaderCode).getPipeline();
  } catch (const std::runtime_error& e) {
      std::cerr << "Pipeline creation failed: " << e.what() << std::endl;
  }
  if (!pipeline) {
      std::cerr << "Pipeline is invalid!" << std::endl;
      exit(1);
  }

  // BUFFER SETUP
  vertexBuffer = mesh.getVertexBuffer();
  uniformBuffer = BufferUtils::createUniformBuffer(device, sizeof(glm::mat4));
  // std::cout << "Size of MData: " << sizeof(MData) << " bytes" << std::endl;
  // mDataUniformBuffer = BufferUtils::createUniformBuffer(device, sizeof(MData));
  
  // BIND GROUP SETUP
  wgpu::BindGroupEntry bgEntry{};
  bgEntry.binding = 0; // Corresponds to @binding(0) in shader
  bgEntry.buffer = uniformBuffer;
  bgEntry.offset = 0;
  bgEntry.size = sizeof(glm::mat4);

  wgpu::BindGroupDescriptor bgDesc{
    .layout = cameraBindGroupLayout,
    .entryCount = 1,
    .entries = &bgEntry
  };
  uniformBindGroup = device.CreateBindGroup(&bgDesc);

  int i=1;  
  int mm=m-1;
   
  int x=0;
  int xx=0;
  int y=-1;
  int topl;
  int botl;
  int midt;
  int midb;
  int midl;
  int midr;
  int right;
  int left;
  std::vector<glm::vec4> colors = {
        {1.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.4f, 1.0f},
        {0.8f, 0.5f, 1.0f, 1.0f},
        {0.8f, 0.3f, 0.4f, 1.0f},
        {0.7f, 0.2f, 0.0f, 1.0f},
        {0.0f, 0.2f, 0.4f, 1.0f},
        {0.8f, 0.3f, 0.4f, 1.0f},
        {0.7f, 0.2f, 0.0f, 1.0f},
        {1.0f, 0.2f, 0.4f, 1.0f},
        {0.0f, 1.0f, 0.4f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
    

    
    float scale=1;
    

    std::vector<InstanceData> instancesLOD ={
        { { mm*3+1,mm+1 }, scale, 0 , colors[8]},
        { { mm*3+1,mm*2+1 }, scale, 1 , colors[9]},
        { { mm*2+1, mm+1 }, scale, 1 , colors[10]},
        { { mm*2+1, mm*2+1 }, scale, 1 , colors[11]},
        
        // { { -2.0f * m,  1.5f * m }, 1, 1 , glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
        
    };
    
    instances.insert(instances.end(), instancesLOD.begin(), instancesLOD.end());

    InstanceData LInstance={{mm*3+2,mm},scale,0,colors[12]};
    LHInstances.push_back(LInstance);
    LInstance={{mm,mm+1},scale,0,colors[12]};
    LInstances.push_back(LInstance);
    for(i=1;i<12;i++){
      x+=mm*scale;
      xx+=mm*scale;
      if(i%2==0)
        x+=scale;
      else{
        y+=scale;
      }
      
        topl=mm*3+2+x;
        botl=2*mm-xx+mm*(scale-1)-y;
        midt=mm*2+2+x-mm*(scale-1);
        midb=mm*2-y;
        midl= mm*2+mm*(scale-1)-x;
        midr= mm*2+2+y;
        right=mm*3+2+mm*(scale-1)+y;
        left=mm-x;
        if(i%2==0){
          InstanceData LHInstance = { {midt, midl}, scale, 0, colors[0] }; // Made it white to be visible
              LHInstances.push_back(LHInstance);

          InstanceData LInstance = { {botl,midl+scale}, scale, 0, colors[2] }; // Made it white to be visible
          LInstances.push_back(LInstance);
        }
        else{
          InstanceData LHInstance = { {botl+scale, midl}, scale, 0, colors[0] };
          LHInstances.push_back(LHInstance);

          InstanceData LInstance = { {botl,right}, scale, 0, colors[2] };
          LInstances.push_back(LInstance);

        }

        InstanceData rfuInstance = { {topl, midr-scale*2}, scale, 0, colors[1] };
        rfuInstances.push_back(rfuInstance);
        rfuInstance = { {botl, midr-scale*2}, scale, 0, colors[1] };
        rfuInstances.push_back(rfuInstance);

        InstanceData rfuHInstance = { {midb+scale*2, left}, scale, 0, colors[12] };
        rfuHInstances.push_back(rfuHInstance);
        rfuHInstance = { {midb+scale*2, right}, scale, 0, colors[12] };
        rfuHInstances.push_back(rfuHInstance);
        // std::cout<<"x: "<<x<<" y: "<<y<<" scale: "<<scale<<std::endl;

      
      
        std::vector<InstanceData> instancesLOD = {
        // --- Top row of blocks ---
        { { topl, left }, scale, 0 , colors[0]},
        { { topl,midl}, scale, 1 , colors[1]},
        { { topl, midr }, scale, 1 , colors[2]},
        { { topl, right }, scale, 1 , colors[3]},

        { { midt, left }, scale, 0 , colors[4]},
        { { midt, right}, scale, 1 , colors[5]},

        { { midb, left  }, scale, 0 , colors[6]},
        { { midb, right}, scale, 1 , colors[7]},

        { { botl, left}, scale, 0 , colors[8]},
        { { botl,midl }, scale, 1 , colors[9]},
        { { botl, midr }, scale, 1 , colors[10]},
        { { botl, right }, scale, 1 , colors[11]},
        
        // { { -2.0f * m,  1.5f * m }, 1, 1 , glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
        
    };
      scale*=2;
      instances.insert(instances.end(), instancesLOD.begin(), instancesLOD.end());
  }
    // Create (or reuse) the instance buffer
    if (!instanceBuffer) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(InstanceData) * instances.size();
        instanceBuffer = device.CreateBuffer(&desc);
    }
    device.GetQueue().WriteBuffer(instanceBuffer, 0, instances.data(), sizeof(InstanceData) * instances.size());

     // Assume this is a member variable like instanceBuffer
    if (!rfuInstanceBuffer) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(InstanceData)
        * rfuInstances.size();
        rfuInstanceBuffer = device.CreateBuffer(&desc);
    }

    device.GetQueue().WriteBuffer(rfuInstanceBuffer, 0, rfuInstances.data(), sizeof(InstanceData) * rfuInstances.size());

    if (!rfuHInstanceBuffer) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(InstanceData)
        * rfuHInstances.size();
        rfuHInstanceBuffer = device.CreateBuffer(&desc);
    }

    device.GetQueue().WriteBuffer(rfuHInstanceBuffer, 0, rfuHInstances.data(), sizeof(InstanceData) * rfuHInstances.size());


    if (!LInstanceBuffer) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(InstanceData)* LInstances.size();
        LInstanceBuffer = device.CreateBuffer(&desc);
    }
    device.GetQueue().WriteBuffer(LInstanceBuffer, 0, LInstances.data(), sizeof(InstanceData) * LInstances.size());
    if (!LHInstanceBuffer) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        desc.size = sizeof(InstanceData)* LHInstances.size();
        LHInstanceBuffer = device.CreateBuffer(&desc);
    }
    device.GetQueue().WriteBuffer(LHInstanceBuffer, 0, LHInstances.data(), sizeof(InstanceData) * LHInstances.size());

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
    surface.Present();
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