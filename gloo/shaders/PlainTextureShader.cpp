#include "PlainTextureShader.hpp"

#include "gloo/SceneNode.hpp"
#include "gloo/components/RenderingComponent.hpp"

namespace GLOO {
PlainTextureShader::PlainTextureShader()
    : ShaderProgram(std::unordered_map<GLenum, std::string>{
          {GL_VERTEX_SHADER, "plain_texture.vert"},
          {GL_FRAGMENT_SHADER, "plain_texture.frag"}}) {
}

void PlainTextureShader::AssociateVertexArray(
    const VertexArray& vertex_array) const {
  if (!vertex_array.HasPositionBuffer()) {
    throw std::runtime_error("Plain texture shader requires vertex positions!");
  }
  if (!vertex_array.HasTexCoordBuffer()) {
    throw std::runtime_error(
        "Plain texture shader requires vertex texture coordinates!");
  }
  vertex_array.LinkPositionBuffer(GetAttributeLocation("vertex_ndc_position"));
  vertex_array.LinkTexCoordBuffer(GetAttributeLocation("vertex_tex_coord"));
}

void PlainTextureShader::SetVertexObject(const VertexObject& obj) const {
  AssociateVertexArray(obj.GetVertexArray());
}

void PlainTextureShader::SetTexture(const Texture& texture,
                                    bool is_depth) const {
  SetUniform("is_depth", is_depth);
  texture.BindToUnit(0);
  SetUniform("in_texture", 0);
}
}  // namespace GLOO
