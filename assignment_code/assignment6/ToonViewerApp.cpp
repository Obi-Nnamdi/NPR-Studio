#include "ToonViewerApp.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

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
}  // namespace

namespace GLOO {
ToonViewerApp::ToonViewerApp(const std::string& app_name, glm::ivec2 window_size,
                             const std::string& model_filename)
    : Application(app_name, window_size), model_filename_(model_filename) {
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

  // Add in a point light
  auto point_light = std::make_shared<PointLight>();
  point_light->SetAttenuation(glm::vec3(0.3));
  point_light->SetDiffuseColor(glm::vec3(1.));
  auto point_node = make_unique<SceneNode>();
  point_node->GetTransform().SetPosition(glm::vec3(0, 2, 0));
  point_node->CreateComponent<LightComponent>(point_light);
  // root.AddChild(std::move(point_node));

  // Create outline node
  auto outline_node = make_unique<OutlineNode>(scene_.get());
  outline_nodes_.push_back(outline_node.get());
  root.AddChild(std::move(outline_node));

  // Create shader instance
  auto shader = std::make_shared<ToneMappingShader>();
  // Load and set up the scene OBJ if we have a specified model file.
  // If not, load up a basic sphere.
  if (model_filename_ != "") {
    MeshData mesh_data = MeshLoader::Import(model_filename_);
    SetAmbientToDiffuse(mesh_data);

    std::shared_ptr<VertexObject> vertex_obj = std::move(mesh_data.vertex_obj);

    // Create scene nodes for each mesh group
    for (MeshGroup& group : mesh_data.groups) {
      auto mesh_node = make_unique<SceneNode>();
      mesh_node->CreateComponent<ShadingComponent>(shader);
      mesh_node->CreateComponent<MaterialComponent>(group.material);
      auto& renderComponent = mesh_node->CreateComponent<RenderingComponent>(vertex_obj);
      // Query the mesh data to only draw the part of the larger mesh belonging to this node
      renderComponent.SetDrawRange(group.start_face_index, group.num_indices);
      root.AddChild(std::move(mesh_node));
    }
  } else {
    std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(2.f, 64, 64);
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    // root.AddChild(std::move(sphere_node));
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

void ToonViewerApp::DrawGUI() {
  // Dear ImGUI documentation at https://github.com/ocornut/imgui?tab=readme-ov-file#usage
  ImGui::Begin("Control Panel");
  ImGui::Text("Lighting Controls:");
  if (ImGui::Button("Toggle Light Type")) {
    sun_node_->ToggleLight();
  }
  ImGui::Text("Shader Controls:");
  if (ImGui::Button("Toggle Toon/Tone Mapping Shader")) {
    ToggleShading();
  }
  ImGui::End();
}

}  // namespace GLOO
