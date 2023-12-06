#version 330 core

in vec3 vertex_ndc_position;
in vec2 vertex_tex_coord;

out vec2 tex_coord;

void main() {
    gl_Position = vec4(vertex_ndc_position, 1.0);
    tex_coord = vertex_tex_coord;
}
