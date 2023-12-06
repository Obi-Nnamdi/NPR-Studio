#include "RenderingComponent.hpp"

#include <stdexcept>
#include <iostream>

namespace GLOO {
RenderingComponent::RenderingComponent(std::shared_ptr<VertexObject> vertex_obj)
    : vertex_obj_(std::move(vertex_obj)) {
  if (!vertex_obj_->HasIndices() && !vertex_obj_->HasPositions()) {
    throw std::runtime_error(
        "Cannot initialize a "
        "RenderingComponent with a VertexObject without positions!");
  }

  // We use -1 to indicate the entire range of indices/positions.
  start_index_ = -1;
  num_indices_ = -1;
}

void RenderingComponent::SetDrawRange(int start_index, int num_indices) {
  start_index_ = start_index;
  num_indices_ = num_indices;
}

void RenderingComponent::Render() const {
  if (vertex_obj_ == nullptr) {
    throw std::runtime_error(
        "Rendering component has no vertex object attached!");
  }
  if (start_index_ >= 0 && num_indices_ > 0) {
    vertex_obj_->GetVertexArray().Render(static_cast<size_t>(start_index_),
                                         static_cast<size_t>(num_indices_));
  } else {
    if (vertex_obj_->HasIndices())
      vertex_obj_->GetVertexArray().Render(0, vertex_obj_->GetIndices().size());
    else
      vertex_obj_->GetVertexArray().Render(0,
                                           vertex_obj_->GetPositions().size());
  }
}

void RenderingComponent::SetDrawMode(DrawMode mode) {
  if (vertex_obj_ == nullptr) {
    throw std::runtime_error(
        "Rendering component has no vertex object attached!");
  }
  vertex_obj_->GetVertexArray().SetDrawMode(mode);
}

void RenderingComponent::SetPolygonMode(PolygonMode mode) {
  if (vertex_obj_ == nullptr) {
    throw std::runtime_error(
        "Rendering component has no vertex object attached!");
  }
  vertex_obj_->GetVertexArray().SetPolygonMode(mode);
}

void RenderingComponent::SetVertexObject(
    std::shared_ptr<VertexObject> vertex_obj) {
  vertex_obj_ = vertex_obj;
}
}  // namespace GLOO
