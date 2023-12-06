#version 330 core

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

layout(location = 0) in vec3 vertex_position;

void main() {
    vec3 world_position = vec3(model_matrix * vec4(vertex_position, 1.0));
    gl_Position = projection_matrix * view_matrix * vec4(world_position, 1.0);
}
