#ifndef GLOO_PRIMITIVE_FACTORY_H_
#define GLOO_PRIMITIVE_FACTORY_H_

#include "gloo/VertexObject.hpp"

namespace GLOO {
class PrimitiveFactory {
 public:
  // Create a radius-r sphere with prescribed vertical/horizontal numbers
  // of slices/stacks.
  static std::unique_ptr<VertexObject> CreateSphere(float r,
                                                    size_t slices,
                                                    size_t stacks);

  // Create a cylinder with prescribed radius, height, and number of sides.
  static std::unique_ptr<VertexObject> CreateCylinder(float r,
                                                      float h,
                                                      size_t num_sides);

  static std::unique_ptr<VertexObject> CreateQuad();

  // Create a line segment between p and q.
  static std::unique_ptr<VertexObject> CreateLineSegment(const glm::vec3& p,
                                                         const glm::vec3& q);
};
}  // namespace GLOO

#endif
