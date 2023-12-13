#version 330 core

out vec4 frag_color;

struct AmbientLight {
    bool enabled;
    vec3 ambient;
};

struct PointLight {
    bool enabled;
    vec3 position;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

struct DirectionalLight {
    bool enabled;
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
    mat4 world_to_light_ndc_matrix;
    sampler2D shadow_map;
    float shadow_bias;
};
struct Material {
    vec3 low_color;
    vec3 high_color;
};

in vec3 world_position;
in vec3 world_normal;
in vec2 tex_coord;

uniform vec3 camera_position;

uniform Material material; // material properties of the object
uniform AmbientLight ambient_light;
uniform PointLight point_light; 
uniform DirectionalLight directional_light;
vec3 CalcAmbientLight();
vec3 CalcPointLight(vec3 normal, vec3 view_dir);
vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir);

void main() {
    vec3 normal = normalize(world_normal);
    vec3 view_dir = normalize(camera_position - world_position);

    frag_color = vec4(0.0);
    // TODO: test difference between performing tone mapping in individual light calculations 
    // vs as a simple mix in final color

    if (ambient_light.enabled) {
        frag_color += vec4(CalcAmbientLight(), 1.0);
    }
    
    if (point_light.enabled) {
        frag_color += vec4(CalcPointLight(normal, view_dir), 1.0);
    }

    if (directional_light.enabled) {
        frag_color += vec4(CalcDirectionalLight(normal, view_dir), 1.0);
    }
}


float desaturate(vec3 color) {
    return 0.3 * color.r + 0.59 * color.g + 0.11 * color.b;
}

vec3 GetAmbientColor() {
    return vec3(0.0);
}

vec3 GetDiffuseColor() {
    return vec3(0.0);
}

vec3 GetSpecularColor() {
    return vec3(0.0);
}

vec3 GetHighColor() {
    return material.high_color;
}

vec3 GetLowColor() {
    return material.low_color;
}

vec3 CalcAmbientLight() {
    return mix(GetLowColor(), GetHighColor(), desaturate(ambient_light.ambient));
    
}

vec3 CalcPointLight(vec3 normal, vec3 view_dir) {
    PointLight light = point_light;
    vec3 light_dir = normalize(light.position - world_position);
    // TODO: handle tone mapping for point lights (and multiple lights in general lol)
    // Replace `GetDiffuseColor` here for tone mapping (between cool and warm color)
    // Try multiple options for getting point lights to work lol, the goal is to keep 
    // the final illumination the same b/t 

    float distance = length(light.position - world_position);
    float attenuation = 1.0 / (light.attenuation.x + 
        light.attenuation.y * distance + 
        light.attenuation.z * (distance * distance));

    float lambertian_term = dot(normal, light_dir); // from [-1, 1]
    float color_mix_factor = attenuation * (1 + lambertian_term) / 2; // from [0, 1]
    vec3 tone_color = color_mix_factor * GetHighColor() + (1 - color_mix_factor) * GetLowColor();

    return tone_color;
}

vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir) {
    // Shade using Tone Mapping
    vec3 light_dir = normalize(-directional_light.direction);
    float lambertian_term = dot(normal, light_dir); // from [-1, 1]
    // TODO Delete this:
    // float color_mix_factor = step(0.5, (1 + lambertian_term) / 2); // from [0, 1]
    float color_mix_factor =(1 + lambertian_term) / 2; // from [0, 1]
    // Tone mapping equation. Adapted from Gooch et al. (1998):
    // TODO: try both (they're the same)
    // vec3 tone_color = mix(GetLowColor(), GetHighColor(), color_mix_factor);
    vec3 tone_color = color_mix_factor * GetHighColor() + (1 - color_mix_factor) * GetLowColor();

    return tone_color;
}

