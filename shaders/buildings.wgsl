// shaders/buildings.wgsl

// Uniform buffer for camera matrices. This part was already correct.
struct Camera {
    viewProj : mat4x4<f32>,
};

@group(0) @binding(0)
var<uniform> camera : Camera;

// --- FIX #1: Define the Vertex Input Structure ---
// This struct tells the shader what data to expect for each vertex.
// The '@location(0)' MUST match the '.shaderLocation = 0' from your C++ code.
// The 'position: vec3<f32>' MUST match the '.format = wgpu::VertexFormat::Float32x3'.
struct VertexInput {
    @location(0) position: vec3<f32>,
};

// --- FIX #2: Define the Vertex Output Structure ---
// This struct defines the data being passed from the vertex shader
// to the fragment shader. The '@builtin(position)' is a special
// required attribute for the final clip space position.
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

// The vertex shader entry point. Now it uses the correctly defined structs.
@vertex
fn vertexMain(input: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    // We transform the 3D model-space position into a 4D clip-space position.
    out.position = camera.viewProj * vec4<f32>(input.position, 1.0);
    return out;
}

// The fragment shader entry point. This part was already correct.
@fragment
fn fragmentMain() -> @location(0) vec4<f32> {
    // Return a solid red color for all pixels.
    return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}