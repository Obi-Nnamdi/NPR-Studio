#include "MiterOutlineShader.hpp"

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
MiterOutlineShader::MiterOutlineShader()
    : ShaderProgram(std::unordered_map<GLenum, std::string>{
          {GL_VERTEX_SHADER, "miter_outline.vert"}, {GL_FRAGMENT_SHADER, "miter_outline.frag"}}) {
  vertex_ubo_ = CreateUBO();
}

void MiterOutlineShader::AssociateVertexArray(VertexArray& vertex_array) const {
  if (!vertex_array.HasPositionBuffer()) {
    throw std::runtime_error("Outline shader requires vertex positions!");
  }
  vertex_array.LinkPositionBuffer(GetAttributeLocation("vertex_position"));
}

MiterOutlineShader::~MiterOutlineShader() {
  // Delete assigned UBO buffer
  glDeleteBuffers(1, &vertex_ubo_);
}

void MiterOutlineShader::SetTargetNode(const SceneNode& node, const glm::mat4& model_matrix) const {
  // Create a UBO from the vertex positions before rendering
  UpdateUBO(node.GetComponentPtr<RenderingComponent>()->GetVertexObjectPtr()->GetPositions());

  // Set transform.
  SetUniform("model_matrix", model_matrix);

  // Set material.
  MaterialComponent* material_component_ptr = node.GetComponentPtr<MaterialComponent>();
  const Material* material_ptr;
  if (material_component_ptr == nullptr) {
    material_ptr = &Material::GetDefaultNPR();
  } else {
    material_ptr = &material_component_ptr->GetMaterial();
  }

  SetUniform("material_color", material_ptr->GetOutlineColor());
  SetUniform("u_thickness", material_ptr->GetOutlineThickness());
}

void MiterOutlineShader::SetCamera(const CameraComponent& camera) const {
  // Update shader using window size
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  glm::vec2 float_window_size = glm::vec2((float)window_size.x, (float)window_size.y);
  SetUniform("u_resolution", float_window_size);

  SetUniform("view_matrix", camera.GetViewMatrix());

  glm::mat4 modelview2(1.0f);
  modelview2 = glm::translate(modelview2, glm::vec3(0.6f, 0.0f, 0.0f));
  modelview2 = glm::scale(modelview2, glm::vec3(0.5f, 0.5f, 1.0f));
  SetUniform("model_view_project_matrix", camera.GetProjectionMatrix() * camera.GetViewMatrix());
  SetUniform("projection_matrix", camera.GetProjectionMatrix());
}

GLuint MiterOutlineShader::CreateUBO() const {
  GLuint ubo;
  GL_CHECK(glGenBuffers(1, &ubo));
  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ubo));

  // Allocate space for the UBO data (size of VertexInfo + maxUBOArraySize - 1 * sizeof(glm::vec4))
  GL_CHECK(glBufferData(GL_UNIFORM_BUFFER,
                        sizeof(VertexInfo) + (maxUBOArraySize - 1) * sizeof(glm::vec4), NULL,
                        GL_DYNAMIC_DRAW));  // Use GL_DYNAMIC_DRAW so we can use mapping

  // Bind the UBO to a binding point (here we choose 0)
  GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, buffer_binding_point_, ubo));

  return ubo;
}

void MiterOutlineShader::ReinitializeUBO() const {
  // Bind buffer
  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, vertex_ubo_));

  // Allocate space for the UBO data (size of VertexInfo + maxUBOArraySize - 1 * sizeof(glm::vec4))
  GL_CHECK(glBufferData(GL_UNIFORM_BUFFER,
                        sizeof(VertexInfo) + (maxUBOArraySize - 1) * sizeof(glm::vec4), NULL,
                        GL_DYNAMIC_DRAW));
}

void MiterOutlineShader::UpdateUBO(const std::vector<glm::vec3>& varray) const {
  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, vertex_ubo_));

  // Map the buffer for writing
  VertexInfo* uboData = static_cast<VertexInfo*>(glMapBufferRange(
      GL_UNIFORM_BUFFER, 0, sizeof(VertexInfo), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

  if (!uboData) {
    // Handle error: Unable to map the buffer
    throw std::runtime_error("unable to map buffer");
    return;
  }

  // Populate the array with data
  for (int i = 0; i < varray.size(); ++i) {
    uboData->myVec4Array[i] = glm::vec4(varray[i], 1.0f);
  }

  // Unmap the buffer, and if the unmap operation fails, we need to reinitialize the buffer
  if (GL_CHECK(glUnmapBuffer(GL_UNIFORM_BUFFER)) == GL_FALSE) {
    std::cerr << "Unmapping operation in MiterOutlineShader failed, reinitializing UBO"
              << std::endl;
    ReinitializeUBO();
  }

  // Bind the UBO to the binding point
  GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, buffer_binding_point_, vertex_ubo_));
}

}  // namespace GLOO