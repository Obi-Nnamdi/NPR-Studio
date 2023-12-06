#ifndef GLOO_ALIAS_TYPES_H_
#define GLOO_ALIAS_TYPES_H_

#include <vector>

#include <glm/glm.hpp>

namespace GLOO {
using PositionArray = std::vector<glm::vec3>;
using NormalArray = std::vector<glm::vec3>;
using ColorArray = std::vector<glm::vec4>;
using TexCoordArray = std::vector<glm::vec2>;
using IndexArray = std::vector<unsigned int>;
}  // namespace GLOO

#endif
