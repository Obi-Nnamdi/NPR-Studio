#version 330 core

uniform mat4 model_matrix;
uniform mat3 normal_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_tex_coord;

out vec3 world_position;
out vec3 world_normal;
out vec2 tex_coord;

void main() {
    world_position = vec3(model_matrix * 
        vec4(vertex_position, 1.0));
    world_normal = normal_matrix * vertex_normal;

    tex_coord = vertex_tex_coord;
    gl_Position = projection_matrix * view_matrix * vec4(world_position, 1.0);
}
