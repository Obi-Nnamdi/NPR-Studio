#ifndef GLOO_VERTEX_OBJECT_H_
#define GLOO_VERTEX_OBJECT_H_

#include "gloo/gl_wrapper/VertexArray.hpp"

namespace GLOO {
// Instances of this class store various vertex data and are responsible
// for sending data from CPU to GPU via the Update* methods.
class VertexObject {
 public:
  VertexObject() : vertex_array_(make_unique<VertexArray>()) {
  }

  // Vertex buffers are created in a lazy manner in the following Update*.
  void UpdatePositions(std::unique_ptr<PositionArray> positions);
  void UpdateNormals(std::unique_ptr<NormalArray> normals);
  void UpdateColors(std::unique_ptr<ColorArray> colors);
  void UpdateTexCoord(std::unique_ptr<TexCoordArray> tex_coords);
  void UpdateIndices(std::unique_ptr<IndexArray> indices);

  bool HasPositions() const {
    return positions_ != nullptr;
  }

  bool HasNormals() const {
    return normals_ != nullptr;
  }

  bool HasColors() const {
    return colors_ != nullptr;
  }

  bool HasTexCoors() const {
    return tex_coords_ != nullptr;
  }

  bool HasIndices() const {
    return indices_ != nullptr;
  }

  const PositionArray& GetPositions() const {
    if (positions_ == nullptr)
      throw std::runtime_error("No position in VertexObject!");
    return *positions_;
  }

  const NormalArray& GetNormals() const {
    if (normals_ == nullptr)
      throw std::runtime_error("No normal in VertexObject!");
    return *normals_;
  }

  const ColorArray& GetColors() const {
    if (colors_ == nullptr)
      throw std::runtime_error("No color in VertexObject!");
    return *colors_;
  }

  const TexCoordArray& GetTexCoords() const {
    if (tex_coords_ == nullptr)
      throw std::runtime_error("No texture coordinate in VertexObject!");
    return *tex_coords_;
  }

  const IndexArray& GetIndices() const {
    if (indices_ == nullptr)
      throw std::runtime_error("No indices in VertexObject!");
    return *indices_;
  }

  VertexArray& GetVertexArray() {
    return *vertex_array_.get();
  }
  const VertexArray& GetVertexArray() const {
    return *vertex_array_.get();
  }

 private:
  std::unique_ptr<VertexArray> vertex_array_;

  // Owner of vertex data.
  std::unique_ptr<PositionArray> positions_;
  std::unique_ptr<NormalArray> normals_;
  std::unique_ptr<ColorArray> colors_;
  std::unique_ptr<TexCoordArray> tex_coords_;
  std::unique_ptr<IndexArray> indices_;
};

}  // namespace GLOO

#endif
