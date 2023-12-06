#include "PhongShader.hpp"

#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>

#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/DirectionalLight.hpp"

namespace GLOO {
PhongShader::PhongShader()
    : ShaderProgram(std::unordered_map<GLenum, std::string>{
          {GL_VERTEX_SHADER, "phong.vert"},
          {GL_FRAGMENT_SHADER, "phong.frag"}}) {
}

void PhongShader::AssociateVertexArray(VertexArray& vertex_array) const {
  if (!vertex_array.HasPositionBuffer()) {
    throw std::runtime_error("Phong shader requires vertex positions!");
  }
  if (!vertex_array.HasNormalBuffer()) {
    throw std::runtime_error("Phong shader requires vertex normals!");
  }
  vertex_array.LinkPositionBuffer(GetAttributeLocation("vertex_position"));
  vertex_array.LinkNormalBuffer(GetAttributeLocation("vertex_normal"));
  if (vertex_array.HasTexCoordBuffer()) {
    GLint loc = GetAttributeLocation("vertex_tex_coord");
    if (loc != -1) {
      vertex_array.LinkTexCoordBuffer(loc);
    }
  }
}

void PhongShader::SetTargetNode(const SceneNode& node,
                                const glm::mat4& model_matrix) const {
  // Associate the right VAO before rendering.
  AssociateVertexArray(node.GetComponentPtr<RenderingComponent>()
                           ->GetVertexObjectPtr()
                           ->GetVertexArray());

  // Set transform.
  glm::mat3 normal_matrix =
      glm::transpose(glm::inverse(glm::mat3(model_matrix)));
  SetUniform("model_matrix", model_matrix);
  SetUniform("normal_matrix", normal_matrix);

  // Set material.
  MaterialComponent* material_component_ptr =
      node.GetComponentPtr<MaterialComponent>();
  const Material* material_ptr;
  if (material_component_ptr == nullptr) {
    material_ptr = &Material::GetDefault();
  } else {
    material_ptr = &material_component_ptr->GetMaterial();
  }
  SetUniform("material.ambient", material_ptr->GetAmbientColor());
  SetUniform("material.diffuse", material_ptr->GetDiffuseColor());
  SetUniform("material.specular", material_ptr->GetSpecularColor());
  SetUniform("material.shininess", material_ptr->GetShininess());

  // Set material textures and bind them to their respective units
  auto diffuse_texture = material_ptr->GetDiffuseTexture();
  auto specular_texture = material_ptr->GetSpecularTexture();
  auto ambient_texture = material_ptr->GetAmbientTexture();

  // Disable texture use by default
  SetUniform("diffuse_use_texture", false);
  SetUniform("specular_use_texture", false);
  SetUniform("ambient_use_texture", false);

  // Bind component textures to their respective units
  // and set up component texture samplers if they're defined.
  if (diffuse_texture != nullptr) {
    diffuse_texture->BindToUnit(diffuse_texture_unit);
    SetUniform("diffuse_use_texture", true);
    SetUniform("material.diffuse_sampler", diffuse_texture_unit);
  }

  if (specular_texture != nullptr) {
    specular_texture->BindToUnit(specular_texture_unit);
    SetUniform("specular_use_texture", true);
    SetUniform("material.specular_sampler", specular_texture_unit);
  }

  if (ambient_texture != nullptr) {
    ambient_texture->BindToUnit(ambient_texture_unit);
    SetUniform("ambient_use_texture", true);
    SetUniform("material.ambient_sampler", ambient_texture_unit);
  }
}

void PhongShader::SetCamera(const CameraComponent& camera) const {
  SetUniform("view_matrix", camera.GetViewMatrix());
  SetUniform("projection_matrix", camera.GetProjectionMatrix());
  // TODO: change camera position calculation to be in line with Piazza post?
  SetUniform("camera_position",
             camera.GetNodePtr()->GetTransform().GetWorldPosition());
}

void PhongShader::SetLightSource(const LightComponent& component) const {
  auto light_ptr = component.GetLightPtr();
  if (light_ptr == nullptr) {
    throw std::runtime_error("Light component has no light attached!");
  }

  // First disable all lights.
  // In a single rendering pass, only one light of one type is enabled.
  SetUniform("ambient_light.enabled", false);
  SetUniform("point_light.enabled", false);
  SetUniform("directional_light.enabled", false);

  if (light_ptr->GetType() == LightType::Ambient) {
    auto ambient_light_ptr = static_cast<AmbientLight*>(light_ptr);
    SetUniform("ambient_light.enabled", true);
    SetUniform("ambient_light.ambient", ambient_light_ptr->GetAmbientColor());
  } else if (light_ptr->GetType() == LightType::Point) {
    auto point_light_ptr = static_cast<PointLight*>(light_ptr);
    SetUniform("point_light.enabled", true);
    SetUniform("point_light.position",
               component.GetNodePtr()->GetTransform().GetPosition());
    SetUniform("point_light.diffuse", point_light_ptr->GetDiffuseColor());
    SetUniform("point_light.specular", point_light_ptr->GetSpecularColor());
    SetUniform("point_light.attenuation", point_light_ptr->GetAttenuation());
  } else if (light_ptr->GetType() == LightType::Directional) {
    auto directional_light_ptr = static_cast<DirectionalLight*>(light_ptr);
    SetUniform("directional_light.enabled", true);
    SetUniform("directional_light.direction",
               directional_light_ptr->GetDirection());
    SetUniform("directional_light.diffuse",
               directional_light_ptr->GetDiffuseColor());
    SetUniform("directional_light.specular",
               directional_light_ptr->GetSpecularColor());
  } else {
    throw std::runtime_error(
        "Encountered light type unrecognized by the shader!");
  }
}

void PhongShader::SetShadowMapping(const Texture& shadow_texture,
                                   const glm::mat4& world_to_light_ndc_matrix) const {
  // Bind shadow map texture to correct unit
  shadow_texture.BindToUnit(shadow_map_unit);
  // Set uniforms
  SetUniform("directional_light.world_to_light_ndc_matrix", world_to_light_ndc_matrix);
  SetUniform("directional_light.shadow_map", shadow_map_unit);
  SetUniform("directional_light.shadow_bias", shadow_bias_);
}
}  // namespace GLOO
