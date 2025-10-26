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


// shader
#include <ShaderLoader.h>

std::map<std::string, bool> keyStates;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));




wgpu::Buffer uniformBuffer;
wgpu::BindGroup uniformBindGroup;

wgpu::Buffer modelUniformBuffer ;
wgpu::BindGroup modelBindGroup ;

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
    // 1. Standard Per-Frame Setup
    //----------------------------------------------------------------
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);


    
    wgpu::RenderPassColorAttachment attachment{
      .view = surfaceTexture.texture.CreateView(),
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = {0.1f, 0.2f, 0.3f, 1.0f} // A dark blue clear color
    };

    wgpu::RenderPassDescriptor renderpass{
        .colorAttachmentCount = 1,
        .colorAttachments = &attachment
    };

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);


    // 2. Set Global State for the Entire Pass
    //----------------------------------------------------------------
    pass.SetPipeline(pipeline);

    // Compute and set the scene-wide camera matrix (Bind Group 0)
    // This is done only ONCE per frame as it's the same for all objects.
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)kWidth / (float)kHeight, 0.1f, 100.0f);
    glm::mat4 viewProjMatrix = projection * view;
    device.GetQueue().WriteBuffer(uniformBuffer, 0, &viewProjMatrix, sizeof(glm::mat4));
    pass.SetBindGroup(0, uniformBindGroup);


    // 3. Assemble the Clipmap Ring, Piece by Piece
    //----------------------------------------------------------------

    // This 'm' value would come from your clipmap logic. We'll use a constant for this example.
    const int m = 10;
    // The size of a block in world units is determined by its vertex count and spacing.
    // If spacing is 1.0, a block of 'm' vertices has a size of 'm - 1'.
    const float blockSize = (m - 1.0f) * 1; // Assuming 'spacing' is a public member of Mesh


    // --- A. Draw the 12 main (m x m) blocks (Gray in diagram) ---
    const std::vector<glm::vec2> blockOffsets = {
        {-1.5f,  1.5f}, {-0.5f,  1.5f}, {0.5f,  1.5f}, {1.5f,  1.5f}, // Top Row
        {-1.5f, -1.5f}, {-0.5f, -1.5f}, {0.5f, -1.5f}, {1.5f, -1.5f}, // Bottom Row
        {-1.5f,  0.5f}, { 1.5f,  0.5f},                               // Middle Left/Right
        {-1.5f, -0.5f}, { 1.5f, -0.5f}
    };

    // Point the GPU to the vertex/index data for THIS type of mesh piece
    pass.SetVertexBuffer(0, mesh.getVertexBuffer(), 0, mesh.getVertexCount() * sizeof(Vertex));
    pass.SetIndexBuffer(mesh.getIndexBuffer(), wgpu::IndexFormat::Uint32, 0, mesh.getIndexCount() * sizeof(uint32_t));

    for (const auto& offset : blockOffsets) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(offset.x * blockSize, 0.0f, offset.y * blockSize));

        // Update the GPU buffer with the matrix for THIS specific block
        device.GetQueue().WriteBuffer(modelUniformBuffer, 0, &model, sizeof(glm::mat4));

        // Set Bind Group 1. This must be done for EACH draw call that uses a different model matrix.
        pass.SetBindGroup(1, modelBindGroup);

        pass.DrawIndexed(mesh.getIndexCount(), 1, 0, 0, 0);
    }


    // // --- B. Draw the 4 (m x 3) Ring Fix-up blocks (Green in diagram) ---
    // // The offset distance is half the full ring width minus half a block width.
    // // Full width = 4 blocks -> 2 * blockSize. Half width = blockSize.
    // const float fixupDist = 2.0f * blockSize - (blockSize / 2.0f);

    // // Transforms are {posX, posY, posZ, rotationY}
    // const std::vector<glm::vec4> fixupTransforms = {
    //     { 0.0f, 0.0f,  1.5f * blockSize,   0.0f}, // Top
    //     { 0.0f, 0.0f, -1.5f * blockSize, 180.0f}, // Bottom
    //     { 1.5f * blockSize, 0.0f,  0.0f,  -90.0f}, // Right
    //     {-1.5f * blockSize, 0.0f,  0.0f,   90.0f}  // Left
    // };

    // // Switch to the vertex/index data for the fix-up mesh
    // pass.SetVertexBuffer(0, mesh.getRfuVertexBuffer(), 0, mesh.getRfuVertexCount() * sizeof(Vertex));
    // pass.SetIndexBuffer(mesh.getRfuIndexBuffer(), wgpu::IndexFormat::Uint32, 0, mesh.getRfuIndexCount() * sizeof(uint32_t));

    // for (const auto& t : fixupTransforms) {
    //     glm::mat4 model = glm::mat4(1.0f);
    //     model = glm::translate(model, glm::vec3(t.x, t.y, t.z));
    //     model = glm::rotate(model, glm::radians(t.w), glm::vec3(0.0f, 1.0f, 0.0f));

    //     device.GetQueue().WriteBuffer(modelUniformBuffer, 0, &model, sizeof(glm::mat4));
    //     pass.SetBindGroup(1, modelBindGroup);
    //     pass.DrawIndexed(mesh.getRfuIndexCount(), 1, 0, 0, 0);
    // }

    // --- C. Draw the Interior Trim and other pieces ---
    // You would continue the pattern here:
    // 1. Define the transforms for the trim pieces.
    // 2. Call pass.SetVertexBuffer() and pass.SetIndexBuffer() with the trim mesh data.
    // 3. Loop through transforms, update model buffer, set bind group, and call DrawIndexed().
    // ...


    // 4. Finalize and Submit Commands
    //----------------------------------------------------------------
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    // For native applications that need to explicitly present
    #if !defined(__EMSCRIPTEN__)
        surface.Present();
    #endif
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

  wgpu::BindGroupLayoutEntry modelBglEntry{
      .binding = 0,
      .visibility = wgpu::ShaderStage::Vertex,
      .buffer.type = wgpu::BufferBindingType::Uniform,
  };
  wgpu::BindGroupLayoutDescriptor modelBglDesc{
      .entryCount = 1,
      .entries = &modelBglEntry
  };
  wgpu::BindGroupLayout modelBindGroupLayout = device.CreateBindGroupLayout(&modelBglDesc);

  std::vector<wgpu::BindGroupLayout> bindGroupLayouts = {cameraBindGroupLayout, modelBindGroupLayout};

  wgpu::PipelineLayoutDescriptor layoutDesc{
      .bindGroupLayoutCount = static_cast<uint32_t>(bindGroupLayouts.size()),
      .bindGroupLayouts = bindGroupLayouts.data()
  };
  wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&layoutDesc);


  PipelineConfig MeshConfig{};
  MeshConfig.surfaceFormat = format;
  MeshConfig.layout = pipelineLayout;
  pipeline=Pipeline(device,MeshConfig,shaderCode).getPipeline();


  // BUFFER SETUP
  vertexBuffer = mesh.getVertexBuffer();
  uniformBuffer = BufferUtils::createUniformBuffer(device, sizeof(glm::mat4));

  modelUniformBuffer = BufferUtils::createUniformBuffer(device, sizeof(glm::mat4));
  
  // BIND GROUP SETUP
  wgpu::BindGroupEntry bgEntry{};
  bgEntry.binding = 0; // Corresponds to @binding(0) in shader
  bgEntry.buffer = uniformBuffer;
  bgEntry.size = sizeof(glm::mat4);

  wgpu::BindGroupDescriptor bgDesc{
    .layout = cameraBindGroupLayout,
    .entryCount = 1,
    .entries = &bgEntry
  };
  uniformBindGroup = device.CreateBindGroup(&bgDesc);
  wgpu::BindGroupEntry modelBgEntry{
  .binding = 0,
  .buffer = modelUniformBuffer,
  .size = sizeof(glm::mat4)
  };
  wgpu::BindGroupDescriptor modelBgDesc{
    .layout = modelBindGroupLayout,
    .entryCount = 1,
    .entries = &modelBgEntry
  };
  modelBindGroup = device.CreateBindGroup(&modelBgDesc);

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

// This struct defines the format of the vertex data that we send from the CPU.
// It must match the layout of the `Vertex` struct in your C++ code.
struct VertexInput {
    // @location(0) corresponds to the first attribute in the vertex buffer layout.
    @location(0) position: vec3f,
};

// This struct holds the camera's view and projection matrices, combined into one.
// This data is the same for every object drawn in a single frame.
struct CameraUniforms {
    view_projection_matrix: mat4x4<f32>,
};

// NEW: This struct holds the per-object model matrix.
// This will change for each individual piece of the clipmap ring we draw.
struct ModelUniforms {
    model_matrix: mat4x4<f32>,
};

// This uniform variable will get its data from the buffer bound at group 0, binding 0.
// In C++, this is your `uniformBindGroup`.
@group(0) @binding(0)
var<uniform> camera: CameraUniforms;

// NEW: This uniform variable gets its data from the buffer bound at group 1, binding 0.
// In C++, this is your `modelBindGroup`. It now correctly expects a 4x4 matrix.
@group(1) @binding(0)
var<uniform> model: ModelUniforms;


@vertex
fn vertexMain(input: VertexInput) -> @builtin(position) vec4f {
    // 1. Convert the input vertex position (vec3f) to a homogeneous coordinate (vec4f).
    //    The 'w' component of 1.0 is used for position vectors.
    let model_position = vec4f(input.position, 1.0);

    // 2. Transform the vertex from its local model space into world space
    //    by multiplying it with the per-object model matrix.
    let world_position = model.model_matrix * model_position;

    // 3. Transform the world space position into final clip space
    //    by multiplying it with the camera's view-projection matrix.
    //    This is the final position that will be rendered.
    return camera.view_projection_matrix * world_position;
}

@fragment
fn fragmentMain() -> @location(0) vec4f {
  // Return a solid color for the fragment (e.g., a nice orange).
  return vec4f(1.0, 0.5, 0.0, 1.0);
}