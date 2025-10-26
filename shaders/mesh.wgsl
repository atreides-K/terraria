// Camera uniform stays the same
struct CameraUniforms {
    view_projection_matrix: mat4x4<f32>,
};
@group(0) @binding(0)
var<uniform> camera: CameraUniforms;

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

    // Apply per-instance transform
    let pos = vec3f(
        (input.position.x * input.scale + input.offset.x)*0.2,
        input.position.y,
        (input.position.z * input.scale + input.offset.y)*0.2
    );

    out.position = camera.view_projection_matrix * vec4f(pos, 1.0);

    // Simple color based on instance index
    let hue = f32(input.instanceIdx) / 4.0; // assuming 4 instances
    out.color = input.color;

    return out;
}

@fragment
fn fragmentMain(input: VSOutput) -> @location(0) vec4f {
    return vec4f(input.color);
}
