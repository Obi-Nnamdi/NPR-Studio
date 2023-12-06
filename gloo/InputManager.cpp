#include "InputManager.hpp"

#include <stdexcept>
#include <cassert>

#include "external.hpp"

namespace GLOO {
void InputManager::SetWindow(GLFWwindow* window) {
  assert(window_ == nullptr);
  window_ = window;
  glfwSetScrollCallback(
      window, [](GLFWwindow* window, double xoffset, double yoffset) {
        if (!ImGui::GetIO().WantCaptureMouse)
          InputManager::GetInstance().mouse_scroll_ += yoffset;
      });
}

bool InputManager::IsKeyPressed(int key) {
  return glfwGetKey(window_, key) == GLFW_PRESS;
}

bool InputManager::IsKeyReleased(int key) {
  return glfwGetKey(window_, key) == GLFW_RELEASE;
}

glm::dvec2 InputManager::GetCursorPosition() {
  double xpos, ypos;
  glfwGetCursorPos(window_, &xpos, &ypos);
  return glm::dvec2(xpos, ypos);
}

bool InputManager::IsLeftMousePressed() {
  if (ImGui::GetIO().WantCaptureMouse)
    return false;
  return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

bool InputManager::IsRightMousePressed() {
  if (ImGui::GetIO().WantCaptureMouse)
    return false;
  return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

bool InputManager::IsMiddleMousePressed() {
  if (ImGui::GetIO().WantCaptureMouse)
    return false;
  return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}

glm::ivec2 InputManager::GetWindowSize() const {
  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  return glm::ivec2(width, height);
}

double InputManager::FetchAndResetMouseScroll() {
  auto ret = mouse_scroll_;
  mouse_scroll_ = 0.0;
  return ret;
}

InputManager::~InputManager() {
  if (window_ != nullptr) {
    // Unregister
    glfwSetScrollCallback(window_, nullptr);
  }
}

}  // namespace GLOO
