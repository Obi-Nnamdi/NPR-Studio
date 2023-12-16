#version 330 core

out vec4 frag_color;

uniform vec3 material_color; 

void main() {
    frag_color = vec4(material_color, 1.0);
}
