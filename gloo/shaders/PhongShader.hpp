#ifndef GLOO_PHONG_SHADER_H_
#define GLOO_PHONG_SHADER_H_

#include "ShaderProgram.hpp"

namespace GLOO {
class PhongShader : public ShaderProgram {
 public:
  PhongShader();
  void SetTargetNode(const SceneNode& node,
                     const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;
  void SetLightSource(const LightComponent& componentt) const override;

  void SetShadowMapping(
      const Texture& shadow_texture,
      const glm::mat4& world_to_light_ndc_matrix) const override;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
  static const int diffuse_texture_unit = 0;
  static const int specular_texture_unit = 1;
  static const int ambient_texture_unit = 2;
  static const int shadow_map_unit = 3;
  const float shadow_bias_ = 0.001;  // No bias value is perfect, but this works pretty well
};
}  // namespace GLOO

#endif
