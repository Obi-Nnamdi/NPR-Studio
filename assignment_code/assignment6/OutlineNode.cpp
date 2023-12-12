#include "OutlineNode.hpp"

#include <glm/gtx/string_cast.hpp>

#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/shaders/ToneMappingShader.hpp"

namespace GLOO {
OutlineNode::OutlineNode() : SceneNode() {
  // Create things we need for rendering
  // TODO: change constructor to allow custom imports
  mesh_ = PrimitiveFactory::CreateCylinder(1.f, 1, 32);
  //   mesh_ = PrimitiveFactory::CreateQuad();

  // Since our mesh is static, we only need to set the outline mesh's vertex positions once
  // (we'll change the indices a lot though).
  outline_mesh_ = std::make_shared<VertexObject>();

  // TODO: Helper Function lol
  // Offset the actual positions we'll use slightly in the vertex normal direction to prevent
  // z-fighting
  auto mesh_positions = mesh_->GetPositions();
  auto mesh_normals = (mesh_->GetNormals());
  for (int i = 0; i < mesh_positions.size(); i++) {
    mesh_positions[i] += mesh_normals[i] * line_bias_;
  }
  outline_mesh_->UpdatePositions(make_unique<PositionArray>(mesh_positions));
  auto& rc_node = CreateComponent<RenderingComponent>(outline_mesh_);
  rc_node.SetDrawMode(DrawMode::Lines);

  mesh_shader_ = std::make_shared<ToneMappingShader>();
  outline_shader_ = std::make_shared<SimpleShader>();
  CreateComponent<ShadingComponent>(outline_shader_);

  // Material
  auto mat = std::make_shared<Material>();
  mat->SetDiffuseColor(glm::vec3(1.));
  CreateComponent<MaterialComponent>(mat);

  // Child Scene Node for actual mesh
  auto meshNode = make_unique<SceneNode>();
  meshNode->CreateComponent<RenderingComponent>(mesh_);
  meshNode->CreateComponent<ShadingComponent>(mesh_shader_);
  AddChild(std::move(meshNode));

  // Outline Specific Setup:
  SetupEdgeMaps();
  // Precompute Border and Crease Edges:
  ComputeBorderEdges();
  ComputeCreaseEdges();  // Note: this should be done in Update if the model geometry changes.
}

void OutlineNode::Update(double delta_time) { RenderEdges(); }

void OutlineNode::RenderEdges(bool silhouette, bool border, bool crease) {
  auto newIndices = make_unique<IndexArray>();
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
      } else {
        std::cout << "Old Edge" << std::endl;
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
      std::cout << "Border Edge:";
      PrintEdge(edge);
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
      continue;
    }
    glm::vec3 face1_n = faces[0].normal;
    glm::vec3 face2_n = faces[1].normal;
    float angleBetween =
        glm::acos(glm::dot(face1_n, face2_n) / (glm::length(face1_n) * glm::length(face2_n)));
    if (angleBetween > crease_threshold_) {
      edge_info_map_[edge].is_crease = true;
      std::cout << "Crease Edge:";
      PrintEdge(edge);
    }
  }
}
}  // namespace GLOO