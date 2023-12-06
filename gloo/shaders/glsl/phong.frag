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
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    sampler2D diffuse_sampler;
    sampler2D specular_sampler;
    sampler2D ambient_sampler;
    float shininess;
};

in vec3 world_position;
in vec3 world_normal;
in vec2 tex_coord;
// Controls the use of textures on each shading component
uniform bool diffuse_use_texture;
uniform bool specular_use_texture;
uniform bool ambient_use_texture;

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

vec3 GetAmbientColor() {
    if (ambient_use_texture) {
        return texture(material.ambient_sampler, tex_coord).rgb;
    }
    return material.ambient;
}

vec3 GetDiffuseColor() {
    if (diffuse_use_texture) {
        return texture(material.diffuse_sampler, tex_coord).rgb;
    }
    return material.diffuse;
}

vec3 GetSpecularColor() {
    if (specular_use_texture) {
        return texture(material.specular_sampler, tex_coord).rgb;
    }
    return material.specular;
}

vec3 CalcAmbientLight() {
    return ambient_light.ambient * GetAmbientColor();
}

vec3 CalcPointLight(vec3 normal, vec3 view_dir) {
    PointLight light = point_light;
    vec3 light_dir = normalize(light.position - world_position);

    float diffuse_intensity = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = diffuse_intensity * light.diffuse * GetDiffuseColor();

    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_intensity = pow(
        max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_color = specular_intensity * 
        light.specular * GetSpecularColor();

    float distance = length(light.position - world_position);
    float attenuation = 1.0 / (light.attenuation.x + 
        light.attenuation.y * distance + 
        light.attenuation.z * (distance * distance));

    return attenuation * (diffuse_color + specular_color);
}

vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir) {
    // Do check for shadows
    float shadow = 0;
    // Get normalized device coordinates of world position in light that range from [-1, 1]
    vec3 light_ndc_point_pos = vec3(directional_light.world_to_light_ndc_matrix * vec4(world_position, 1.0));
    
    // Convert ndc to texture coordinates that range from [0, 1]
    vec3 light_tex_point_pos = 0.5 * light_ndc_point_pos + 0.5;
    
    // Get Depth of current point and the occluder in the shadow map texture
    float point_depth = light_tex_point_pos.z;
    // Do percentage closer filtering
    // Algorithm adapted from LearnOpenGL: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    int pcf_size = 2; // 5x5 filter
    vec2 texelSize = 1.0 / textureSize(directional_light.shadow_map, 0);
    // Sample a square of points around fragment
    for (int x = -pcf_size; x <= pcf_size; x++) {
        for (int y = -pcf_size; y <= pcf_size; y++) {
            // Check if point is in shadow
            float occluder_depth = texture(directional_light.shadow_map, light_tex_point_pos.xy + vec2(x, y) * texelSize).r;
            if (occluder_depth + directional_light.shadow_bias < point_depth) {
                shadow += 1; // No color from directional light at that point
            }
        }   
    }
    shadow /= pow((pcf_size * 2 + 1), 2);

    // Shade normally
    // Note: I don't use a `light` local variable here because OpenGL gets mad
    // if a sampler2D (or any opaque type) is part of a struct that's 
    // used in a non-uniform way
    vec3 light_dir = normalize(-directional_light.direction);
    float diffuse_intensity = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = diffuse_intensity * directional_light.diffuse * GetDiffuseColor();

    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_intensity = pow(
        max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_color = specular_intensity * 
        directional_light.specular * GetSpecularColor();

    vec3 final_color = diffuse_color + specular_color;
    return (1 - shadow) * final_color;
}

