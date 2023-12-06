#ifndef PLAIN_TEXTURE_SHADER_H_
#define PLAIN_TEXTURE_SHADER_H_

#include "ShaderProgram.hpp"

#include "gloo/gl_wrapper/Texture.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
class PlainTextureShader : public ShaderProgram {
 public:
  PlainTextureShader();

  void SetVertexObject(const VertexObject& obj) const;
  void SetTexture(const Texture& texture, bool is_depth) const;

 private:
  void AssociateVertexArray(const VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
