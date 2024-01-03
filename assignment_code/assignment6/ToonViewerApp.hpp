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
  void SetShadingType(const ToonShadingType& shading_type);
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

  void OverrideNPRColorsFromDiffuse(float illuminationFactor = 1.5, float shadowFactor = .5,
                                    float outlineFactor = 1);

  void RenderImageToFile(const std::string filename, const std::string extension) const;
  void SaveRenderSettings(const std::string filename, const bool& includeColorInfo = true,
                          const bool& includeLightInfo = true, const bool& includeMeshInfo = true,
                          const bool& includeOutlineInfo = true,
                          const bool& includeShaderInfo = true);  // saves in assets/presets folder
  void LoadRenderSettings(const std::string filename);  // loads from assets/presets folder

  // GUI variables

  bool show_silhouette_ = true;
  bool show_crease_ = true;
  bool show_border_ = true;
  bool use_miter_joins_ = false;
  bool show_mesh_ = true;
  // Control for getting screenshots from renderer
  // TODO do this in a less hacky way (do rendering to a texture?)
  int renderingImageCountdown = -1;

  float crease_threshold_ = 30;  // in degrees
  float outline_thickness_ = 4;  // in pixels
  float point_light_radius_ = 1 / .15;
  std::vector<float> background_color_;  // rbga 4-vector list of background color
  // rgb 3-vector lists
  std::vector<float> illumination_color_;
  std::vector<float> shadow_color_;
  std::vector<float> outline_color_;

  std::shared_ptr<ToonShader> toon_shader_;
  std::shared_ptr<ToneMappingShader> tone_mapping_shader_;
  ToonShadingType shading_type_;
};
}  // namespace GLOO

#endif