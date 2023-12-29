#include "OutlineNode.hpp"

#include <glm/gtx/string_cast.hpp>

#include "assignment_code/common/edgeutils.hpp"
#include "assignment_code/common/helpers.hpp"
#include "gloo/Material.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/MiterOutlineShader.hpp"
#include "gloo/shaders/OutlineShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/shaders/ToneMappingShader.hpp"
#include "gloo/shaders/ToonShader.hpp"

namespace GLOO {
OutlineNode::OutlineNode(const Scene* scene, const std::shared_ptr<VertexObject> mesh,
                         const std::shared_ptr<ShaderProgram> mesh_shader)
    : SceneNode(), parent_scene_(scene) {
  // Create things we need for rendering
  mesh_ = mesh == nullptr ? PrimitiveFactory::CreateCylinder(1.f, 1, 32) : mesh;
  // TODO debug miter join stuff with CreateQuad();

  // Since our mesh is static, we only need to set the outline mesh's vertex positions once
  // (we'll change the indices a lot though).
  SetOutlineMesh();
  DoRenderSetup(mesh_shader);
  // Populate mesh with default material
  mesh_node_->CreateComponent<MaterialComponent>(
      std::make_shared<Material>(Material::GetDefaultNPR()));

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
  if (mesh->HasNormals()) {
    mesh_->UpdateNormals(make_unique<NormalArray>(mesh->GetNormals()));
  } else {
    mesh_->UpdateNormals(CalculateNormals(mesh->GetPositions(), mesh->GetIndices()));
  }

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
  // mesh_node_->SetActive(false);

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
  // Create outline shader
  outline_shader_ = std::make_shared<OutlineShader>();
  CreateComponent<ShadingComponent>(outline_shader_);
  auto& rc_node = CreateComponent<RenderingComponent>(outline_mesh_);
  rc_node.SetDrawMode(DrawMode::Lines);

  // Create miter outline shader
  miter_outline_shader_ = std::make_shared<MiterOutlineShader>();

  // Material (white color lines)
  auto mat = std::make_shared<Material>();
  mat->SetDiffuseColor(glm::vec3(1.));  // white
  mat->SetOutlineThickness(4);  // Default thickness
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

void OutlineNode::SetIlluminatedColor(const glm::vec3& color) {
  auto material = mesh_node_->GetComponentPtr<MaterialComponent>()->GetMaterial();
  material.SetIlluminatedColor(color);
  mesh_node_->GetComponentPtr<MaterialComponent>()->SetMaterial(
      std::make_shared<Material>(material));
}

void OutlineNode::SetShadowColor(const glm::vec3& color) {
  auto material = mesh_node_->GetComponentPtr<MaterialComponent>()->GetMaterial();
  material.SetShadowColor(color);
  mesh_node_->GetComponentPtr<MaterialComponent>()->SetMaterial(
      std::make_shared<Material>(material));
}

void OutlineNode::SetOutlineColor(const glm::vec3& color) {
  auto material = GetComponentPtr<MaterialComponent>()->GetMaterial();
  material.SetOutlineColor(color);
  GetComponentPtr<MaterialComponent>()->SetMaterial(std::make_shared<Material>(material));
}

void OutlineNode::OverrideNPRColorsFromDiffuse(float illuminationFactor, float shadowFactor) {
  // Use the diffuse color of the material we have to set its shadow and illumination color
  auto material_component_ptr = mesh_node_->GetComponentPtr<MaterialComponent>();
  const Material* material_ptr;
  if (material_component_ptr == nullptr) {
    material_ptr = &Material::GetDefault();
  } else {
    material_ptr = &material_component_ptr->GetMaterial();
  }

  auto diffuseColor = material_ptr->GetDiffuseColor();
  SetIlluminatedColor(illuminationFactor * diffuseColor);
  SetShadowColor(shadowFactor * diffuseColor);
}

void OutlineNode::SetOutlineThickness(const float& width) {
  // Update material with new outline width
  auto material = GetComponentPtr<MaterialComponent>()->GetMaterial();
  material.SetOutlineThickness(width);
  GetComponentPtr<MaterialComponent>()->SetMaterial(std::make_shared<Material>(material));
}

void OutlineNode::SetOutlineMethod(OutlineMethod method) { outline_method_ = method; }

void OutlineNode::SetMeshVisibility(bool visible) { mesh_node_->SetActive(visible); }

void OutlineNode::CalculateFaceDirections() {
  // TODO: this is treated as an orthographic projection, try doing this with persepctive projection
  // Can try using projection matrix of camera?
  // Get camera information
  auto camera_pointer = parent_scene_->GetActiveCameraPtr();
  //  Get global camera direction by transforming its "z" vector into global coordinates
  glm::vec3 global_camera_direction =
      glm::vec3(glm::inverse(camera_pointer->GetViewMatrix()) * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

  // Transform global camera direction into object coordinates
  glm::vec3 local_camera_direction = glm::inverse(GetTransform().GetLocalToWorldMatrix()) *
                                     glm::vec4(global_camera_direction, 0.0f);
  // Iterate through faces and calculate if they're pointing towards or away the camera
  for (auto face : faces_) {
    face->front_facing = glm::dot(face->normal, local_camera_direction) >= 0;
  }
}

void OutlineNode::Update(double delta_time) {
  // Reset Polyline edge nodes
  // TODO: you can do better here but for now this is fine
  for (auto& edgeNode : edge_nodes_) {
    edgeNode->SetActive(false);
    // edgeNode->~SceneNode();
  }
  // std::cout << "Children: " << GetChildrenCount() << std::endl;
  // On each frame, recaclulate the silhouette edges and draw all update edges
  // Only recalculate silhouette edges when we're displaying them
  if (show_silhouette_edges_) {
    ComputeSilhouetteEdges();
  }
  RenderEdges();
}

void OutlineNode::RenderEdges() {
  auto newIndices = make_unique<IndexArray>();
  auto renderedEdges = std::vector<Edge>();

  // Only iterate through our edges if we're going to draw any of them
  if (show_silhouette_edges_ || show_border_edges_ || show_crease_edges_) {
    for (const auto& item : edge_info_map_) {
      Edge edge = item.first;
      EdgeInfo info = item.second;
      // Only draw edges that we allow
      if ((info.is_silhouette && show_silhouette_edges_) ||
          (info.is_border && show_border_edges_) || (info.is_crease && show_crease_edges_)) {
        // Toggle between rendering with miter joins and "fast" edge rendering
        if (outline_method_ == OutlineMethod::STANDARD) {
          newIndices->push_back(edge.first);
          newIndices->push_back(edge.second);
        } else if (outline_method_ == OutlineMethod::MITER) {
          // TODO: maybe render edges in passes?
          renderedEdges.push_back(edge);  // record edge for polyline drawing purposes
        }
      }
    }
  }
  // Polyline stuff
  auto polylines = edgesToPolylines(renderedEdges);
  auto& positions = outline_mesh_->GetPositions();
  auto material = GetComponentPtr<MaterialComponent>()->GetMaterial();
  // std::cout << "Num Polylines: " << polylines.size() << std::endl;
  for (int i = 0; i < polylines.size(); ++i) {
    // TODO turn into their own node class
    auto& polyline = polylines[i];
    // Reuse edge nodes if we can
    SceneNode* edgeNode;
    if (edge_nodes_.size() > i) {
      edgeNode = edge_nodes_[i];
      edgeNode->SetActive(true);
    } else {
      // Make a new edge node if we don't have enough
      auto newEdgeNode = make_unique<SceneNode>();
      edgeNode = newEdgeNode.get();
      edge_nodes_.push_back(newEdgeNode.get());
      AddChild(std::move(newEdgeNode));
    }
    // TODO 2-length paths don't show up!
    // (They do if you treat them as loops in edgeutils)
    // TODO: switch between miter join rendering and regular edge rendering based on path length
    // (i.e. don't do it for 2-length things)
    // TODO increase edge bias
    // Define the polyline size. If the polyline is a loop, there's technically another point from
    // the back to the front that wasn't incldued, making the polyline one vertex longer.
    auto polylineSize = polyline.is_loop ? polyline.path.size() + 1 : polyline.path.size();
    auto numVertices = 6 * (polylineSize - 1);
    // Create indices for polyline
    auto polyLineIndices = make_unique<IndexArray>();
    for (int i = 0; i < numVertices; ++i) {
      polyLineIndices->push_back(i);
    }
    // Create positions for polyline, adding extra vertices to head or tail if the polyline is a
    // loop
    auto firstElt = polyline.path.front();
    auto lastElt = polyline.path.back();
    if (polyline.is_loop) {
      auto secondElt = polyline.path[1];
      // Add first elt to the end, followed by the element immediately after it.
      polyline.path.push_back(firstElt);
      polyline.path.push_back(secondElt);
      // Add last element to the beginning
      polyline.path.insert(polyline.path.begin(), lastElt);
    } else {
      // Otherwise, just add the first element to the beginning and last element to the end
      polyline.path.push_back(lastElt);
      polyline.path.insert(polyline.path.begin(), firstElt);
    }
    auto polylinePositions = make_unique<PositionArray>();
    for (auto& index : polyline.path) {
      polylinePositions->push_back(positions[index]);
    }
    std::shared_ptr<VertexObject> polylineMesh = std::make_shared<VertexObject>();
    polylineMesh->UpdatePositions(std::move(polylinePositions));
    polylineMesh->UpdateIndices(std::move(polyLineIndices));

    edgeNode->CreateComponent<MaterialComponent>(std::make_shared<Material>(material));
    edgeNode->CreateComponent<ShadingComponent>(miter_outline_shader_);
    edgeNode->CreateComponent<RenderingComponent>(polylineMesh);

    // Print polyline information
    // std::cout << "Polyline: ";
    // for (size_t node : polyline.path) {
    //   std::cout << node << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "Loop: " << (polyline.is_loop ? "True" : "False");
    // std::cout << std::endl;
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
    std::shared_ptr<Face> face = std::make_shared<Face>();
    face->i1 = i1;
    face->i2 = i2;
    face->i3 = i3;
    face->normal = face_normal;

    faces_.push_back(face);

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
    glm::vec3 face1_n = faces[0]->normal;
    glm::vec3 face2_n = faces[1]->normal;
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
  CalculateFaceDirections();
  for (const auto& item : edge_face_map_) {
    Edge edge = item.first;
    auto faces = item.second;
    if (faces.size() != 2) {
      edge_info_map_[edge].is_silhouette = false;
      continue;
    }
    // Perform silhouette edge test
    if (faces[0]->front_facing != faces[1]->front_facing) {
      edge_info_map_[edge].is_silhouette = true;
    } else {
      edge_info_map_[edge].is_silhouette = false;
    }
  }
}
}  // namespace GLOO