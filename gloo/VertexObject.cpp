#include "VertexObject.hpp"

#include <memory>
#include <iostream>
#include <stdexcept>

#include "gloo/gl_wrapper/BindGuard.hpp"
#include "gloo/SceneNode.hpp"

namespace GLOO {
void VertexObject::UpdatePositions(std::unique_ptr<PositionArray> positions) {
  if (positions_ == nullptr) {
    vertex_array_->CreatePositionBuffer();
  }
  positions_ = std::move(positions);
  vertex_array_->UpdatePositions(*positions_);
}

void VertexObject::UpdateIndices(std::unique_ptr<IndexArray> indices) {
  if (indices_ == nullptr) {
    vertex_array_->CreateIndexBuffer();
  }
  indices_ = std::move(indices);
  vertex_array_->UpdateIndices(*indices_);
}

void VertexObject::UpdateNormals(std::unique_ptr<NormalArray> normals) {
  if (normals_ == nullptr) {
    vertex_array_->CreateNormalBuffer();
  }
  normals_ = std::move(normals);
  vertex_array_->UpdateNormals(*normals_);
}

void VertexObject::UpdateColors(std::unique_ptr<ColorArray> colors) {
  if (colors_ == nullptr) {
    vertex_array_->CreateColorBuffer();
  }
  colors_ = std::move(colors);
  vertex_array_->UpdateColors(*colors_);
}

void VertexObject::UpdateTexCoord(std::unique_ptr<TexCoordArray> tex_coords) {
  if (tex_coords_ == nullptr) {
    vertex_array_->CreateTexCoordBuffer();
  }
  tex_coords_ = std::move(tex_coords);
  vertex_array_->UpdateTexCoords(*tex_coords_);
}

}  // namespace GLOO
