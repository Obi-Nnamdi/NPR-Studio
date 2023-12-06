#version 330 core

uniform mat4 model_matrix;
uniform mat4 world_to_light_ndc_matrix;

layout(location = 0) in vec3 vertex_position;

void main() {
    vec3 world_position = vec3(model_matrix * vec4(vertex_position, 1.0));
    gl_Position = world_to_light_ndc_matrix * vec4(world_position, 1.0);
}
