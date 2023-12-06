#include "Application.hpp"

#include <iostream>

#include "gloo/utils.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
Application::Application(std::string app_name, glm::ivec2 window_size)
    : app_name_(app_name), window_size_(window_size) {
  InitializeGLFW();
  InitializeGUI();

  scene_ = make_unique<Scene>(make_unique<SceneNode>());
  renderer_ = make_unique<Renderer>(*this);
}

Application::~Application() {
  // Release resources before destroying everything else.
  scene_.release();
  renderer_.release();

  DestroyGUI();
  glfwDestroyWindow(window_handle_);
  glfwTerminate();
}

void Application::InitializeGLFW() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window_handle_ = glfwCreateWindow(window_size_.x, window_size_.y,
                                    app_name_.c_str(), nullptr, nullptr);

  if (window_handle_ == nullptr) {
    std::cerr << "Failed to create GLFW window!" << std::endl;
    return;
  }
  glfwMakeContextCurrent(window_handle_);

  InputManager::GetInstance().SetWindow(window_handle_);

  glfwSetWindowUserPointer(window_handle_, this);
  glfwSetFramebufferSizeCallback(
      window_handle_, +[](GLFWwindow* window, int width, int height) {
        static_cast<Application*>(glfwGetWindowUserPointer(window))
            ->FramebufferSizeCallback(glm::ivec2(width, height));
      });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD!" << std::endl;
    return;
  }

  // On retina display, the initial window size will be larger
  // than requested.
  int initial_width, initial_height;
  glfwGetFramebufferSize(window_handle_, &initial_width, &initial_height);
  FramebufferSizeCallback(glm::ivec2(initial_width, initial_height));
}

void Application::InitializeGUI() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window_handle_, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

bool Application::IsFinished() {
  return glfwWindowShouldClose(window_handle_);
}

void Application::UpdateGUI() {
  // ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  DrawGUI();
}

void Application::RenderGUI() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  // TODO
}

void Application::DestroyGUI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Application::Tick(double delta_time, double current_time) {
  // Process window events.
  glfwPollEvents();
  UpdateGUI();

  // Logic update before rendering.
  scene_->Update(delta_time);

  // Rendering scene and GUI.
  renderer_->Render(*scene_);
  RenderGUI();

  glfwSwapBuffers(window_handle_);
}

void Application::FramebufferSizeCallback(glm::ivec2 window_size) {
  window_size_ = window_size;
  GL_CHECK(glViewport(0, 0, window_size_.x, window_size_.y));
}
}  // namespace GLOO
