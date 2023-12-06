#ifndef GLOO_MATERIAL_H_
#define GLOO_MATERIAL_H_

#include <glm/glm.hpp>

#include "gl_wrapper/Texture.hpp"

namespace GLOO {
class Material {
 public:
  Material()
      : ambient_color_(0.0f),
        diffuse_color_(0.0f),
        specular_color_(0.0f),
        shininess_(0.0f) {
  }
  Material(const glm::vec3& ambient_color,
           const glm::vec3& diffuse_color,
           const glm::vec3& specular_color,
           float shininess)
      : ambient_color_(ambient_color),
        diffuse_color_(diffuse_color),
        specular_color_(specular_color),
        shininess_(shininess) {
  }

  static const Material& GetDefault() {
    static Material default_material(glm::vec3(0.5f, 0.1f, 0.2f),
                                     glm::vec3(0.5f, 0.1f, 0.2f),
                                     glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
    return default_material;
  }

  glm::vec3 GetAmbientColor() const {
    return ambient_color_;
  }

  void SetAmbientColor(const glm::vec3& color) {
    ambient_color_ = color;
  }

  glm::vec3 GetDiffuseColor() const {
    return diffuse_color_;
  }

  void SetDiffuseColor(const glm::vec3& color) {
    diffuse_color_ = color;
  }

  glm::vec3 GetSpecularColor() const {
    return specular_color_;
  }

  void SetSpecularColor(const glm::vec3& color) {
    specular_color_ = color;
  }

  float GetShininess() const {
    return shininess_;
  }

  void SetShininess(float shininess) {
    shininess_ = shininess;
  }
  void SetAmbientTexture(std::shared_ptr<Texture> tex) {
    ambient_tex_ = std::move(tex);
  }

  void SetDiffuseTexture(std::shared_ptr<Texture> tex) {
    diffuse_tex_ = std::move(tex);
  }

  void SetSpecularTexture(std::shared_ptr<Texture> tex) {
    specular_tex_ = std::move(tex);
  }

  std::shared_ptr<Texture> GetAmbientTexture() const {
    return ambient_tex_;
  }

  std::shared_ptr<Texture> GetDiffuseTexture() const {
    return diffuse_tex_;
  }

  std::shared_ptr<Texture> GetSpecularTexture() const {
    return specular_tex_;
  }

 private:
  glm::vec3 ambient_color_;
  glm::vec3 diffuse_color_;
  glm::vec3 specular_color_;
  float shininess_;
  std::shared_ptr<Texture> ambient_tex_;
  std::shared_ptr<Texture> diffuse_tex_;
  std::shared_ptr<Texture> specular_tex_;
};
}  // namespace GLOO

#endif
