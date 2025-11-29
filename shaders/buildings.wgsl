// shaders/buildings.wgsl

struct Camera {
    viewProj : mat4x4<f32>,
};

@group(0) @binding(0)
var<uniform> camera : Camera;

struct VertexInput {
    @location(0) position: vec3<f32>,
};

// --- FIX: Output Structure ---
struct VertexOutput {
    // 1. The Screen Position (Required by GPU to draw)
    @builtin(position) clip_position: vec4<f32>,
    
    // 2. The 3D World Position (Used for Math/Lighting)
    @location(0) world_pos: vec3<f32>,
};

@vertex
fn vertexMain(input: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    // A. Pass the raw 3D position to the pixel shader for lighting
    out.world_pos = input.position; 

    // B. Calculate where this point appears on the screen
    out.clip_position = camera.viewProj * vec4<f32>(input.position, 1.0);
    
    return out;
}

@fragment
fn fragmentMain(in: VertexOutput) -> @location(0) vec4<f32> {
    // --- LIGHTING CONSTANTS ---
    let lightDir = normalize(vec3<f32>(0.5, 1.0, -0.5)); // Sun Direction
    let objectColor = vec3<f32>(0.8, 0.2, 0.2);          // Brick Red
    let ambientColor = vec3<f32>(0.3, 0.3, 0.4);         // Shadow Color
    let sunColor = vec3<f32>(1.0, 0.9, 0.8);             // Sun Light Color

    // --- NORMAL CALCULATION ---
    // We use the WORLD position (meters), not the screen position.
    let dx = dpdx(in.world_pos);
    let dy = dpdy(in.world_pos);

    // Calculate the face normal
    // If the lit side is BLACK, swap this to: cross(dx, dy)
    let normal = normalize(cross(dy, dx)); 

    // --- LIGHTING MATH ---
    // Dot product determines how much the face points at the sun
    let NdotL = max(dot(normal, lightDir), 0.0);

    // Combine
    let lighting = ambientColor + (sunColor * NdotL);
    let finalColor = objectColor * lighting;

    return vec4<f32>(finalColor, 1.0);
}