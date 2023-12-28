#ifndef GLOO_MITER_OUTLINE_SHADER_H
#define GLOO_MITER_OUTLINE_SHADER_H
#include "ShaderProgram.hpp"

namespace GLOO {
/**
 * Shader for created outlines with a desired thickness.
 */
const size_t maxUBOArraySize = 1000;
struct VertexInfo {
  // int arraySize;
  glm::vec4 myVec4Array[maxUBOArraySize];  // Minimum array size (adjust as needed)
};

class MiterOutlineShader : public ShaderProgram {
 public:
  MiterOutlineShader();
  ~MiterOutlineShader();
  void SetTargetNode(const SceneNode& node, const glm::mat4& model_matrix) const override;
  void SetCamera(const CameraComponent& camera) const override;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
  GLuint CreateUBO() const;
  void UpdateUBO(const std::vector<glm::vec3>& varray) const;
  GLuint vertex_ubo_;
};

}  // namespace GLOO
#endif