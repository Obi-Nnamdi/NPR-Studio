#include "PolylineNode.hpp"

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

PolylineNode::PolylineNode(const Polyline& polyline, const PositionArray& meshPositions,
                           const std::shared_ptr<Material>& material,
                           const std::shared_ptr<ShaderProgram>& shader) {
  SetPolyline(polyline, meshPositions);
  // Add materials and shader, using default versions if they're nullptr.
  if (material != nullptr) {
    CreateComponent<MaterialComponent>(material);
  } else {
    CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefaultNPR()));
  }
  if (shader != nullptr) {
    CreateComponent<ShadingComponent>(shader);
  } else {
    CreateComponent<ShadingComponent>(std::make_shared<MiterOutlineShader>());
  }
  // Print polyline information
  // std::cout << "Polyline: ";
  // for (size_t node : polyline.path) {
  //   std::cout << node << " ";
  // }
  // std::cout << std::endl;
  // std::cout << "Loop: " << (polyline.is_loop ? "True" : "False");
  // std::cout << std::endl;
}

void PolylineNode::SetPolyline(const Polyline& polyline, const PositionArray& meshPositions) {
  // Copy polyline over.
  polyline_.path = polyline.path;
  polyline_.is_loop = polyline.is_loop;

  // TODO we currently have a problem where the miter joins intersect the existing model geometry
  // and get partially rendered behind it, which might mean we need to increase edge bias? Doesn't
  // seem like it does mcuh though. This is pretty visible in border edges. (Lamp.obj and the
  // default cylinder)

  //   TODO multiple connections between vertices aren't represented properly with our polyline
  //   method.

  // Define the polyline size. If the polyline is a loop, there's technically another point from
  // the back to the front that wasn't incldued, making the polyline one vertex longer.
  auto polylineSize = polyline_.is_loop ? polyline_.path.size() + 1 : polyline_.path.size();
  auto numVertices =
      6 * (polylineSize - 1);  // 6 vertices are rendered in the shader per polyline segment.
  // Create indices for polyline
  auto polyLineIndices = make_unique<IndexArray>();
  auto polylinePositions = make_unique<PositionArray>();
  for (int i = 0; i < numVertices; ++i) {
    polyLineIndices->push_back(i);
  }
  // Create positions for polyline, adding extra vertices to head or tail if
  // the polyline is a loop. Note that the first and last indices of the final position array act as
  // the tangents of the first and last line segments.
  auto firstElt = polyline_.path.front();
  auto lastElt = polyline_.path.back();
  if (polyline_.is_loop) {
    auto secondElt = polyline_.path[1];
    // Add first elt to the end, followed by the element immediately after it.
    polyline_.path.push_back(firstElt);
    polyline_.path.push_back(secondElt);
    // Add last element to the beginning
    polyline_.path.insert(polyline_.path.begin(), lastElt);
    // Populate Polyline Indices
    for (auto& index : polyline_.path) {
      polylinePositions->push_back(meshPositions[index]);
    }
  } else {
    // Populate Polyline Indices based on original path
    for (auto& index : polyline_.path) {
      polylinePositions->push_back(meshPositions[index]);
    }
    // Use the slopes of the line segements connecting to the first and last elements of the
    // polyline path to populate the first and last points of the position array
    auto firstEltPos = meshPositions[firstElt];
    auto secondEltPos = meshPositions[polyline_.path[1]];
    auto lastEltPos = meshPositions[lastElt];
    auto secondToLastEltPos = meshPositions[polyline_.path[polyline_.path.size() - 2]];
    glm::vec3 firstSlope = glm::normalize(secondEltPos - firstEltPos);
    glm::vec3 lastSlope = glm::normalize(lastEltPos - secondToLastEltPos);

    // Insert the new points.
    polylinePositions->push_back(lastEltPos + lastSlope);
    polylinePositions->insert(polylinePositions->begin(), firstEltPos - firstSlope);
  }

  // Update or create a vertex object for the positions/indices.
  auto renderingComponentPtr = GetComponentPtr<RenderingComponent>();
  if (renderingComponentPtr != nullptr) {
    // Update the vertex object
    GetComponentPtr<RenderingComponent>()->GetVertexObjectPtr()->UpdatePositions(
        std::move(polylinePositions));
    GetComponentPtr<RenderingComponent>()->GetVertexObjectPtr()->UpdateIndices(
        std::move(polyLineIndices));
  } else {
    // Create a new vertex object for the polyline
    std::shared_ptr<VertexObject> polylineMesh = std::make_shared<VertexObject>();
    polylineMesh->UpdatePositions(std::move(polylinePositions));
    polylineMesh->UpdateIndices(std::move(polyLineIndices));

    CreateComponent<RenderingComponent>(std::move(polylineMesh));
  }
}
void PolylineNode::SetMaterial(const std::shared_ptr<Material>& material) {
  auto materialComponentPtr = GetComponentPtr<MaterialComponent>();
  if (materialComponentPtr != nullptr) {
    GetComponentPtr<MaterialComponent>()->SetMaterial(material);
  } else {
    CreateComponent<MaterialComponent>(material);
  }
}
void PolylineNode::SetShader(const std::shared_ptr<ShaderProgram>& shader) {
  GetComponentPtr<ShadingComponent>()->SetShader(shader);
}
}  // namespace GLOO