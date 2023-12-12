#ifndef GLOO_TOON_SHADER_H
#define GLOO_TOON_SHADER_H

#include "ShaderProgram.hpp"

namespace GLOO {
/**
 * Shader that represents tone mapping between a lower color and higher color.
 */
class ToonShader : public ShaderProgram {
 public:
  ToonShader();
  void SetTargetNode(const SceneNode& node, const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;
  void SetLightSource(const LightComponent& componentt) const override;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
