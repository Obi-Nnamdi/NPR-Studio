#ifndef POLYLINE_NODE_H_
#define POLYLINE_NODE_H_

#include "gloo/Material.hpp"
#include "gloo/Scene.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

namespace GLOO {
/**
 * Struct defining a polyline with a path, that can also be a loop.
 * A polyline with a loop won't have any of its path indices duplicated,
 * but its first element and last element are connected.
 */
struct Polyline {
  std::vector<size_t> path;
  bool is_loop;
};

/**
 * Class representing a node that renders a single polyline.
 */
class PolylineNode : public SceneNode {
 public:
  /**
   * Creates a polyline node from a polyline and a mesh.
   *
   * @param polyline The polyline to render.
   * @param meshPositions The positions of the mesh vertices.
   * @param material The material to use for rendering. If nullptr, a default
   * material will be used.
   * @param shader The shader to use for rendering. If nullptr, a default
   * shader will be used.
   */
  PolylineNode(const Polyline& polyline, const PositionArray& meshPositions,
               const std::shared_ptr<Material>& material = nullptr,
               const std::shared_ptr<ShaderProgram>& shader = nullptr);

  /**
   * Updates rendered polyline with new positions.
   * @param polyline The polyline to render.
   * @param meshPositions The positions of the mesh vertices.
   */
  void SetPolyline(const Polyline& polyline, const PositionArray& meshPositions);
  void SetMaterial(const std::shared_ptr<Material>& material);
  void SetShader(const std::shared_ptr<ShaderProgram>& shader);

 private:
  Polyline polyline_;
};
}  // namespace GLOO
#endif