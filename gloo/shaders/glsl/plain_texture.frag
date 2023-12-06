#version 330 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D in_texture;
uniform bool is_depth;

void main() {
    if (is_depth) {
        float depth = texture(in_texture, tex_coord).r;
        frag_color = vec4(vec3(depth), 1.0);
    } else {
        frag_color = vec4(texture(in_texture, tex_coord).rgb, 1.0);
    }
}
