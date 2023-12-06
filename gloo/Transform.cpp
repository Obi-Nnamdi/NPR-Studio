#include "Transform.hpp"

#include <memory>
#include <stdexcept>

#include <glm/gtx/matrix_decompose.hpp>

#include "gloo/SceneNode.hpp"

namespace GLOO {
Transform::Transform(SceneNode& node)
    : position_(0.f),
      rotation_(glm::quat(1.f, 0.f, 0.f, 0.f)),
      scale_(glm::vec3(1.f)),
      node_(node) {
  UNUSED(node_);
  UpdateLocalTransformMatrix();
}

void Transform::SetPosition(const glm::vec3& position) {
  position_ = position;
  UpdateLocalTransformMatrix();
}

void Transform::SetRotation(const glm::quat& rotation) {
  rotation_ = rotation;
  UpdateLocalTransformMatrix();
}

void Transform::SetRotation(const glm::vec3& axis, float angle) {
  SetRotation(glm::quat(cosf(angle / 2), axis.x * sinf(angle / 2),
                        axis.y * sinf(angle / 2), axis.z * sinf(angle / 2)));
}

void Transform::SetScale(const glm::vec3& scale) {
  scale_ = scale;
  UpdateLocalTransformMatrix();
}

void Transform::SetMatrix4x4(const glm::mat4& T) {
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(T, scale_, rotation_, position_, skew, perspective);
  // Won't use skew or perspective.
  UpdateLocalTransformMatrix();
}

glm::vec3 Transform::GetForwardDirection() const {
  return glm::mat3_cast(rotation_) * GetWorldForward();
}

glm::vec3 Transform::GetUpDirection() const {
  return glm::mat3_cast(rotation_) * GetWorldUp();
}

glm::vec3 Transform::GetRightDirection() const {
  return glm::mat3_cast(rotation_) * GetWorldRight();
}

glm::vec3 Transform::GetWorldUp() {
  return glm::vec3(0.f, 1.f, 0.f);
}

glm::vec3 Transform::GetWorldRight() {
  return glm::vec3(1.f, 0.f, 0.f);
}

glm::vec3 Transform::GetWorldForward() {
  return glm::vec3(0.f, 0.f, -1.f);
}

glm::vec3 Transform::GetWorldPosition() const {
  return glm::vec3(GetLocalToWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

glm::mat4 Transform::GetLocalToParentMatrix() const {
  return local_transform_mat_;
}

glm::mat4 Transform::GetLocalToAncestorMatrix(SceneNode* ancestor) const {
  SceneNode* parent = node_.GetParentPtr();
  if (parent == ancestor) {
    return local_transform_mat_;
  } else {
    if (parent == nullptr) {
      throw std::runtime_error("Ancestor does not exist!");
    }
    return parent->GetTransform().GetLocalToAncestorMatrix(ancestor) *
           local_transform_mat_;
  }
}

glm::mat4 Transform::GetLocalToWorldMatrix() const {
  return GetLocalToAncestorMatrix(nullptr);
}

void Transform::UpdateLocalTransformMatrix() {
  glm::mat4 new_matrix(1.f);

  // Order: scale, rotate, translate
  new_matrix = glm::scale(glm::mat4(1.f), scale_) * new_matrix;
  new_matrix = glm::mat4_cast(rotation_) * new_matrix;
  new_matrix = glm::translate(glm::mat4(1.f), position_) * new_matrix;

  local_transform_mat_ = std::move(new_matrix);
}
}  // namespace GLOO
