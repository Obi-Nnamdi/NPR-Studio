#include "ToonViewerApp.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../common/helpers.hpp"
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

void SetNPRColorsFromDiffuse(GLOO::MeshData& mesh_data, float illuminationFactor = 1.5,
                             float shadowFactor = .5) {
  // For groups that don't have shadow/illuminated colors, we use their diffuse color * 1.5 as the
  // illuminated color, and the diffuse color * .5 as the shadow color.
  for (auto& g : mesh_data.groups) {
    if (glm::length(g.material->GetIlluminatedColor()) < 1e-3) {
      g.material->SetIlluminatedColor(illuminationFactor * g.material->GetDiffuseColor());
    }
    if (glm::length(g.material->GetShadowColor()) < 1e-3) {
      g.material->SetShadowColor(shadowFactor * g.material->GetDiffuseColor());
    }
  }
}
}  // namespace

namespace GLOO {
ToonViewerApp::ToonViewerApp(const std::string& app_name, glm::ivec2 window_size,
                             const std::string& model_filename)
    : Application(app_name, window_size), model_filename_(model_filename) {
  background_color_ = {0, 0, 0, 1};
  illumination_color_ = {0, 0, 0};
  shadow_color_ = {0, 0, 0};
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
    SetNPRColorsFromDiffuse(mesh_data, 1.2, .5);

    std::shared_ptr<VertexObject> vertex_obj = std::move(mesh_data.vertex_obj);
    // TODO: fix up materials

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

void ToonViewerApp::OverrideNPRColorsFromDiffuse(float illuminationFactor, float shadowFactor) {
  for (auto node : outline_nodes_) {
    node->OverrideNPRColorsFromDiffuse(illuminationFactor, shadowFactor);
  }
}

void ToonViewerApp::DrawGUI() {
  // Dear ImGUI documentation at https://github.com/ocornut/imgui?tab=readme-ov-file#usage
  ImGui::Begin("Control Panel");
  ImGui::Text("Lighting Controls:");
  // Button for toggling light visibility
  if (ImGui::Button("Toggle Light Type")) {
    sun_node_->ToggleLight();
  }

  ImGui::Text("Shader Controls:");
  // Controls background color of our scene
  if (ImGui::ColorEdit4("Background Color", &background_color_.front())) {
    SetBackgroundColor(vectorToVec4(background_color_));
  }
  if (ImGui::ColorEdit3("Illumination Color", &illumination_color_.front())) {
    SetIlluminatedColor(vectorToVec3(illumination_color_));
  }
  if (ImGui::ColorEdit3("Shadow Color", &shadow_color_.front())) {
    SetShadowColor(vectorToVec3(shadow_color_));
  }
  if (ImGui::Button("Reset Shader Colors to Material Diffuse")) {
    OverrideNPRColorsFromDiffuse(1.2, 0.5);
  }
  // Button for toggling between our shader types
  if (ImGui::Button("Toggle Toon/Tone Mapping Shader")) {
    ToggleShading();
  }

  // Checkboxes for toggling edge type displays
  ImGui::Text("Edge Controls:");
  if (ImGui::Checkbox("Draw Silhouette Edges", &showSilhouette)) {
    UpdateSilhouetteStatus();
  }

  if (ImGui::Checkbox("Draw Crease Edges", &showCrease)) {
    UpdateCreaseStatus();
  }

  if (ImGui::Checkbox("Draw Border Edges", &showBorder)) {
    UpdateBorderStatus();
  }

  ImGui::Text("Crease Threshold:");
  // TODO: should this go to 360?
  if (ImGui::SliderFloat("Degrees", &crease_threshold_, 0, 180, "%.1f")) {
    UpdateCreaseThreshold();
  }

  ImGui::End();
}

}  // namespace GLOO
