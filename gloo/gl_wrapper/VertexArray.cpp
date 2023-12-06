#include "VertexArray.hpp"

#include <iostream>

#include "BindGuard.hpp"
#include "gloo/utils.hpp"

namespace GLOO {
VertexArray::VertexArray()
    : draw_mode_(DrawMode::Triangles), polygon_mode_(PolygonMode::Fill) {
  GL_CHECK(glGenVertexArrays(1, &handle_));
}

VertexArray::~VertexArray() {
  if (handle_ != GLuint(-1))
    GL_CHECK(glDeleteVertexArrays(1, &handle_));
}

VertexArray::VertexArray(VertexArray&& other) noexcept {
  handle_ = other.handle_;
  other.handle_ = GLuint(-1);

  pos_buf_ = std::move(other.pos_buf_);
  normal_buf_ = std::move(other.normal_buf_);
  color_buf_ = std::move(other.color_buf_);
  tex_coord_buf_ = std::move(other.tex_coord_buf_);
  idx_buf_ = std::move(other.idx_buf_);
  draw_mode_ = other.draw_mode_;
  polygon_mode_ = other.polygon_mode_;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
  handle_ = other.handle_;
  other.handle_ = GLuint(-1);

  pos_buf_ = std::move(other.pos_buf_);
  normal_buf_ = std::move(other.normal_buf_);
  color_buf_ = std::move(other.color_buf_);
  tex_coord_buf_ = std::move(other.tex_coord_buf_);
  idx_buf_ = std::move(other.idx_buf_);
  draw_mode_ = other.draw_mode_;
  polygon_mode_ = other.polygon_mode_;
  return *this;
}

void VertexArray::Bind() const {
  GL_CHECK(glBindVertexArray(handle_));
}

void VertexArray::Unbind() const {
  GL_CHECK(glBindVertexArray(0));
}

void VertexArray::CreatePositionBuffer() {
  pos_buf_ = make_unique<PositionBuffer>(GL_STATIC_DRAW);
}

void VertexArray::CreateNormalBuffer() {
  normal_buf_ = make_unique<NormalBuffer>(GL_STATIC_DRAW);
}

void VertexArray::CreateColorBuffer() {
  color_buf_ = make_unique<ColorBuffer>(GL_STATIC_DRAW);
}

void VertexArray::CreateTexCoordBuffer() {
  tex_coord_buf_ = make_unique<TexCoordBuffer>(GL_STATIC_DRAW);
}

void VertexArray::CreateIndexBuffer() {
  idx_buf_ = make_unique<IndexBuffer>(GL_STATIC_DRAW);
  BindGuard vao_bg(this);
  // Different from other types of vertex buffers, EBOs should not be unbounded.
  idx_buf_->Bind();
}

void VertexArray::UpdatePositions(const PositionArray& positions) const {
  pos_buf_->Update(positions);
}

void VertexArray::UpdateNormals(const NormalArray& normals) const {
  normal_buf_->Update(normals);
}

void VertexArray::UpdateColors(const ColorArray& colors) const {
  color_buf_->Update(colors);
}

void VertexArray::UpdateTexCoords(const TexCoordArray& tex_coords) const {
  tex_coord_buf_->Update(tex_coords);
}

void VertexArray::UpdateIndices(const IndexArray& indices) const {
  idx_buf_->Update(indices);
}

void VertexArray::LinkPositionBuffer(GLuint attr_idx) const {
  BindGuard vao_bg(this);
  BindGuard buf_bg(pos_buf_.get());
  // The line below attaches the vertex buffer to the VAO.
  GL_CHECK(glVertexAttribPointer(attr_idx, 3, GL_FLOAT, GL_FALSE, 0, 0));
  GL_CHECK(glEnableVertexAttribArray(attr_idx));
}

void VertexArray::LinkNormalBuffer(GLuint attr_idx) const {
  BindGuard vao_bg(this);
  BindGuard buf_bg(normal_buf_.get());
  // The line below attaches the vertex buffer to the VAO.
  GL_CHECK(glVertexAttribPointer(attr_idx, 3, GL_FLOAT, GL_FALSE, 0, 0));
  GL_CHECK(glEnableVertexAttribArray(attr_idx));
}

void VertexArray::LinkColorBuffer(GLuint attr_idx) const {
  BindGuard vao_bg(this);
  BindGuard buf_bg(color_buf_.get());
  // The line below attaches the vertex buffer to the VAO.
  GL_CHECK(glVertexAttribPointer(attr_idx, 4, GL_FLOAT, GL_FALSE, 0, 0));
  GL_CHECK(glEnableVertexAttribArray(attr_idx));
}

void VertexArray::LinkTexCoordBuffer(GLuint attr_idx) const {
  BindGuard vao_bg(this);
  BindGuard buf_bg(tex_coord_buf_.get());
  // The line below attaches the vertex buffer to the VAO.
  GL_CHECK(glVertexAttribPointer(attr_idx, 2, GL_FLOAT, GL_FALSE, 0, 0));
  GL_CHECK(glEnableVertexAttribArray(attr_idx));
}

void VertexArray::SetDrawMode(DrawMode mode) {
  draw_mode_ = mode;
}

void VertexArray::SetPolygonMode(PolygonMode mode) {
  polygon_mode_ = mode;
}

void VertexArray::Render(size_t start_index, size_t num_indices) const {
  // WARNING: need to declare stack variables before RAII
  // to avoid alignment issues.

  BindGuard vao_bg(this);

  if (polygon_mode_ == PolygonMode::Wireframe) {
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
  } else {
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  }

  GLint draw_mode = draw_mode_ == DrawMode::Triangles ? GL_TRIANGLES : GL_LINES;

  if (idx_buf_ != nullptr) {
    GL_CHECK(glDrawElements(
        draw_mode, static_cast<GLsizei>(num_indices), GL_UNSIGNED_INT,
        reinterpret_cast<void*>(start_index * sizeof(unsigned int))));
  } else {
    GL_CHECK(glDrawArrays(draw_mode, (GLint)start_index, (GLsizei)num_indices));
  }
}

void VertexArray::Render() const {
  if (idx_buf_ != nullptr)
    Render(0, idx_buf_->GetSize());
  else {
    if (pos_buf_ == nullptr)
      throw std::runtime_error("Cannot render VertexArray without positions!");
    Render(0, pos_buf_->GetSize());
  }
}

static_assert(std::is_move_constructible<VertexArray>(), "");
static_assert(std::is_move_assignable<VertexArray>(), "");

static_assert(!std::is_copy_constructible<VertexArray>(), "");
static_assert(!std::is_copy_assignable<VertexArray>(), "");
}  // namespace GLOO
