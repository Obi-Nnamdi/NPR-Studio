#ifndef GLOO_APPLICATION_H_
#define GLOO_APPLICATION_H_

#include <memory>
#include <string>

#include "external.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"

namespace GLOO {
class Application {
 public:
  Application(std::string app_name, glm::ivec2 window_size);
  virtual ~Application();
  bool IsFinished();
  void Tick(double delta_time, double current_time);
  glm::ivec2 GetWindowSize() const {
    return window_size_;
  }

  virtual void FramebufferSizeCallback(glm::ivec2 window_size);

 protected:
  virtual void DrawGUI() {
  }
  virtual void SetupScene() = 0;
  std::unique_ptr<Scene> scene_;

 private:
  void InitializeGLFW();
  void InitializeGUI();
  void SetRenderingOptions();
  void UpdateGUI();
  void RenderGUI();
  void DestroyGUI();

  GLFWwindow* window_handle_;
  std::string app_name_;
  glm::ivec2 window_size_;

  std::unique_ptr<Renderer> renderer_;
};
}  // namespace GLOO

#endif
