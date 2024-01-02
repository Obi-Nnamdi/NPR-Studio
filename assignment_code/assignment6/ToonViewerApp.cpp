#include "ToonViewerApp.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

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
    node->SetSilhouetteStatus(showSilhouette);
  }
}

void ToonViewerApp::UpdateCreaseStatus() {
  // Update outline nodes
  for (auto node : outline_nodes_) {
    node->SetCreaseStatus(showCrease);
  }
}

void ToonViewerApp::UpdateBorderStatus() {
  // Update outline nodes
  for (auto node : outline_nodes_) {
    node->SetBorderStatus(showBorder);
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
  OutlineMethod outlineMethod = useMiterJoins ? OutlineMethod::MITER : OutlineMethod::STANDARD;
  for (auto node : outline_nodes_) {
    node->SetOutlineMethod(outlineMethod);
  }
}

void ToonViewerApp::UpdateMeshVisibility() {
  for (auto node : outline_nodes_) {
    node->SetMeshVisibility(showMesh);
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

  std::string full_filename = filename + extension;
  // Write image data to file
  stbi_flip_vertically_on_write(true);  // Flip image vertically when writing for openGL
  if (extension == ".png") {
    int row_stride =
        sizeof(uint8_t) * width * channels;  // distance between rows of image data (in bytes)
    stbi_write_png_compression_level = 0;    // max quality
    stbi_write_png(full_filename.c_str(), width, height, channels, imageData, row_stride);
  } else if (extension == ".jpg") {
    int imageQuality = 100;  // max quality
    stbi_write_jpg(full_filename.c_str(), width, height, channels, imageData, imageQuality);
  } else if (extension == ".bmp") {
    stbi_write_bmp(full_filename.c_str(), width, height, channels, imageData);
  } else if (extension == ".tga") {
    stbi_write_tga(full_filename.c_str(), width, height, channels, imageData);
  }

  // Free imageData memory
  delete[] imageData;
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
  // Use ImGUI::SameLine to add multiple items next to each other
  ImGui::ShowDemoWindow();

  ImGui::Begin("Rendering Controls");

  // Use SetNextItemOpen() so set the default state of a node to be open. We could
  // also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
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
    if (ImGui::Checkbox("Use Miter Join Method (slow/experimental)", &useMiterJoins)) {
      UpdateOutlineMethod();
    }
    ImGui::Separator();

    ImGui::Text("Edge Width:");
    if (ImGui::SliderFloat("Pixels", &outline_thickness_, 0, 100, "%.1f")) {
      UpdateOutlineThickness();
    }
    ImGui::Separator();

    if (ImGui::Checkbox("Draw Silhouette Edges", &showSilhouette)) {
      UpdateSilhouetteStatus();
    }

    if (ImGui::Checkbox("Draw Crease Edges", &showCrease)) {
      UpdateCreaseStatus();
    }

    if (ImGui::Checkbox("Draw Border Edges", &showBorder)) {
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
    if (ImGui::Checkbox("Show Mesh", &showMesh)) {
      UpdateMeshVisibility();
    }
  }

  // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader("File Controls:")) {
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .2f);
    if (ImGui::Button("Save Image")) {
      renderingImageCountdown = 3;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Rendered file saves in working directory.");
      ImGui::EndTooltip();
    }

    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .4f);
    ImGui::SameLine();
    ImGui::InputTextWithHint(/*label = */ "", "filename", renderFilename,
                             IM_ARRAYSIZE(renderFilename));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * .15f);
    ImGui::Combo("filetype", &item_current, fileExtensions, IM_ARRAYSIZE(fileExtensions));
  }

  ImGui::End();
}

}  // namespace GLOO
