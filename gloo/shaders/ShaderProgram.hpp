#ifndef GLOO_SHADER_PROGRAM_H_
#define GLOO_SHADER_PROGRAM_H_

#include "gloo/gl_wrapper/IBindable.hpp"

#include <string>
#include <unordered_map>

#include <glad/glad.h>

#include "gloo/gl_wrapper/VertexArray.hpp"
#include "gloo/Transform.hpp"
#include "gloo/gl_wrapper/Texture.hpp"

namespace GLOO {
class CameraComponent;
class LightComponent;
class SceneNode;

class ShaderProgram : public IBindable {
 public:
  ShaderProgram(
      const std::unordered_map<GLenum, std::string>& shader_filenames);
  virtual ~ShaderProgram();
  void Bind() const override;
  void Unbind() const override;
  GLint GetAttributeLocation(const std::string& name) const;

  // The following Set* methods are called by the renderer, thus const.
  virtual void SetTargetNode(const SceneNode& node,
                             const glm::mat4& local_to_world_mat) const {
  }
  virtual void SetCamera(const CameraComponent& camera) const {
  }
  virtual void SetLightSource(const LightComponent& light) const {
  }
  virtual void SetShadowMapping(
      const Texture& shadow_texture,
      const glm::mat4& world_to_light_NDC_matrix) const {
  }

 protected:
  // Protected because only shader subclasses have information to the names.
  void SetUniform(const std::string& name, const glm::mat4& value) const;
  void SetUniform(const std::string& name, const glm::mat3& value) const;
  void SetUniform(const std::string& name, const glm::vec3& value) const;
  void SetUniform(const std::string& name, float value) const;
  void SetUniform(const std::string& name, int value) const;

 private:
  static GLuint LoadShader(GLenum type,
                           std::string shader_code,
                           const std::string& shader_filename);

  const static int kErrorLogBufferSize = 512;

  std::unordered_map<GLenum, GLuint> shader_handles_;
  GLuint shader_program_;
};
}  // namespace GLOO

#endif
