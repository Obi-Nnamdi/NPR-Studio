#include "ToonViewerApp.hpp"

#include <sys/stat.h>
// Include windows header if we're on windows
#ifdef _WIN32
#include <windows.h>
#endif

#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include "../common/helpers.hpp"
#include "../stb/stb_image_write.h"
#include "OutlineNode.hpp"
#include "SunNode.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/cameras/BasicCameraNode.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/shaders/ToneMappingShader.hpp"
#include "gloo/shaders/ToonShader.hpp"

namespace {
void SetAmbientToDiffuse(GLOO::MeshData& mesh_data) {
  // Certain groups do not have an ambient color, so we use their diffuse colors
  // instead.
  for (auto& g : mesh_data.groups) {
    if (glm::length(g.material->GetAmbientColor()) < 1e-3) {
      g.material->SetAmbientColor(g.material->GetDiffuseColor());
      g.material->SetAmbientTexture(g.material->GetDiffuseTexture());
    }
  }
}

// TODO: delete this fucntion and just use the override NPR colors from diffuse one.
void SetNPRColorsFromDiffuse(GLOO::MeshData& mesh_data, float illuminationFactor = 1.5,
                             float shadowFactor = .5, float outlineFactor = 1) {
  // For groups that don't have shadow/illuminated colors, we use their diffuse color * 1.5 as the
  // illuminated color, and the diffuse color * .5 as the shadow color.
  // For groups with no edge color, we use a darker version of the shadow color.
  for (auto& g : mesh_data.groups) {
    auto diffuse_color = g.material->GetDiffuseColor();
    // TODO: maybe replace with default NPR material if diffuse isn't large enough?
    if (glm::length(g.material->GetIlluminatedColor()) < 1e-3) {
      g.material->SetIlluminatedColor(illuminationFactor * diffuse_color);
    }
    if (glm::length(g.material->GetShadowColor()) < 1e-3) {
      g.material->SetShadowColor(shadowFactor * diffuse_color);
    }
    if (glm::length(g.material->GetOutlineColor()) < 1e-3) {
      g.material->SetOutlineColor(outlineFactor * diffuse_color);
    }
  }
}

// Prints vector of floats to a space separated string
std::string floatVectorToString(const std::vector<float>& values) {
  std::string output = "";
  for (int i = 0; i < values.size(); i++) {
    float value = values[i];
    std::ostringstream stream;
    stream << value;
    output += std::string(stream.str());
    // Add a space at the end of every float except the last one
    if (i != values.size() - 1) {
      output += " ";
    }
  }
  return output;
}

#ifdef _WIN32
// Function to create a directory if it doesn't exist (Windows-specific)
void CreateDirectoryIfNotExists(const std::string& directoryPath) {
  if (!CreateDirectoryA(directoryPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
    std::cerr << "Error creating directory: " << directoryPath << std::endl;
  }
}
#elif defined(__APPLE__) || defined(__linux__)
// Function to create a directory if it doesn't exist (POSIX - macOS/Linux)
void CreateDirectoryIfNotExists(const std::string& directoryPath) {
  struct stat st;
  if (stat(directoryPath.c_str(), &st) == -1) {
    // Directory doesn't exist, so create it
    if (mkdir(directoryPath.c_str(), 0777) != 0) {
      std::cerr << "Error creating directory: " << directoryPath << std::endl;
    }
  }
}
#else
#error Unsupported platform for reading and writing file directories
#endif
}  // namespace

namespace GLOO {
ToonViewerApp::ToonViewerApp(const std::string& app_name, glm::ivec2 window_size,
                             const std::string& model_filename)
    : Application(app_name, window_size), model_filename_(model_filename) {
  background_color_ = {0, 0, 0, 1};
  // These colors mirror the defaults found in Material::GetDefaultNPR().
  // TOOD: better way to do this?
  illumination_color_ = {1, 1, 1};
  shadow_color_ = {.1, .1, .1};
  outline_color_ = {1, 1, 1};
  toon_shader_ = std::make_shared<ToonShader>();
  tone_mapping_shader_ = std::make_shared<ToneMappingShader>();
  shading_type_ = ToonShadingType::TONE_MAPPING;
}

void ToonViewerApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  // Setting up the camera.
  auto camera_node = make_unique<ArcBallCameraNode>(50.0f, 1.0f, 10.0f);
  // camera_node->GetTransform().SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
  camera_node->GetTransform().SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), kPi / 2);
  camera_node->Calibrate();
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  // Add in the sun.
  auto sun = make_unique<SunNode>();
  sun_node_ = sun.get();
  root.AddChild(std::move(sun));

  // Create shader instance
  auto shader = std::make_shared<ToneMappingShader>();
  // Load and set up the scene OBJ if we have a specified model file.
  // If not, load up a basic sphere.
  if (model_filename_ != "") {
    MeshData mesh_data = MeshLoader::Import(model_filename_);
    SetAmbientToDiffuse(mesh_data);
    SetNPRColorsFromDiffuse(mesh_data, 1.2, .5, 1);

    std::shared_ptr<VertexObject> vertex_obj = std::move(mesh_data.vertex_obj);

    if (mesh_data.groups.size() == 0) {
      // load full model at once
      auto outline_node = make_unique<OutlineNode>(scene_.get(), vertex_obj, tone_mapping_shader_);
      outline_nodes_.push_back(outline_node.get());
      root.AddChild(std::move(outline_node));
    } else {
      // Create scene nodes for each mesh group
      for (MeshGroup& group : mesh_data.groups) {
        // Query the mesh data to only draw the part of the larger mesh belonging to this node
        auto outline_node =
            make_unique<OutlineNode>(scene_.get(), vertex_obj, group.start_face_index,
                                     group.num_indices, group.material, tone_mapping_shader_);
        outline_nodes_.push_back(outline_node.get());
        root.AddChild(std::move(outline_node));
      }
    }
  } else {
    // Other basic mesh options:
    // std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(2.f, 64, 64);
    // auto mesh = PrimitiveFactory::CreateQuad();
    auto outline_node = make_unique<OutlineNode>(scene_.get(), nullptr, tone_mapping_shader_);
    outline_nodes_.push_back(outline_node.get());
    root.AddChild(std::move(outline_node));
  }
}

void ToonViewerApp::ToggleShading() {
  // Toggle shading type parameter
  shading_type_ = shading_type_ == ToonShadingType::TOON ? ToonShadingType::TONE_MAPPING
                                                         : ToonShadingType::TOON;

  SetShadingType(shading_type_);
}

void ToonViewerApp::SetShadingType(const ToonShadingType& shading_type) {
  // Get associated shader
  // TODO: Map data structure for easier logic?
  std::shared_ptr<ShaderProgram> newShader;
  if (shading_type_ == ToonShadingType::TOON) {
    newShader = toon_shader_;
  } else {
    newShader = tone_mapping_shader_;
  }

  // Update outline nodes
  for (auto node : outline_nodes_) {
    node->ChangeMeshShader(newShader);
  }
}

void ToonViewerApp::UpdateSilhouetteStatus() {
  // Update outline nodes
  for (auto node : outline_nodes_) {
    node->SetSilhouetteStatus(show_silhouette_);
  }
}

void ToonViewerApp::UpdateCreaseStatus() {
  // Update outline nodes
  for (auto node : outline_nodes_) {
    node->SetCreaseStatus(show_crease_);
  }
}

void ToonViewerApp::UpdateBorderStatus() {
  // Update outline nodes
  for (auto node : outline_nodes_) {
    node->SetBorderStatus(show_border_);
  }
}

void ToonViewerApp::UpdateCreaseThreshold() {
  for (auto node : outline_nodes_) {
    node->SetCreaseThreshold(crease_threshold_);
  }
}

void ToonViewerApp::UpdateOutlineThickness() {
  for (auto node : outline_nodes_) {
    node->SetOutlineThickness(outline_thickness_);
  }
}

void ToonViewerApp::UpdateOutlineMethod() {
  OutlineMethod outlineMethod = use_miter_joins_ ? OutlineMethod::MITER : OutlineMethod::STANDARD;
  for (auto node : outline_nodes_) {
    node->SetOutlineMethod(outlineMethod);
  }
}

void ToonViewerApp::UpdatePerformanceModeStatus() {
  ApplyFuncToOutlineNodes([this](OutlineNode* node) {
    node->SetPerformanceModeStatus(this->enable_outline_performance_mode_);
  });
}

void ToonViewerApp::UpdateMeshVisibility() {
  for (auto node : outline_nodes_) {
    node->SetMeshVisibility(show_mesh_);
  }
}

void ToonViewerApp::SetIlluminatedColor(const glm::vec3& color) {
  for (auto node : outline_nodes_) {
    node->SetIlluminatedColor(color);
  }
}

void ToonViewerApp::SetShadowColor(const glm::vec3& color) {
  for (auto node : outline_nodes_) {
    node->SetShadowColor(color);
  }
}

void ToonViewerApp::SetOutlineColor(const glm::vec3& color) {
  for (auto node : outline_nodes_) {
    node->SetOutlineColor(color);
  }
}

void ToonViewerApp::OverrideNPRColorsFromDiffuse(float illuminationFactor, float shadowFactor,
                                                 float outlineFactor) {
  for (auto node : outline_nodes_) {
    node->OverrideNPRColorsFromDiffuse(illuminationFactor, shadowFactor, outlineFactor);
  }
}

void ToonViewerApp::ApplyFuncToOutlineNodes(const std::function<void(OutlineNode*)>& func) {
  for (auto node : outline_nodes_) {
    func(node);
  }
}

void ToonViewerApp::RenderImageToFile(const std::string filename,
                                      const std::string extension) const {
  // Use glReadPixels to get image data from entire window, starting at its lower left corner.
  auto windowSize = GetWindowSize();
  auto width = windowSize.x;
  auto height = windowSize.y;
  glm::ivec2 lowerLeftCorner(0, 0);
  int channels = 4;  // RGBA image data

  // Use heap allocation to store image data
  uint8_t* imageData =
      new uint8_t[width * height * channels];  // RGBA image data in row-major order
  GL_CHECK(glReadPixels(lowerLeftCorner.x, lowerLeftCorner.y, width, height, GL_RGBA,
                        GL_UNSIGNED_INT_8_8_8_8_REV, imageData));
  // Note that byte order is weird unless you use GL_UNSIGNED_INT_8_8_8_8_REV

  std::string full_filename = GetRenderDir() + filename + extension;
  // Write image data to file
  int success = 0;
  stbi_flip_vertically_on_write(true);  // Flip image vertically when writing for openGL
  if (extension == ".png") {
    int row_stride =
        sizeof(uint8_t) * width * channels;  // distance between rows of image data (in bytes)
    stbi_write_png_compression_level = 0;    // max quality
    success = stbi_write_png(full_filename.c_str(), width, height, channels, imageData, row_stride);
  } else if (extension == ".jpg") {
    int quality = 100;  // max quality
    success = stbi_write_jpg(full_filename.c_str(), width, height, channels, imageData, quality);
  } else if (extension == ".bmp") {
    success = stbi_write_bmp(full_filename.c_str(), width, height, channels, imageData);
  } else if (extension == ".tga") {
    success = stbi_write_tga(full_filename.c_str(), width, height, channels, imageData);
  }

  if (success == 0) {  // failure to write file
    std::cerr << "ERROR: Image saving operation failed!" << std::endl;
  }

  // Free imageData memory
  delete[] imageData;
}

void ToonViewerApp::SaveRenderSettings(const std::string filename, const bool& includeColorInfo,
                                       const bool& includeLightInfo, const bool& includeMeshInfo,
                                       const bool& includeOutlineInfo,
                                       const bool& includeShaderInfo) {
  CreateDirectoryIfNotExists(GetPresetDir());
  std::string full_filename = GetPresetDir() + filename + ".npr";
  std::ofstream file(full_filename);
  // .npr filetype is structured to have "commands" that group similar bits of information, where
  // each command is terminated by "end". It allows for modular information, so you can specify only
  // certain settings in an .npr file.
  if (file.is_open()) {
    // Colors commmand - records colors of scene
    if (includeColorInfo) {
      file << "colors\n";
      file << "background"
           << " " << floatVectorToString(background_color_) << "\n";
      file << "illum"
           << " " << floatVectorToString(illumination_color_) << "\n";
      file << "shadow"
           << " " << floatVectorToString(shadow_color_) << "\n";
      file << "outline"
           << " " << floatVectorToString(outline_color_) << "\n";
      file << "end\n";
      file << "\n";
    }
    if (includeShaderInfo) {
      // Shader Command - records shader-specific information
      file << "shader\n";
      file << "type"
           << " " << shading_type_ << "\n";
      file << "end\n";
      file << "\n";
    }
    // Outlines Command - records information about outlines
    if (includeOutlineInfo) {
      file << "outlines\n";
      file << "miter"
           << " " << use_miter_joins_ << "\n";
      // TODO: Include performance mode info?
      file << "sil"
           << " " << show_silhouette_ << "\n";
      file << "crease"
           << " " << show_crease_ << "\n";
      file << "border"
           << " " << show_border_ << "\n";
      file << "width"
           << " " << outline_thickness_ << "\n";
      file << "thresh"
           << " " << crease_threshold_ << "\n";
      file << "end\n";
      file << "\n";
    }
    // Mesh Command - records global mesh settings
    if (includeMeshInfo) {
      file << "mesh\n";
      file << "visible"
           << " " << show_mesh_ << "\n";
      file << "end\n";
      file << "\n";
    }
    // Light Command - records global light settings
    if (includeLightInfo) {
      file << "light\n";
      file << "type"
           << " " << ((int)sun_node_->GetLightType()) << "\n";
      file << "radius"
           << " " << point_light_radius_ << "\n";
      file << "end\n";
    }
  }
}

void ToonViewerApp::LoadRenderSettings(const std::string filename) {
  std::string full_filename = GetPresetDir() + filename + ".npr";
  std::ifstream file(full_filename);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      // Handle importing colors
      if (line == "colors") {
        while (std::getline(file, line)) {
          if (line == "end") {
            break;
          }
          std::vector<std::string> tokens = Split(line, ' ');
          std::string command = tokens[0];  // The command is always the first line token
          // Make a float vector out of the rest of the line
          std::vector<float> values;
          for (int i = 1; i < tokens.size(); i++) {
            values.push_back(std::stof(tokens[i]));
          }
          if (command == "background") {
            background_color_ = values;
            SetBackgroundColor(vectorToVec4(values));
          } else if (command == "illum") {
            illumination_color_ = values;
            SetIlluminatedColor(vectorToVec3(values));
          } else if (command == "shadow") {
            shadow_color_ = values;
            SetShadowColor(vectorToVec3(values));
          } else if (command == "outline") {
            outline_color_ = values;
            SetOutlineColor(vectorToVec3(values));
          }
        }
      }
      // Handle shader settings
      else if (line == "shader") {
        while (std::getline(file, line)) {
          if (line == "end") {
            break;
          }
          std::vector<std::string> tokens = Split(line, ' ');
          std::string command = tokens[0];
          if (command == "type") {
            std::string value = tokens[1];
            ToonShadingType shading_type = static_cast<ToonShadingType>(std::stoi(value));
            shading_type_ = shading_type;
            SetShadingType(shading_type_);
          }
        }
      }
      // Handle outline settings
      else if (line == "outlines") {
        while (std::getline(file, line)) {
          if (line == "end") {
            break;
          }
          std::vector<std::string> tokens = Split(line, ' ');
          std::string command = tokens[0];
          std::string value = tokens[1];
          if (command == "miter") {
            use_miter_joins_ = std::stoi(value);
            UpdateOutlineMethod();
          } else if (command == "sil") {
            show_silhouette_ = std::stoi(value);
            UpdateSilhouetteStatus();
          } else if (command == "crease") {
            show_crease_ = std::stoi(value);
            UpdateCreaseStatus();
          } else if (command == "border") {
            show_border_ = std::stoi(value);
            UpdateBorderStatus();
          } else if (command == "width") {
            outline_thickness_ = std::stof(value);
            UpdateOutlineThickness();
          } else if (command == "thresh") {
            crease_threshold_ = std::stof(value);
            UpdateCreaseThreshold();
          }
        }
      }
      // Handle mesh settings
      else if (line == "mesh") {
        while (std::getline(file, line)) {
          if (line == "end") {
            break;
          }
          std::vector<std::string> tokens = Split(line, ' ');
          std::string command = tokens[0];
          std::string value = tokens[1];
          if (command == "visible") {
            show_mesh_ = std::stoi(value);
            UpdateMeshVisibility();
          }
        }
      }
      // Handle light settings
      else if (line == "light") {
        while (std::getline(file, line)) {
          if (line == "end") {
            break;
          }
          std::vector<std::string> tokens = Split(line, ' ');
          std::string command = tokens[0];
          std::string value = tokens[1];
          if (command == "type") {
            sun_node_->SetLightType(static_cast<LightType>(std::stoi(value)));
          } else if (command == "radius") {
            point_light_radius_ = std::stof(value);
            sun_node_->SetRadius(point_light_radius_);
          }
        }
      }
    }
  } else {
    std::cerr << "Error loading preset file " << full_filename << std::endl;
  }
}

void ToonViewerApp::DrawGUI() {
  // Information for file rendering.
  const char* fileExtensions[] = {".png", ".jpg", ".bmp", ".tga"};
  static char renderFilename[512];
  static int item_current = 0;

  // Special cases to hide GUI when we're taking a screenshot:
  if (renderingImageCountdown == 0) {
    RenderImageToFile(renderFilename, fileExtensions[item_current]);
    renderingImageCountdown--;
    return;
  } else if (renderingImageCountdown > 0) {
    renderingImageCountdown--;
    return;
  }

  // Dear ImGUI documentation at https://github.com/ocornut/imgui?tab=readme-ov-file#usage
  // ImGui::ShowDemoWindow();

  ImGui::Begin("Rendering Controls");

  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader("Lighting Controls:")) {
    // Button for toggling light visibility
    if (ImGui::Button("Toggle Light Type (Point/Directional)")) {
      sun_node_->ToggleLight();
    }
    ImGui::Separator();

    ImGui::Text("Point Light Controls:");
    // Slider for changing light radius
    if (ImGui::SliderFloat("Light Radius", &point_light_radius_, 0, 30, "%.2f")) {
      sun_node_->SetRadius(point_light_radius_);
    }
  }
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader("Shader Controls:")) {
    // Controls background color of our scene
    if (ImGui::ColorEdit4("Background Color", &background_color_.front(),
                          ImGuiColorEditFlags_AlphaPreview)) {
      SetBackgroundColor(vectorToVec4(background_color_));
    }
    if (ImGui::ColorEdit3("Illumination Color", &illumination_color_.front())) {
      SetIlluminatedColor(vectorToVec3(illumination_color_));
    }
    if (ImGui::ColorEdit3("Shadow Color", &shadow_color_.front())) {
      SetShadowColor(vectorToVec3(shadow_color_));
    }
    if (ImGui::ColorEdit3("Outline Color", &outline_color_.front())) {
      SetOutlineColor(vectorToVec3(outline_color_));
    }
    if (ImGui::Button("Reset Colors to Material Diffuse")) {
      OverrideNPRColorsFromDiffuse(1.2, 0.5);
    }
    // Button for toggling between our shader types
    if (ImGui::Button("Toggle Toon/Tone Mapping Shader")) {
      ToggleShading();
    }
  }

  // Checkboxes for toggling edge type displays
  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader("Edge Controls:")) {
    if (ImGui::Checkbox("Use Miter Join Method (slow/experimental)", &use_miter_joins_)) {
      UpdateOutlineMethod();
    }
    if (ImGui::Checkbox("Performance Mode", &enable_outline_performance_mode_)) {
      UpdatePerformanceModeStatus();
    }
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Renders scene without Miter Joins when camera is moving.");
      ImGui::EndTooltip();
    }
    ImGui::Separator();

    ImGui::Text("Edge Width:");
    if (ImGui::SliderFloat("Pixels", &outline_thickness_, 0, 100, "%.1f")) {
      UpdateOutlineThickness();
    }
    ImGui::Separator();

    if (ImGui::Checkbox("Draw Silhouette Edges", &show_silhouette_)) {
      UpdateSilhouetteStatus();
    }

    if (ImGui::Checkbox("Draw Crease Edges", &show_crease_)) {
      UpdateCreaseStatus();
    }

    if (ImGui::Checkbox("Draw Border Edges", &show_border_)) {
      UpdateBorderStatus();
    }
    ImGui::Separator();

    ImGui::Text("Crease Threshold:");
    // TODO: should this go to 360?
    if (ImGui::SliderFloat("Degrees", &crease_threshold_, 0, 180, "%.1f")) {
      UpdateCreaseThreshold();
    }
  }

  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader("Mesh Controls:")) {
    if (ImGui::Checkbox("Show Mesh", &show_mesh_)) {
      UpdateMeshVisibility();
    }
  }

  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader("File Controls:")) {
    // Image saving dialog
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .2f);
    if (ImGui::Button("Save Image")) {
      renderingImageCountdown = 3;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Saves in assets/renders/ folder of project.");
      ImGui::EndTooltip();
    }

    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .4f);
    ImGui::SameLine();
    // Static render variables defined at beginning of function
    ImGui::PushID("Image Filename Label");
    ImGui::InputTextWithHint(/*label = */ "", "filename", renderFilename,
                             IM_ARRAYSIZE(renderFilename));
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .15f);
    ImGui::Combo("format", &item_current, fileExtensions, IM_ARRAYSIZE(fileExtensions));

    // Preset Saving Dialog
    static char saveSettingsFilename[512];
    static bool includeColorInfo = true;
    static bool includeLightInfo = true;
    static bool includeMeshInfo = true;
    static bool includeOutlineInfo = true;
    static bool includeShaderInfo = true;
    static const std::vector<bool*> allSaveSettings = {&includeColorInfo, &includeLightInfo,
                                                       &includeMeshInfo, &includeOutlineInfo,
                                                       &includeShaderInfo};
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .3f);
    if (ImGui::Button("Save Settings")) {
      SaveRenderSettings(saveSettingsFilename, includeColorInfo, includeLightInfo, includeMeshInfo,
                         includeOutlineInfo, includeShaderInfo);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Saves in assets/presets/ folder of project.");
      ImGui::EndTooltip();
    }

    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .5f);
    ImGui::SameLine();
    ImGui::PushID("Save Settings Filename Label");
    ImGui::InputTextWithHint(/*label = */ "", "filename", saveSettingsFilename,
                             IM_ARRAYSIZE(saveSettingsFilename));
    ImGui::PopID();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Settings to save:");
    ImGui::SameLine();
    if (ImGui::Button("Select All")) {
      for (auto setting : allSaveSettings) {
        *setting = true;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Deselect All")) {
      for (auto setting : allSaveSettings) {
        *setting = false;
      }
    }
    ImGui::Columns(3, "settings columns", false);
    ImGui::Checkbox("Color Settings", &includeColorInfo);
    ImGui::NextColumn();
    ImGui::Checkbox("Edge Settings", &includeOutlineInfo);
    ImGui::NextColumn();
    ImGui::Checkbox("Light Settings", &includeLightInfo);
    ImGui::NextColumn();
    ImGui::Checkbox("Mesh Settings", &includeMeshInfo);
    ImGui::NextColumn();
    ImGui::Checkbox("Shader Settings", &includeShaderInfo);
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::Spacing();

    // Preset Loading Dialog
    static char loadSettingsFilename[512];
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .3f);
    if (ImGui::Button("Load Settings")) {
      LoadRenderSettings(loadSettingsFilename);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Loads from assets/presets/ folder of project. Don't include extension.");
      ImGui::EndTooltip();
    }

    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .5f);
    ImGui::SameLine();
    ImGui::PushID("Load Settings Filename Label");
    ImGui::InputTextWithHint(/*label = */ "", "filename", loadSettingsFilename,
                             IM_ARRAYSIZE(loadSettingsFilename));
    ImGui::PopID();
  }

  ImGui::End();
}

}  // namespace GLOO
