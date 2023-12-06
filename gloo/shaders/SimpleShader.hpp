#ifndef GLOO_SIMPLE_SHADER_H_
#define GLOO_SIMPLE_SHADER_H_

#include "ShaderProgram.hpp"

namespace GLOO {
// A simple shader for debug purposes.
class SimpleShader : public ShaderProgram {
 public:
  SimpleShader();
  void SetTargetNode(const SceneNode& node,
                     const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
