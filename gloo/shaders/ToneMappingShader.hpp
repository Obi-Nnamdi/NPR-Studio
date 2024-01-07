#ifndef GLOO_TONE_MAPPING_SHADER_H_
#define GLOO_TONE_MAPPING_SHADER_H_

#include "ShaderProgram.hpp"

namespace GLOO {
/**
 * Shader that represents tone mapping between a lower color and higher color.
 */
class ToneMappingShader : public ShaderProgram {
 public:
  ToneMappingShader();
  void SetTargetNode(const SceneNode& node, const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;
  void SetLightSource(const LightComponent& component) const override;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
