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

  // TODO 2-length paths don't show up!
  // (They do if you treat them as loops in edgeutils)
  // TODO: switch between miter join rendering and regular edge rendering based on path length
  // (i.e. don't do it for 2-length things)
  // TODO increase edge bias

  // Define the polyline size. If the polyline is a loop, there's technically another point from
  // the back to the front that wasn't incldued, making the polyline one vertex longer.
  auto polylineSize = polyline_.is_loop ? polyline_.path.size() + 1 : polyline_.path.size();
  auto numVertices = 6 * (polylineSize - 1);
  // Create indices for polyline
  auto polyLineIndices = make_unique<IndexArray>();
  for (int i = 0; i < numVertices; ++i) {
    polyLineIndices->push_back(i);
  }
  // Create positions for polyline, adding extra vertices to head or tail if the polyline is a
  // loop
  auto firstElt = polyline_.path.front();
  auto lastElt = polyline_.path.back();
  if (polyline_.is_loop) {
    auto secondElt = polyline_.path[1];
    // Add first elt to the end, followed by the element immediately after it.
    polyline_.path.push_back(firstElt);
    polyline_.path.push_back(secondElt);
    // Add last element to the beginning
    polyline_.path.insert(polyline_.path.begin(), lastElt);
  } else {
    // Otherwise, just add the first element to the beginning and last element to the end
    polyline_.path.push_back(lastElt);
    polyline_.path.insert(polyline_.path.begin(), firstElt);
  }
  auto polylinePositions = make_unique<PositionArray>();
  for (auto& index : polyline_.path) {
    polylinePositions->push_back(meshPositions[index]);
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