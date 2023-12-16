#ifndef GLOO_OUTLINE_SHADER_H
#define GLOO_OUTLINE_SHADER_H

#include "ShaderProgram.hpp"

namespace GLOO {
/**
 * Shader for created outlines with a desired thickness.
 */

class OutlineShader : public ShaderProgram {
 public:
  OutlineShader();
  void SetTargetNode(const SceneNode& node, const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};

}  // namespace GLOO

#endif