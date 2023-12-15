#ifndef HELPERS_H_
#define HELPERS_H_

#include "gloo/utils.hpp"
#include "gloo/alias_types.hpp"

namespace GLOO {
std::unique_ptr<NormalArray> CalculateNormals(const PositionArray& positions,
                                              const IndexArray& indices);
glm::vec4 vectorToVec4(const std::vector<float>& floatVector);
glm::vec3 vectorToVec3(const std::vector<float>& floatVector);
}

#endif
