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
    float shininess;
    float specular_intensity;
    float diffuse_intensity;
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
    // TODO (large): refactor rendering pipeline to allow for all lights to be rendered at once, so 
    // general tone mapping can work with multiple lights

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

float GetSpecularIntensity() {
    return material.specular_intensity;
}

float GetDiffuseIntensity() {
    return material.diffuse_intensity;
}

float GetShininess() {
    return material.shininess;
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
    
    // Matte shading
    float lambertian_term = dot(normal, light_dir); // from [-1, 1]
    float diffuse_term = GetDiffuseIntensity() * ((1 + lambertian_term) / 2);
    
    // Specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_term = GetSpecularIntensity() * pow(max(dot(view_dir, reflect_dir), 0.0), GetShininess());

    // Add in point light attenuation
    float distance = length(light.position - world_position);
    float attenuation = 1.0 / (light.attenuation.x + 
        light.attenuation.y * distance + 
        light.attenuation.z * (distance * distance));

    float color_mix_factor = clamp(attenuation * (diffuse_term + specular_term), 0, 1); // clamp from [0, 1]
    vec3 tone_color = color_mix_factor * GetHighColor() + (1 - color_mix_factor) * GetLowColor();

    return tone_color;
}

vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir) {
    // Shade using Tone Mapping
    vec3 light_dir = normalize(-directional_light.direction);
    // Matte shading
    float lambertian_term = dot(normal, light_dir); // from [-1, 1]
    float diffuse_term = GetDiffuseIntensity() * ((1 + lambertian_term) / 2);
    
    // Specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_term = GetSpecularIntensity() * pow(max(dot(view_dir, reflect_dir), 0.0), GetShininess());

    // Tone mapping equation. Adapted from Gooch et al. (1998):
    // Use color mix factor to mix between high and low color:
    float color_mix_factor = clamp(diffuse_term + specular_term, 0, 1); // clamp from [0, 1]
    vec3 tone_color = color_mix_factor * GetHighColor() + (1 - color_mix_factor) * GetLowColor();

    return tone_color;
}

