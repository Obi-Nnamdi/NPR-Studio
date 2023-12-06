#include "ArcBallCameraNode.hpp"

#include <iostream>

#include <glm/gtx/quaternion.hpp>

#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/VertexObject.hpp"

namespace GLOO {
ArcBallCameraNode::ArcBallCameraNode(float fov, float aspect, float distance)
    : SceneNode(), fov_(fov), distance_(distance) {
  auto camera = make_unique<CameraComponent>(fov, aspect, 0.1f, 100.f);
  AddComponent(std::move(camera));

  start_position_ = GetTransform().GetPosition();
  start_rotation_ = GetTransform().GetRotation();
  start_distance_ = distance;

  PlotAxes();
}

void ArcBallCameraNode::Calibrate() {
  start_position_ = GetTransform().GetPosition();
  start_rotation_ = GetTransform().GetRotation();
}

void ArcBallCameraNode::Update(double delta_time) {
  UpdateViewport();

  auto& input_manager = InputManager::GetInstance();

  static bool prev_released = true;

  if (input_manager.IsMiddleMousePressed()) {
    if (prev_released) {
      mouse_start_click_ = InputManager::GetInstance().GetCursorPosition();
    }
    PlaneTranslation(InputManager::GetInstance().GetCursorPosition());
    prev_released = false;
  } else if (input_manager.IsLeftMousePressed()) {
    if (prev_released) {
      mouse_start_click_ = InputManager::GetInstance().GetCursorPosition();
    }
    ArcBallRotation(InputManager::GetInstance().GetCursorPosition());
    prev_released = false;
  } else if (input_manager.IsRightMousePressed()) {
    if (prev_released) {
      mouse_start_click_ = InputManager::GetInstance().GetCursorPosition();
    }
    DistanceZoom(InputManager::GetInstance().GetCursorPosition());
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyPressed('A')) {
    if (prev_released) {
      ToggleAxes();
    }
    prev_released = false;
  }  else {
    auto scroll = input_manager.FetchAndResetMouseScroll();
    if (scroll != 0.0) {
      DistanceZoom(-float(scroll) * 0.1f);
    }
    prev_released = true;
    start_position_ = GetTransform().GetPosition();
    start_rotation_ = GetTransform().GetRotation();
    start_distance_ = distance_;
  }

  auto V = make_unique<glm::mat4>(glm::lookAt(
      glm::vec3(0, 0, distance_), glm::vec3(0), glm::vec3(0, 1.f, 0)));
  *V *= glm::toMat4(GetTransform().GetRotation()) *
        glm::translate(glm::mat4(1.f), GetTransform().GetPosition());
  GetComponentPtr<CameraComponent>()->SetViewMatrix(std::move(V));
}

void ArcBallCameraNode::UpdateViewport() {
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  float aspect_ratio =
      static_cast<float>(window_size.x) / static_cast<float>(window_size.y);
  GetComponentPtr<CameraComponent>()->SetAspectRatio(aspect_ratio);
}

void ArcBallCameraNode::ArcBallRotation(glm::dvec2 pos) {
  float sx, sy, sz, ex, ey, ez;
  float scale;
  float sl, el;
  float dotprod;
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();

  // find vectors from center of window
  sx = float(mouse_start_click_.x - (window_size.x / 2.f));
  sy = float(mouse_start_click_.y - (window_size.y / 2.f));
  ex = float(pos.x - (window_size.x / 2.f));
  ey = float(pos.y - (window_size.y / 2.f));

  // invert y coordinates (raster versus device coordinates)
  sy = -sy;
  ey = -ey;

  // scale by inverse of size of window and magical sqrt2 factor
  scale = fmin(float(window_size.x), float(window_size.y));

  scale = 1.f / scale;

  sx *= scale;
  sy *= scale;
  ex *= scale;
  ey *= scale;

  // project points to unit circle
  sl = hypot(sx, sy);
  el = hypot(ex, ey);

  if (sl > 1.f) {
    sx /= sl;
    sy /= sl;
    sl = 1.0;
  }
  if (el > 1.f) {
    ex /= el;
    ey /= el;
    el = 1.f;
  }

  // project up to unit sphere - find Z coordinate
  sz = sqrt(1.0f - sl * sl);
  ez = sqrt(1.0f - el * el);

  // rotate (sx,sy,sz) into (ex,ey,ez)

  // compute angle from dot-product of unit vectors (and double it).
  // compute axis from cross product.
  dotprod = sx * ex + sy * ey + sz * ez;

  if (dotprod != 1 && !std::isnan(dotprod)) {
    glm::vec3 axis(sy * ez - ey * sz, sz * ex - ez * sx, sx * ey - ex * sy);
    axis = glm::normalize(axis);
    if (std::isnan(axis.x) || std::isnan(axis.y) || std::isnan(axis.z)) {
      return;
    }

    float angle = 2.0f * acos(dotprod);

    GetTransform().SetRotation(glm::angleAxis(angle, axis) * start_rotation_);
  }
}

void ArcBallCameraNode::PlaneTranslation(glm::dvec2 pos) {
  // compute "distance" of image plane (wrt projection matrix)
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  float d = static_cast<float>(window_size.y) / 2.0f /
            tan(fov_ * kPi / 180.0f / 2.0f);

  // compute up plane intersect of clickpoint (wrt fovy)
  float su = float(-mouse_start_click_.y + window_size.y / 2.0f);
  float cu = float(-pos.y + window_size.y / 2.0f);

  // compute right plane intersect of clickpoint (ASSUMED FOVY is 1)
  float sr = float(mouse_start_click_.x - window_size.x / 2.0f);
  float cr = float(pos.x - window_size.x / 2.0f);

  // this maps move
  glm::vec2 move(cr - sr, cu - su);
  move *= -distance_ / d;

  auto rot = glm::toMat4(GetTransform().GetRotation());
  GetTransform().SetPosition(start_position_ -
                             (move.x * glm::vec3(glm::transpose(rot)[0]) +
                              move.y * glm::vec3(glm::transpose(rot)[1])));
}

void ArcBallCameraNode::DistanceZoom(glm::dvec2 pos) {
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  DistanceZoom(float((pos.y - mouse_start_click_.y) / window_size.y));
}

void ArcBallCameraNode::DistanceZoom(float delta) {
  distance_ = start_distance_ * exp(delta);
}

void ArcBallCameraNode::PlotAxes() {
  auto x_line = std::make_shared<VertexObject>();
  auto y_line = std::make_shared<VertexObject>();
  auto z_line = std::make_shared<VertexObject>();

  auto line_shader = std::make_shared<SimpleShader>();

  auto indices = IndexArray();
  indices.push_back(0);
  indices.push_back(1);

  x_line->UpdateIndices(make_unique<IndexArray>(indices));
  y_line->UpdateIndices(make_unique<IndexArray>(indices));
  z_line->UpdateIndices(make_unique<IndexArray>(indices));

  auto x_positions = make_unique<PositionArray>();
  x_positions->push_back(glm::vec3(-1.f, 0, 0));
  x_positions->push_back(glm::vec3(1.f, 0, 0));
  x_line->UpdatePositions(std::move(x_positions));

  auto y_positions = make_unique<PositionArray>();
  y_positions->push_back(glm::vec3(0, -1.f, 0));
  y_positions->push_back(glm::vec3(0, 1.f, 0));
  y_line->UpdatePositions(std::move(y_positions));

  auto z_positions = make_unique<PositionArray>();
  z_positions->push_back(glm::vec3(0, 0, -1.f));
  z_positions->push_back(glm::vec3(0, 0, 1.f));
  z_line->UpdatePositions(std::move(z_positions));

  auto line_node = make_unique<SceneNode>();
  line_node->CreateComponent<ShadingComponent>(line_shader);
  auto& rc_x_line = line_node->CreateComponent<RenderingComponent>(x_line);
  rc_x_line.SetDrawMode(DrawMode::Lines);
  auto color = glm::vec3(1.f, 0.f, 0.f);
  auto material = std::make_shared<Material>(color, color, color, 0.0f);
  line_node->CreateComponent<MaterialComponent>(material);
  x_line_node_ptr_ = line_node.get();
  x_line_node_ptr_->SetActive(false);
  AddChild(std::move(line_node));

  line_node = make_unique<SceneNode>();
  line_node->CreateComponent<ShadingComponent>(line_shader);
  auto& rc_y_line = line_node->CreateComponent<RenderingComponent>(y_line);
  rc_y_line.SetDrawMode(DrawMode::Lines);
  color = glm::vec3(0.f, 1.f, 0.f);
  material = std::make_shared<Material>(color, color, color, 0.0f);
  line_node->CreateComponent<MaterialComponent>(material);
  y_line_node_ptr_ = line_node.get();
  y_line_node_ptr_->SetActive(false);
  AddChild(std::move(line_node));

  line_node = make_unique<SceneNode>();
  line_node->CreateComponent<ShadingComponent>(line_shader);
  auto& rc_z_line = line_node->CreateComponent<RenderingComponent>(z_line);
  rc_z_line.SetDrawMode(DrawMode::Lines);
  color = glm::vec3(0.f, 0.f, 1.f);
  material = std::make_shared<Material>(color, color, color, 0.0f);
  line_node->CreateComponent<MaterialComponent>(material);
  z_line_node_ptr_ = line_node.get();
  z_line_node_ptr_->SetActive(false);
  AddChild(std::move(line_node));
}

void ArcBallCameraNode::ToggleAxes() {
  x_line_node_ptr_->SetActive(!x_line_node_ptr_->IsActive());
  y_line_node_ptr_->SetActive(!y_line_node_ptr_->IsActive());
  z_line_node_ptr_->SetActive(!z_line_node_ptr_->IsActive());
}

}  // namespace GLOO
