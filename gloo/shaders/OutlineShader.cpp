#include "OutlineShader.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>
#include <stdexcept>

#include "gloo/InputManager.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"

namespace GLOO {
OutlineShader::OutlineShader()
    : ShaderProgram(std::unordered_map<GLenum, std::string>{{GL_VERTEX_SHADER, "outline.vert"},
                                                            {GL_GEOMETRY_SHADER, "outline.geom"},
                                                            {GL_FRAGMENT_SHADER, "outline.frag"}}) {
}
// TODO: Use miter join method.
void OutlineShader::AssociateVertexArray(VertexArray& vertex_array) const {
  // TODO: Add fancy vertex array and ssbo calculations here?
  if (!vertex_array.HasPositionBuffer()) {
    throw std::runtime_error("Outline shader requires vertex positions!");
  }
  vertex_array.LinkPositionBuffer(GetAttributeLocation("vertex_position"));
}

void OutlineShader::SetTargetNode(const SceneNode& node, const glm::mat4& model_matrix) const {
  // Associate the right VAO before rendering.
  AssociateVertexArray(
      node.GetComponentPtr<RenderingComponent>()->GetVertexObjectPtr()->GetVertexArray());

  // Set transform.
  SetUniform("model_matrix", model_matrix);

  // Set material.
  MaterialComponent* material_component_ptr = node.GetComponentPtr<MaterialComponent>();
  if (material_component_ptr == nullptr) {
    // Default material: white.
    SetUniform("material_color", glm::vec3(1.f));
    SetUniform("u_thickness", 4.0f);
  } else {
    SetUniform("material_color", material_component_ptr->GetMaterial().GetDiffuseColor());
    SetUniform("u_thickness", material_component_ptr->GetMaterial().GetOutlineThickness());
  }
}

void OutlineShader::SetCamera(const CameraComponent& camera) const {
  // Update shader using window size
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  glm::vec2 inverse_window_size = glm::vec2(1. / window_size.x, 1. / window_size.y);
  SetUniform("u_viewportInvSize", inverse_window_size);

  SetUniform("view_matrix", camera.GetViewMatrix());
  SetUniform("projection_matrix", camera.GetProjectionMatrix());
}

}  // namespace GLOO