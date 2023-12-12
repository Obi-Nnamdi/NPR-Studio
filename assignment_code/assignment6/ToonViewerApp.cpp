#include "ToonViewerApp.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

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
    : Application(app_name, window_size), model_filename_(model_filename) {}

void ToonViewerApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  // Setting up the camera. PLEASE DO NOT MODIFY THE INITIAL CAMERA TRANSFORM.
  auto camera_node = make_unique<ArcBallCameraNode>(50.0f, 1.0f, 10.0f);
  // camera_node->GetTransform().SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
  camera_node->GetTransform().SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), kPi / 2);
  camera_node->Calibrate();
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  // Add in the ambient light so the shadowed areas won't be completely black.
  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.15f));
  auto ambient_node = make_unique<SceneNode>();
  ambient_node->CreateComponent<LightComponent>(ambient_light);
  // root.AddChild(std::move(ambient_node));

  // Add in the directional light as the sun.
  // root.AddChild(make_unique<SunNode>());

  // Add in a point light
  auto point_light = std::make_shared<PointLight>();
  point_light->SetAttenuation(glm::vec3(0.3));
  point_light->SetDiffuseColor(glm::vec3(1.));
  auto point_node = make_unique<SceneNode>();
  point_node->GetTransform().SetPosition(glm::vec3(0, 2, 0));
  point_node->CreateComponent<LightComponent>(point_light);
  root.AddChild(std::move(point_node));

  // Create shader instance
  auto shader = std::make_shared<ToonShader>();
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
    root.AddChild(std::move(sphere_node));
  }
}

}  // namespace GLOO
