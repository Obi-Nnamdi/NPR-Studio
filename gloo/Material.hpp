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
        shininess_(0.0f),
        illuminated_color_(0.0f),
        shadow_color_(0.0f),
        outline_color_(0.0f),
        outline_thickness_(0.0f) {}

  // Realistic Material Constructor
  Material(const glm::vec3& ambient_color, const glm::vec3& diffuse_color,
           const glm::vec3& specular_color, float shininess)
      : ambient_color_(ambient_color),
        diffuse_color_(diffuse_color),
        specular_color_(specular_color),
        shininess_(shininess),
        illuminated_color_(0.0f),
        shadow_color_(0.0f),
        outline_color_(0.0f),
        outline_thickness_(0.0f) {}

  // NPR Material Constructor (arguments are weird because of the realistic material constructor
  // already existing)
  Material(const glm::vec3& illuminated_color, const glm::vec3& shadow_color,
           const float& outline_thickness, const glm::vec3& outline_color)
      : ambient_color_(0.0f),
        diffuse_color_(0.0f),
        specular_color_(0.0f),
        illuminated_color_(illuminated_color),
        shadow_color_(shadow_color),
        outline_color_(outline_color),
        outline_thickness_(outline_thickness) {}

  static const Material& GetDefault() {
    static Material default_material(glm::vec3(0.5f, 0.1f, 0.2f),
                                     glm::vec3(0.5f, 0.1f, 0.2f),
                                     glm::vec3(0.4f, 0.4f, 0.4f), 20.0f);
    return default_material;
  }

  static const Material& GetDefaultNPR() {
    static Material default_material_npr(glm::vec3(1.), glm::vec3(0.1), 4, glm::vec3(1.));
    return default_material_npr;
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

  void SetShadowColor(const glm::vec3& color) { shadow_color_ = color; }
  glm::vec3 GetShadowColor() const { return shadow_color_; }

  void SetIlluminatedColor(const glm::vec3& color) { illuminated_color_ = color; }
  glm::vec3 GetIlluminatedColor() const { return illuminated_color_; }

  void SetOutlineColor(const glm::vec3& color) { outline_color_ = color; }
  glm::vec3 GetOutlineColor() const { return outline_color_; }

  void SetOutlineThickness(const float& width) { outline_thickness_ = width; }
  float GetOutlineThickness() { return outline_thickness_; }

  // TODO: SetCoolColor and SetWarmColor options?
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
  // Used in NPR rendering
  glm::vec3 illuminated_color_;
  glm::vec3 shadow_color_;
  glm::vec3 outline_color_;
  float outline_thickness_;  // in pixels
  std::shared_ptr<Texture> ambient_tex_;
  std::shared_ptr<Texture> diffuse_tex_;
  std::shared_ptr<Texture> specular_tex_;
};
}  // namespace GLOO

#endif
