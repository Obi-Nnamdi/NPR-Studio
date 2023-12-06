#version 330 core

out vec4 frag_color;

void main() {
    // Uniform green color.
    frag_color = vec4(vec3(0.0, 0.0, 1.0), 1.0);
    // Depth will be recorded automatically.
}
