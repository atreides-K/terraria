  struct CameraUniforms {
        view_projection_matrix: mat4x4<f32>,
    };

    // Tell the shader where to find this uniform data
    @group(0) @binding(0)
    var<uniform> camera: CameraUniforms;

    struct VertexInput{
      @location(0) position : vec3f
    };

    @vertex fn vertexMain(input:VertexInput) ->
      @builtin(position) vec4f {
        // Use the matrix to transform the vertex position
        return camera.view_projection_matrix * vec4f(input.position, 1.0);
    }

    @fragment fn fragmentMain() -> @location(0) vec4f {
      return vec4f(1, 0.8, 0.4, 1.0); 
    }