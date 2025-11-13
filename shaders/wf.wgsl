// Camera uniform stays the same
struct CameraUniforms {
    view_projection_matrix: mat4x4<f32>,
};

struct TerrainUniforms{
    heightScale: f32,
    terrainWidth: f32,
    terrainHeight: f32,
};

@group(0) @binding(0)
var<uniform> camera: CameraUniforms;

@group(1) @binding(0) var lodTextures: texture_2d_array<f32>;
@group(1) @binding(1) var lodSampler: sampler;
@group(1) @binding(2) var<uniform> terrainData: TerrainUniforms;

// Per-instance data now comes from vertex attributes
struct VertexInput {
    @location(0) position : vec3f,    // mesh vertex
    @location(1) offset   : vec2f,    // instance offset
    @location(2) scale    : f32,      // instance scale
    @location(3) level    : u32,      // optional
    @location(4) color: vec4<f32>, 
    @builtin(instance_index) instanceIdx : u32,
};

struct VSOutput {
    @builtin(position) position : vec4f,
    @location(0) color : vec4f,
};

@vertex
fn vertexMain(input: VertexInput) -> VSOutput {
    var out : VSOutput;

    
    let world_xz = (input.position.xz * input.scale + input.offset);

// Normalize the world coordinates and shift the origin to the center of the texture
    let uv = (world_xz  / vec2f(terrainData.terrainWidth, terrainData.terrainHeight)) + vec2f(0.5, 0.5);

    let height=textureSampleLevel(lodTextures, lodSampler, uv, input.level,0.0f).r;
    // Apply per-instance transform
     let final_world_pos = vec3f(
        world_xz.x,
        height * 0.1,
        world_xz.y
    );

    out.position = camera.view_projection_matrix * vec4f(final_world_pos, 1.0);

    // Simple color based on instance index
    // let hue = f32(input.instanceIdx) / 4.0; // assuming 4 instances
    out.color = input.color;

    return out;
}

@fragment
fn fragmentMain(input: VSOutput) -> @location(0) vec4f {
    return vec4f(input.color);
}
