#include "SimpleShader.hpp"

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

namespace GLOO {
SimpleShader::SimpleShader()
    : ShaderProgram(std::unordered_map<GLenum, std::string>(
          {{GL_VERTEX_SHADER, "simple.vert"},
           {GL_FRAGMENT_SHADER, "simple.frag"}})) {
}

void SimpleShader::AssociateVertexArray(VertexArray& vertex_array) const {
  if (!vertex_array.HasPositionBuffer()) {
    throw std::runtime_error("Simple shader requires vertex positions!");
  }
  vertex_array.LinkPositionBuffer(GetAttributeLocation("vertex_position"));
}

void SimpleShader::SetTargetNode(const SceneNode& node,
                                 const glm::mat4& model_matrix) const {
  // Associate the right VAO before rendering.
  AssociateVertexArray(node.GetComponentPtr<RenderingComponent>()
                           ->GetVertexObjectPtr()
                           ->GetVertexArray());

  // Set transform.
  SetUniform("model_matrix", model_matrix);

  // Set material.
  MaterialComponent* material_component_ptr =
      node.GetComponentPtr<MaterialComponent>();
  if (material_component_ptr == nullptr) {
    // Default material: greenish.
    SetUniform("material_color", glm::vec3(0.0f, 0.7f, 0.2f));
  } else {
    SetUniform("material_color",
               material_component_ptr->GetMaterial().GetDiffuseColor());
  }
}

void SimpleShader::SetCamera(const CameraComponent& camera) const {
  SetUniform("view_matrix", camera.GetViewMatrix());
  SetUniform("projection_matrix", camera.GetProjectionMatrix());
}

}  // namespace GLOO
