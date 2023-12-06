#ifndef GLOO_FRAMEBUFFER_H_
#define GLOO_FRAMEBUFFER_H_

#include "BindGuard.hpp"
#include "gloo/external.hpp"
#include "Texture.hpp"

namespace GLOO {
class Framebuffer : public IBindable {
 public:
  Framebuffer();
  ~Framebuffer();

  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;

  // Allow both move-construct and move-assign.
  Framebuffer(Framebuffer&& other) noexcept;
  Framebuffer& operator=(Framebuffer&& other) noexcept;

  void Bind() const override;
  void Unbind() const override;
  void AssociateTexture(const Texture& texture, GLenum attachment);

 private:
  GLuint handle_{GLuint(-1)};
};
}  // namespace GLOO

#endif
