#ifndef GLOO_TEXTURE_H_
#define GLOO_TEXTURE_H_

#include <unordered_map>

#include "gloo/external.hpp"
#include "gloo/Image.hpp"

namespace GLOO {
using TextureConfig = std::unordered_map<GLenum, GLint>;
class Texture {
 public:
  Texture();
  Texture(const TextureConfig& config);
  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  // Allow both move-construct and move-assign.
  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;

  // Bind the current texture to a texture unit ("id")
  void BindToUnit(int id) const;
  // Update the texture contents with "image"
  void UpdateImage(const Image& image);
  // Allocate space for the texture without storing the data
  void Reserve(GLint internal_format,
               size_t width,
               size_t height,
               GLenum format,
               GLenum type);
  GLuint GetHandle() const {
    return handle_;
  }

 private:
  void Initialize(const TextureConfig& config);
  static const TextureConfig& GetDefaultConfig();

  GLuint handle_{GLuint(-1)};
};
}  // namespace GLOO

#endif
