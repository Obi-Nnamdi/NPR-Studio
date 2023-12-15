#include "helpers.hpp"

namespace GLOO {
std::unique_ptr<NormalArray> CalculateNormals(const PositionArray& positions,
                                              const IndexArray& indices) {
  auto normals = make_unique<NormalArray>(positions.size(), glm::vec3(0.0f));

  for (size_t i = 0; i < indices.size(); i += 3) {
    int v1 = indices[i];
    int v2 = indices[i + 1];
    int v3 = indices[i + 2];
    auto& p1 = positions[v1];
    auto& p2 = positions[v2];
    auto& p3 = positions[v3];
    auto n = glm::cross(p2 - p1, p3 - p1);
    // No need to normalize here, since the norm of n is
    // proportional to the area.
    (*normals)[v1] += n;
    (*normals)[v2] += n;
    (*normals)[v3] += n;
  }

  for (size_t i = 0; i < normals->size(); i++) {
    (*normals)[i] = glm::normalize((*normals)[i]);
  }

  return normals;
}

glm::vec4 vectorToVec4(const std::vector<float>& floatVector) {
  if (floatVector.size() < 4) {
    throw std::runtime_error("Invalid vector size to convert to vec4");
  }
  return glm::vec4(floatVector[0], floatVector[1], floatVector[2], floatVector[3]);
}

glm::vec3 vectorToVec3(const std::vector<float>& floatVector) {
  if (floatVector.size() < 3) {
    throw std::runtime_error("Invalid vector size to convert to vec3");
  }
  return glm::vec3(floatVector[0], floatVector[1], floatVector[2]);
}
}  // namespace GLOO
