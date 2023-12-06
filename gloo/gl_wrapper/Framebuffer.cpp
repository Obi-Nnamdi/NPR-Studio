#include "Framebuffer.hpp"

#include <stdexcept>

#include "BindGuard.hpp"
#include "gloo/utils.hpp"

namespace GLOO {
Framebuffer::Framebuffer() {
  GL_CHECK(glGenFramebuffers(1, &handle_));
}

Framebuffer::~Framebuffer() {
  if (handle_ != GLuint(-1))
    GL_CHECK(glDeleteFramebuffers(1, &handle_));
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept {
  handle_ = other.handle_;
  other.handle_ = GLuint(-1);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
  handle_ = other.handle_;
  other.handle_ = GLuint(-1);
  return *this;
}

void Framebuffer::Bind() const {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, handle_));
}

void Framebuffer::Unbind() const {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Framebuffer::AssociateTexture(const Texture& texture, GLenum attachment) {
  Bind();
  GL_CHECK(glFramebufferTexture2D(
      /* target = */ GL_FRAMEBUFFER,
      /* attachment = */ attachment,
      /* textarget = */ GL_TEXTURE_2D,
      /* texture = */ texture.GetHandle(),
      /* level = */ 0));
  Unbind();
}

static_assert(std::is_move_constructible<Framebuffer>(), "");
static_assert(std::is_move_assignable<Framebuffer>(), "");

static_assert(!std::is_copy_constructible<Framebuffer>(), "");
static_assert(!std::is_copy_assignable<Framebuffer>(), "");
}  // namespace GLOO
