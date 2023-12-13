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

  std::shared_ptr<ToonShader> toon_shader_;
  std::shared_ptr<ToneMappingShader> tone_mapping_shader_;
  ToonShadingType shading_type_;
};
}  // namespace GLOO

#endif