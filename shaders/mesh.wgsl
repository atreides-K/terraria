// 1. Update Camera Uniforms to include position
struct CameraUniforms {
    view_projection_matrix: mat4x4<f32>,
    world_position: vec4<f32>, // .xyz = position, .w = padding
};

struct TerrainUniforms {
    heightScale: f32,
    terrainWidth: f32,
    terrainHeight: f32,
};

@group(0) @binding(0)
var<uniform> camera: CameraUniforms;

@group(1) @binding(0) var lodTextures: texture_2d_array<f32>;
@group(1) @binding(1) var lodSampler: sampler;
@group(1) @binding(2) var<uniform> terrainData: TerrainUniforms;

struct VertexInput {
    @location(0) position : vec3f,
    @location(1) offset   : vec2f,
    @location(2) scale    : f32, 
    @location(3) level    : u32,
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

    // --- LOGIC CHANGE START ---

    // 1. Determine the size of one grid cell (triangle edge) in meters for THIS instance.
    // You had hardcoded 30.0, so we keep it. Ideally pass this as uniform.
    let unitsPerVertex = 30.0; 
    let gridStep = unitsPerVertex; 

    // 2. "Snap" the camera position to the nearest grid line.
    // This prevents the "swimming" artifact. The grid jumps in 30m (or 60m, 120m) increments.
    let snappedCameraXZ = floor(camera.world_position.xz / gridStep) * gridStep;

    // 3. Calculate the local position of this vertex relative to the center
    // input.position.xz is 0..N
    // input.offset is the relative offset of the ring patch
    let local_mesh_pos = (input.position.xz * input.scale + input.offset) * unitsPerVertex;

    // 4. Final World XZ = Snapped Camera + Local Mesh Position
    let world_xz = snappedCameraXZ + local_mesh_pos;

    // --- LOGIC CHANGE END ---


    // Normalize world coordinates for UV (0.0 to 1.0)
    let uv = (world_xz / vec2f(terrainData.terrainWidth, terrainData.terrainHeight)) + vec2f(0.5, 0.5);

    // Sample Height
    let height = textureSampleLevel(lodTextures, lodSampler, uv, input.level, 0.0f).r;

    // Construct Final Position
    let final_world_pos = vec3f(
        world_xz.x,
        height - 1100.0f, // Adjust this based on your DEM's base height
        world_xz.y
    );

    out.position = camera.view_projection_matrix * vec4f(final_world_pos, 1.0);
    out.color = input.color;

    return out;
}

@fragment
fn fragmentMain(input: VSOutput) -> @location(0) vec4f {
    return vec4f(input.color);
}