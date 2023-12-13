#include "OutlineNode.hpp"

#include <glm/gtx/string_cast.hpp>

#include "assignment_code/common/helpers.hpp"
#include "gloo/Material.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/shaders/ToneMappingShader.hpp"
#include "gloo/shaders/ToonShader.hpp"

namespace GLOO {
OutlineNode::OutlineNode(const Scene* scene, const std::shared_ptr<VertexObject> mesh,
                         const std::shared_ptr<ShaderProgram> mesh_shader)
    : SceneNode(), parent_scene_(scene) {
  // Create things we need for rendering
  mesh_ = mesh == nullptr ? PrimitiveFactory::CreateCylinder(1.f, 1, 32) : mesh;

  // Since our mesh is static, we only need to set the outline mesh's vertex positions once
  // (we'll change the indices a lot though).
  SetOutlineMesh();
  DoRenderSetup(mesh_shader);

  // Outline Specific Setup:
  SetupEdgeMaps();
  // Precompute Border and Crease Edges:
  ComputeBorderEdges();
  ComputeCreaseEdges();  // Note: this should be done in Update if the model geometry changes.
}

OutlineNode::OutlineNode(const Scene* scene, const std::shared_ptr<VertexObject> mesh,
                         const size_t startIndex, const size_t numIndices,
                         const std::shared_ptr<Material> mesh_material,
                         const std::shared_ptr<ShaderProgram> mesh_shader)
    : SceneNode(), parent_scene_(scene) {
  mesh_ = std::make_shared<VertexObject>();
  auto mesh_positions = mesh->GetPositions();
  mesh_->UpdatePositions(make_unique<PositionArray>(mesh_positions));
  mesh_->UpdateNormals(make_unique<NormalArray>(mesh->GetNormals()));

  // Slice the original mesh indices using the constructor parameters to get the edges we're
  // actually rendering
  auto mesh_indices = mesh->GetIndices();
  size_t startVertexIndex = startIndex;
  auto start = mesh_indices.begin() + startVertexIndex;
  auto end = mesh_indices.begin() + startVertexIndex + numIndices;
  IndexArray truncated_mesh_indices(start, end);
  mesh_->UpdateIndices(make_unique<IndexArray>(truncated_mesh_indices));

  SetOutlineMesh();
  DoRenderSetup(mesh_shader);
  // Add the specific material we have to this mesh
  mesh_node_->CreateComponent<MaterialComponent>(mesh_material);

  // Outline Specific Setup:
  SetupEdgeMaps();
  // Precompute Border and Crease Edges:
  ComputeBorderEdges();
  ComputeCreaseEdges();
}

void OutlineNode::SetOutlineMesh() {
  // Calculate normals if our mesh doesn't have any
  if (!mesh_->HasNormals()) {
    mesh_->UpdateNormals(CalculateNormals(mesh_->GetPositions(), mesh_->GetIndices()));
  }

  outline_mesh_ = std::make_shared<VertexObject>();
  // Offset the actual positions we'll use slightly in the vertex normal direction to prevent
  // z-fighting
  auto mesh_positions = mesh_->GetPositions();
  auto mesh_normals = (mesh_->GetNormals());
  for (int i = 0; i < mesh_positions.size(); i++) {
    mesh_positions[i] += mesh_normals[i] * line_bias_;
  }
  outline_mesh_->UpdatePositions(make_unique<PositionArray>(mesh_positions));
}

void OutlineNode::DoRenderSetup(std::shared_ptr<ShaderProgram> mesh_shader) {
  // Create new tone mapping shader if shader isn't specified
  mesh_shader_ = mesh_shader == nullptr ? std::make_shared<ToneMappingShader>() : mesh_shader;
  outline_shader_ = std::make_shared<SimpleShader>();
  CreateComponent<ShadingComponent>(outline_shader_);
  auto& rc_node = CreateComponent<RenderingComponent>(outline_mesh_);
  rc_node.SetDrawMode(DrawMode::Lines);

  // Material (white color lines)
  auto mat = std::make_shared<Material>();
  mat->SetDiffuseColor(glm::vec3(1.));
  CreateComponent<MaterialComponent>(mat);

  // Child Scene Node for actual mesh
  auto meshNode = make_unique<SceneNode>();
  meshNode->CreateComponent<RenderingComponent>(mesh_);
  meshNode->CreateComponent<ShadingComponent>(mesh_shader_);
  mesh_node_ = meshNode.get();
  AddChild(std::move(meshNode));
}

void OutlineNode::SetSilhouetteStatus(bool status) { show_silhouette_edges_ = status; }
void OutlineNode::SetCreaseStatus(bool status) { show_crease_edges_ = status; }
void OutlineNode::SetBorderStatus(bool status) { show_border_edges_ = status; }

void OutlineNode::ChangeMeshShader(ToonShadingType shadingType) {
  // Create entirely new shader and assign it to mesh
  if (shadingType == ToonShadingType::TOON) {
    mesh_shader_ = std::make_shared<ToonShader>();
  } else {
    mesh_shader_ = std::make_shared<ToneMappingShader>();
  }
  mesh_node_->GetComponentPtr<ShadingComponent>()->SetShader(mesh_shader_);
}

void OutlineNode::ChangeMeshShader(std::shared_ptr<ShaderProgram> shader) {
  // Directly substitute mesh shader for given shader
  mesh_shader_ = shader;
  mesh_node_->GetComponentPtr<ShadingComponent>()->SetShader(mesh_shader_);
}

void OutlineNode::SetCreaseThreshold(float degrees) {
  crease_threshold_ = glm::radians(degrees);
  ComputeCreaseEdges();
}

void OutlineNode::Update(double delta_time) {
  // On each frame, recaclulate the silhouette edges and draw all update edges
  // Only recalculate silhouette edges when we're displaying them
  if (show_silhouette_edges_) {
    ComputeSilhouetteEdges();
  }
  RenderEdges(show_silhouette_edges_, show_border_edges_, show_crease_edges_);
}

void OutlineNode::RenderEdges(bool silhouette, bool border, bool crease) {
  auto newIndices = make_unique<IndexArray>();

  // Only iterate through our edges if we're going to draw any of them
  if (silhouette || border || crease) {
    for (const auto& item : edge_info_map_) {
      Edge edge = item.first;
      EdgeInfo info = item.second;
      // Only draw edges that we allow
      if ((info.is_silhouette && silhouette) || (info.is_border && border) ||
          (info.is_crease && crease)) {
        newIndices->push_back(edge.first);
        newIndices->push_back(edge.second);
      }
    }
  }
  outline_mesh_->UpdateIndices(std::move(newIndices));
}

void PrintEdge(Edge edge) {
  std::cout << "[" << edge.first << ", " << edge.second << "]" << std::endl;
}

void OutlineNode::SetupEdgeMaps() {
  IndexArray indices = mesh_->GetIndices();
  PositionArray positions = mesh_->GetPositions();
  // Enforce Precondition, we should be dealing with a regular mesh.
  if (indices.size() % 3 != 0) {
    throw std::runtime_error("Mesh should be made fully out of triangles!");
  }

  // Traverse Indices of Groups of 3, processing a face each time
  for (size_t i = 0; i < indices.size(); i += 3) {
    size_t i1 = indices[i];
    size_t i2 = indices[i + 1];
    size_t i3 = indices[i + 2];

    // Face normal calculations
    auto& p1 = positions[i1];
    auto& p2 = positions[i2];
    auto& p3 = positions[i3];
    auto face_normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));

    // Define Face
    Face face;
    face.i1 = i1;
    face.i2 = i2;
    face.i3 = i3;
    face.normal = face_normal;

    // Define Edges on Face
    Edge edge1(i1, i2);
    Edge edge2(i2, i3);
    Edge edge3(i3, i1);
    std::vector<Edge> edges = {edge1, edge2, edge3};
    // Initialize edges in edge info and edge face maps
    for (auto& edge : edges) {
      // Edge info map
      if (edge_info_map_.count(edge) == 0) {
        EdgeInfo edge_info;
        edge_info.is_border = false;
        edge_info.is_crease = false;
        edge_info.is_silhouette = false;
        edge_info_map_[edge] = edge_info;
      }

      // Edge face map
      edge_face_map_[edge].push_back(face);
    }
  }
}

void OutlineNode::ComputeBorderEdges() {
  // From Lake et al. (2000), Border edges only lie on the edge of a single polygon
  // Note: if we had support for multiple materials
  // for an object we'd also have to account for that
  for (const auto& item : edge_face_map_) {
    Edge edge = item.first;
    auto faces = item.second;
    if (faces.size() == 1) {
      edge_info_map_[edge].is_border = true;
      //   std::cout << "Border Edge:";
      //   PrintEdge(edge);
    } else {
      edge_info_map_[edge].is_border = false;
    }
  }
}

void OutlineNode::ComputeCreaseEdges() {
  // From Lake et al. (2000), A crease edge is detected when the dihedral angle between two faces is
  // greater than a given threshold.
  for (const auto& item : edge_face_map_) {
    Edge edge = item.first;
    auto faces = item.second;
    if (faces.size() != 2) {
      edge_info_map_[edge].is_crease = false;
      continue;
    }
    glm::vec3 face1_n = faces[0].normal;
    glm::vec3 face2_n = faces[1].normal;
    float angleBetween =
        glm::acos(glm::dot(face1_n, face2_n) / (glm::length(face1_n) * glm::length(face2_n)));
    if (angleBetween > crease_threshold_) {
      edge_info_map_[edge].is_crease = true;
      //   std::cout << "Crease Edge:";
      //   PrintEdge(edge);
    } else {
      edge_info_map_[edge].is_crease = false;
    }
  }
}

void OutlineNode::ComputeSilhouetteEdges() {
  // From Lake et al. (2000), an edge is marked as a silhouette edge if a front-facing and a
  // back-facing polygon share the edge.
  auto camera_pointer = parent_scene_->GetActiveCameraPtr();
  glm::vec3 camera_position =
      glm::vec3(glm::inverse(camera_pointer->GetViewMatrix()) * glm::vec4(0, 0, 0, 1.0));
  glm::vec3 world_position = GetTransform().GetWorldPosition();
  glm::vec3 camera_direction = camera_position - world_position;
  for (const auto& item : edge_face_map_) {
    Edge edge = item.first;
    auto faces = item.second;
    if (faces.size() != 2) {
      edge_info_map_[edge].is_silhouette = false;
      continue;
    }
    // Perform silhouette edge test
    glm::vec3 face1_n = faces[0].normal;
    glm::vec3 face2_n = faces[1].normal;
    float silhouette_param =
        glm::dot(face1_n, camera_direction) * glm::dot(face2_n, camera_direction);
    if (silhouette_param <= 0) {
      edge_info_map_[edge].is_silhouette = true;
      //   std::cout << "Silhouette Edge:";
      //   PrintEdge(edge);
    } else {
      edge_info_map_[edge].is_silhouette = false;
    }
  }
}
}  // namespace GLOO