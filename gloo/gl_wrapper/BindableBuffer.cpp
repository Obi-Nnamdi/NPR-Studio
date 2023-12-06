#include "BindableBuffer.hpp"

#include <type_traits>

#include "gloo/utils.hpp"

namespace GLOO {
BindableBuffer::BindableBuffer(GLenum target) : target_(target) {
  GL_CHECK(glGenBuffers(1, &handle_));
}

BindableBuffer::~BindableBuffer() {
  Reset();
}

BindableBuffer::BindableBuffer(BindableBuffer&& other) noexcept {
  handle_ = other.Release();
  target_ = other.target_;
}

BindableBuffer& BindableBuffer::operator=(BindableBuffer&& other) noexcept {
  Reset(other.Release());
  target_ = other.target_;
  return *this;
}

void BindableBuffer::Reset(GLuint handle) {
  GL_CHECK(glDeleteBuffers(1, &handle_));
  handle_ = handle;
}

GLuint BindableBuffer::Release() {
  GLuint handle = handle_;
  handle_ = 0;
  return handle;
}

void BindableBuffer::Bind() const {
  GL_CHECK(glBindBuffer(target_, handle_));
}

void BindableBuffer::Unbind() const {
  GL_CHECK(glBindBuffer(target_, 0));
}

static_assert(std::is_move_constructible<BindableBuffer>(), "");
static_assert(std::is_move_assignable<BindableBuffer>(), "");

static_assert(!std::is_copy_constructible<BindableBuffer>(), "");
static_assert(!std::is_copy_assignable<BindableBuffer>(), "");
}  // namespace GLOO
