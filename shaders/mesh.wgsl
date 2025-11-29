// shaders/terrain.wgsl

// --- UNIFORMS & BINDINGS (Keep as is) ---
struct CameraUniforms {
    view_projection_matrix: mat4x4<f32>,
    world_position: vec4<f32>,
};

struct TerrainUniforms {
    heightScale: f32,
    terrainWidth: f32,
    terrainHeight: f32,
};

@group(0) @binding(0) var<uniform> camera: CameraUniforms;

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

// --- FIX 1: UPDATE OUTPUT STRUCT ---
struct VSOutput {
    @builtin(position) position : vec4f,
    @location(0) color : vec4f,
    
    // We need to pass these to the fragment shader to calculate lighting!
    @location(1) uv : vec2f,
    @location(2) @interpolate(flat) level : u32, 
};

@vertex
fn vertexMain(input: VertexInput) -> VSOutput {
    var out : VSOutput;

    // 1. Grid Logic (Keep exactly as you had it)
    let unitsPerVertex = 30.0; 
    let gridStep = unitsPerVertex; 
    let snappedCameraXZ = floor(camera.world_position.xz / gridStep) * gridStep;
    let local_mesh_pos = (input.position.xz * input.scale + input.offset) * unitsPerVertex;
    let world_xz = snappedCameraXZ + local_mesh_pos;

    // 2. UV Calculation
    let uv = (world_xz / vec2f(terrainData.terrainWidth, terrainData.terrainHeight)) + vec2f(0.5, 0.5);

    // 3. Height Sampling
    let height = textureSampleLevel(lodTextures, lodSampler, uv, input.level, 0.0f).r;

    // 4. Position Output
    let final_world_pos = vec3f(
        world_xz.x,
        height - 1100.0f, 
        world_xz.y
    );

    out.position = camera.view_projection_matrix * vec4f(final_world_pos, 1.0);
    out.color = input.color;
    
    // --- FIX 2: PASS DATA TO FRAGMENT SHADER ---
    out.uv = uv;
    out.level = input.level;

    return out;
}

@fragment
fn fragmentMain(in: VSOutput) -> @location(0) vec4f {
    // --- LIGHTING SETUP ---
    // Light coming from top-right-ish
    let lightDir = normalize(vec3f(0.5, 1.0, -0.5));
    
    // Base colors
    let grassColor = vec3f(0.1, 0.5, 0.2);
    let rockColor = vec3f(0.4, 0.35, 0.3);

    // --- NORMAL CALCULATION (The Important Part) ---
    // We sample the heightmap 3 times to determine the slope.
    
    // 1. How big is one pixel in the texture?
    let texDim = vec2f(textureDimensions(lodTextures));
    let pixelStep = 1.0 / texDim.x; 

    // 2. Sample Height at Center, Right, and Up
    // We use the same LOD level as the geometry to prevent lighting artifacts
    let h_C = textureSampleLevel(lodTextures, lodSampler, in.uv, in.level, 0.0).r;
    let h_R = textureSampleLevel(lodTextures, lodSampler, in.uv + vec2f(pixelStep, 0.0), in.level, 0.0).r;
    let h_U = textureSampleLevel(lodTextures, lodSampler, in.uv + vec2f(0.0, pixelStep), in.level, 0.0).r;

    // 3. Calculate Slope (Rise over Run)
    let slopeX = (h_R - h_C) * terrainData.heightScale;
    let slopeZ = (h_U - h_C) * terrainData.heightScale;

    // 4. Construct Normal Vector
    // "2.0" is a smoothing factor. 
    // - Increase to 4.0 for very smooth/soft hills.
    // - Decrease to 1.0 for sharp/rocky terrain.
    // We scale by (unitsPerVertex) conceptually to handle the 30m grid spacing
    let normal = normalize(vec3f(-slopeX, 2.0, -slopeZ));

    // --- LIGHTING MATH ---
    let NdotL = max(dot(normal, lightDir), 0.0);
    
    // Ambient light (Sky color)
    let ambient = vec3f(0.2, 0.25, 0.35);

    // Simple Texture: Mix grass and rock based on slope
    // If the normal points straight up (y > 0.9), it's grass. If steep, it's rock.
    let slopeFactor = normal.y; 
    let terrainColor = rockColor ;
    
    // Mix with your Debug Color (input.color) if you want to keep seeing the LOD rings
    // let finalBaseColor = terrainColor * in.color.rgb; // Visualize LODs + Texture
    let finalBaseColor = terrainColor; // Just Texture

    let finalColor = finalBaseColor * (ambient + vec3f(NdotL));

    return vec4f(finalColor, 1.0);
}