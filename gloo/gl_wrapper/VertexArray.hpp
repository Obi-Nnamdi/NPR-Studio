#ifndef GLOO_VERTEX_ARRAY_OBJECT_H_
#define GLOO_VERTEX_ARRAY_OBJECT_H_

#include "IBindable.hpp"

#include "gloo/external.hpp"
#include "gloo/alias_types.hpp"
#include "VertexBuffer.hpp"

namespace GLOO {
enum class DrawMode { Triangles, Lines };

enum class PolygonMode { Wireframe, Fill };

class VertexArray : public IBindable {
 public:
  VertexArray();
  ~VertexArray();

  VertexArray(const VertexArray&) = delete;
  VertexArray& operator=(const VertexArray&) = delete;

  // Allow both move-construct and move-assign.
  VertexArray(VertexArray&& other) noexcept;
  VertexArray& operator=(VertexArray&& other) noexcept;

  void Bind() const override;
  void Unbind() const override;

  void CreatePositionBuffer();
  void CreateNormalBuffer();
  void CreateColorBuffer();
  void CreateTexCoordBuffer();
  void CreateIndexBuffer();
  void UpdatePositions(const PositionArray& positions) const;
  void UpdateNormals(const NormalArray& normals) const;
  void UpdateColors(const ColorArray& colors) const;
  void UpdateTexCoords(const TexCoordArray& tex_coords) const;
  void UpdateIndices(const IndexArray& indices) const;
  void LinkPositionBuffer(GLuint attr_idx) const;
  void LinkNormalBuffer(GLuint attr_idx) const;
  void LinkColorBuffer(GLuint attr_idx) const;
  void LinkTexCoordBuffer(GLuint attr_idx) const;

  bool HasPositionBuffer() const {
    return pos_buf_ != nullptr;
  }

  bool HasNormalBuffer() const {
    return normal_buf_ != nullptr;
  }

  bool HasColorBuffer() const {
    return color_buf_ != nullptr;
  }

  bool HasTexCoordBuffer() const {
    return tex_coord_buf_ != nullptr;
  }

  bool HasIndexBuffer() const {
    return idx_buf_ != nullptr;
  }

  void SetDrawMode(DrawMode mode);
  void SetPolygonMode(PolygonMode mode);
  void Render(size_t start_index, size_t num_indices) const;
  void Render() const;

 private:
  // Buffers are invisible to the outside.
  using PositionBuffer = VertexBuffer<glm::vec3, GL_ARRAY_BUFFER>;
  using NormalBuffer = VertexBuffer<glm::vec3, GL_ARRAY_BUFFER>;
  using ColorBuffer = VertexBuffer<glm::vec4, GL_ARRAY_BUFFER>;
  using TexCoordBuffer = VertexBuffer<glm::vec2, GL_ARRAY_BUFFER>;
  using IndexBuffer = VertexBuffer<unsigned int, GL_ELEMENT_ARRAY_BUFFER>;

  std::unique_ptr<PositionBuffer> pos_buf_;
  std::unique_ptr<NormalBuffer> normal_buf_;
  std::unique_ptr<ColorBuffer> color_buf_;
  std::unique_ptr<TexCoordBuffer> tex_coord_buf_;
  std::unique_ptr<IndexBuffer> idx_buf_;

  DrawMode draw_mode_;
  PolygonMode polygon_mode_;
  GLuint handle_{GLuint(-1)};
};
}  // namespace GLOO

#endif
