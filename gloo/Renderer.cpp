
#include <cassert>
#include <iostream>
#include <glad/glad.h>
#include <glm/gtx/string_cast.hpp>

#include "Application.hpp"
#include "Scene.hpp"
#include "utils.hpp"
#include "gl_wrapper/BindGuard.hpp"
#include "shaders/ShaderProgram.hpp"
#include "components/ShadingComponent.hpp"
#include "components/CameraComponent.hpp"
#include "debug/PrimitiveFactory.hpp"

namespace {
const size_t kShadowWidth = 4096;
const size_t kShadowHeight = 4096;
const glm::mat4 kLightProjection =
    glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 80.0f);
}  // namespace

namespace GLOO {
Renderer::Renderer(Application& application) : application_(application) {
  UNUSED(application_);
  // Reserve Space for Shadow Depth Texture
  shadow_depth_tex_ = make_unique<Texture>();
  shadow_depth_tex_->Reserve(GL_DEPTH_COMPONENT, kShadowWidth, kShadowHeight, GL_DEPTH_COMPONENT,
                             GL_FLOAT);

  shadow_buffer_ = make_unique<Framebuffer>();
  // Add texture to framebuffer for rendering
  shadow_buffer_->AssociateTexture(*shadow_depth_tex_, GL_DEPTH_ATTACHMENT);

  shadow_shader_ = make_unique<ShadowShader>();
  // To render a quad on in the lower-left of the screen, you can assign texture
  // to quad_ created below and then call quad_->GetVertexArray().Render().
  plain_texture_shader_ = make_unique<PlainTextureShader>();
  quad_ = PrimitiveFactory::CreateQuad();
}

void Renderer::SetRenderingOptions() const {
  GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

  // Enable depth test.
  GL_CHECK(glEnable(GL_DEPTH_TEST));
  GL_CHECK(glDepthFunc(GL_LEQUAL));

  // Enable blending for multi-pass forward rendering.
  GL_CHECK(glEnable(GL_BLEND));
  GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
}

void Renderer::Render(const Scene& scene) const {
  SetRenderingOptions();
  RenderScene(scene);
}

void Renderer::RecursiveRetrieve(const SceneNode& node,
                                 RenderingInfo& info,
                                 const glm::mat4& model_matrix) {
  // model_matrix is parent to world transformation.
  glm::mat4 new_matrix =
      model_matrix * node.GetTransform().GetLocalToParentMatrix();
  auto robj_ptr = node.GetComponentPtr<RenderingComponent>();
  if (robj_ptr != nullptr && node.IsActive())
    info.emplace_back(robj_ptr, new_matrix);

  size_t child_count = node.GetChildrenCount();
  for (size_t i = 0; i < child_count; i++) {
    RecursiveRetrieve(node.GetChild(i), info, new_matrix);
  }
}

Renderer::RenderingInfo Renderer::RetrieveRenderingInfo(
    const Scene& scene) const {
  RenderingInfo info;
  const SceneNode& root = scene.GetRootNode();
  // Efficient implementation without redundant matrix multiplications.
  RecursiveRetrieve(root, info, glm::mat4(1.0f));
  return info;
}

void Renderer::RenderScene(const Scene& scene) const {
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  const SceneNode& root = scene.GetRootNode();
  auto rendering_info = RetrieveRenderingInfo(scene);
  auto light_ptrs = root.GetComponentPtrsInChildren<LightComponent>();
  if (light_ptrs.size() == 0) {
    // Make sure there are at least 2 passes of we don't forget to set color
    // mask back.
    return;
  }

  CameraComponent* camera = scene.GetActiveCameraPtr();

  {
    // Here we first do a depth pass (note that this has nothing to do with the
    // shadow map). The goal of this depth pass is to exclude pixels that are
    // not really visible from the camera, in later rendering passes. You can
    // safely leave this pass here without understanding/modifying it, for
    // assignment 5. If you are interested in learning more, see
    // https://www.khronos.org/opengl/wiki/Early_Fragment_Test#Optimization

    GL_CHECK(glDepthMask(GL_TRUE));
    bool color_mask = GL_FALSE;
    GL_CHECK(glColorMask(color_mask, color_mask, color_mask, color_mask));

    for (const auto& pr : rendering_info) {
      auto robj_ptr = pr.first;
      SceneNode& node = *robj_ptr->GetNodePtr();
      auto shading_ptr = node.GetComponentPtr<ShadingComponent>();
      if (shading_ptr == nullptr) {
        std::cerr << "Some mesh is not attached with a shader during rendering!"
                  << std::endl;
        continue;
      }
      ShaderProgram* shader = shading_ptr->GetShaderPtr();

      BindGuard shader_bg(shader);

      // Set various uniform variables in the shaders.
      shader->SetTargetNode(node, pr.second);
      shader->SetCamera(*camera);

      robj_ptr->Render();
    }
  }

  // The real shadow map/Phong shading passes.
  for (size_t light_id = 0; light_id < light_ptrs.size(); light_id++) {
    LightComponent& light = *light_ptrs.at(light_id);
    // Render shadow maps for lights that can cast shadows
    glm::mat4 world_to_light_ndc_matrix;
    if (light.CanCastShadow()) {
      // Create world_to_light_ndc matrix and render shadow
      auto light_node = light.GetNodePtr();
      glm::mat4 light_view_matrix =
          glm::inverse(light_node->GetTransform().GetLocalToWorldMatrix());
      world_to_light_ndc_matrix = kLightProjection * light_view_matrix;
      RenderShadow(world_to_light_ndc_matrix, rendering_info);
    }

    GL_CHECK(glDepthMask(GL_FALSE));
    bool color_mask = GL_TRUE;
    GL_CHECK(glColorMask(color_mask, color_mask, color_mask, color_mask));

    for (const auto& pr : rendering_info) {
      auto robj_ptr = pr.first;
      SceneNode& node = *robj_ptr->GetNodePtr();
      auto shading_ptr = node.GetComponentPtr<ShadingComponent>();
      if (shading_ptr == nullptr) {
        std::cerr << "Some mesh is not attached with a shader during rendering!"
                  << std::endl;
        continue;
      }
      ShaderProgram* shader = shading_ptr->GetShaderPtr();

      BindGuard shader_bg(shader);

      // Set various uniform variables in the shaders.
      shader->SetTargetNode(node, pr.second);
      shader->SetCamera(*camera);
      shader->SetLightSource(light);
      // Pass in the shadow texture to the shader via SetShadowMapping if
      // the light can cast shadow.
      if (light.CanCastShadow()) {
        shader->SetShadowMapping(*shadow_depth_tex_, world_to_light_ndc_matrix);
      }

      robj_ptr->Render();
    }
  }

  // Re-enable writing to depth buffer.
  GL_CHECK(glDepthMask(GL_TRUE));
}

void Renderer::RenderShadow(const glm::mat4& world_to_light_ndc_matrix,
                            const RenderingInfo& rendering_info) const {
  // Direct OpenGL to render to shadow buffer (and automatically unbind)
  BindGuard shadow_buffer_bg(shadow_buffer_.get());

  // Set up shadow map rendering context
  GL_CHECK(glViewport(0, 0, kShadowWidth, kShadowHeight));
  GL_CHECK(glDepthMask(GL_TRUE));
  // don't render colors to shadow buffer
  GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
  // clear depth buffer for shadow buffer (only do this when the framebuffer is complete)
  GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));

  BindGuard shader_bg(shadow_shader_.get());  // binder guard for shader

  // Set the light we're rendering from in shadow shader
  shadow_shader_->SetWorldToLightMatrix(world_to_light_ndc_matrix);

  // Render each object using the shadow shader
  for (const auto& pr : rendering_info) {
    auto robj_ptr = pr.first;
    SceneNode& node = *robj_ptr->GetNodePtr();

    // Set uniform variables in shadow shader.
    shadow_shader_->SetTargetNode(node, pr.second);
    robj_ptr->Render();
  }

  // Reset viewport size for regular rendering
  glm::ivec2 window_size = application_.GetWindowSize();
  GL_CHECK(glViewport(0, 0, window_size.x, window_size.y));
}

void Renderer::RenderTexturedQuad(const Texture& texture, bool is_depth) const {
  BindGuard shader_bg(plain_texture_shader_.get());
  plain_texture_shader_->SetVertexObject(*quad_);
  plain_texture_shader_->SetTexture(texture, is_depth);
  quad_->GetVertexArray().Render();
}

void Renderer::DebugShadowMap() const {
  GL_CHECK(glDisable(GL_DEPTH_TEST));
  GL_CHECK(glDisable(GL_BLEND));

  glm::ivec2 window_size = application_.GetWindowSize();
  glViewport(0, 0, window_size.x / 4, window_size.y / 4);
  RenderTexturedQuad(*shadow_depth_tex_, true);

  glViewport(0, 0, window_size.x, window_size.y);
}
}  // namespace GLOO
