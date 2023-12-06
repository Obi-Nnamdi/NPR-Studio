#ifndef HELPERS_H_
#define HELPERS_H_

#include "gloo/utils.hpp"
#include "gloo/alias_types.hpp"

namespace GLOO {
std::unique_ptr<NormalArray> CalculateNormals(const PositionArray& positions,
                                              const IndexArray& indices);
}

#endif
