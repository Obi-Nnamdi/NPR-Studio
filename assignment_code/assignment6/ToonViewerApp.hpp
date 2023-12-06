#ifndef TOON_VIEWER_APP_H_
#define TOON_VIEWER_APP_H_
#include "gloo/Application.hpp"

namespace GLOO {
class ToonViewerApp : public Application {
 public:
  ToonViewerApp(const std::string& app_name, glm::ivec2 window_size);
  void SetupScene() override;
};
}  // namespace GLOO

#endif