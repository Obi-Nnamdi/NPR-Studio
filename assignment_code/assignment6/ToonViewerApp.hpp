#ifndef TOON_VIEWER_APP_H_
#define TOON_VIEWER_APP_H_
#include "OutlineNode.hpp"
#include "SunNode.hpp"
#include "gloo/Application.hpp"
#include "gloo/shaders/ToneMappingShader.hpp"
#include "gloo/shaders/ToonShader.hpp"

namespace GLOO {
class ToonViewerApp : public Application {
 public:
  ToonViewerApp(const std::string& app_name, glm::ivec2 window_size,
                const std::string& model_filename);
  void SetupScene() override;

 protected:
  void DrawGUI() override;

 private:
  std::string model_filename_;
  SunNode* sun_node_;
  std::vector<OutlineNode*> outline_nodes_;
  void ToggleShading();
  // Functions for globally hiding or unhiding each edge type
  void UpdateSilhouetteStatus();
  void UpdateCreaseStatus();
  void UpdateBorderStatus();

  void UpdateCreaseThreshold();
  void UpdateOutlineThickness();
  void UpdateOutlineMethod();
  void UpdateMeshVisibility();
  void SetIlluminatedColor(const glm::vec3& color);
  void SetShadowColor(const glm::vec3& color);
  void SetOutlineColor(const glm::vec3& color);

  void OverrideNPRColorsFromDiffuse(float illuminationFactor = 1.5, float shadowFactor = .5);

  bool showSilhouette = true;
  bool showCrease = true;
  bool showBorder = true;
  bool useMiterJoins = false;
  bool showMesh = true;

  float crease_threshold_ = 30;  // in degrees
  float outline_thickness_ = 4;  // in pixels
  std::vector<float> background_color_;  // rbga 4-vector list of background color
  std::vector<float> illumination_color_;
  std::vector<float> shadow_color_;
  std::vector<float> outline_color_;

  std::shared_ptr<ToonShader> toon_shader_;
  std::shared_ptr<ToneMappingShader> tone_mapping_shader_;
  ToonShadingType shading_type_;
};
}  // namespace GLOO

#endif