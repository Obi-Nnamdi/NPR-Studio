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
  mesh_shader_ = std::make_shared<ToneMappingShader>();
  outline_shader_ = std::make_shared<SimpleShader>();
  SetupEdgeMaps();
  // Precompute Border and Crease Edges:
  ComputeBorderEdges();
}

void OutlineNode::Update(double delta_time) {}

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
}  // namespace GLOO