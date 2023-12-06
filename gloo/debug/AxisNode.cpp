#include "AxisNode.hpp"

#include "gloo/VertexObject.hpp"
#include "gloo/Material.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"

namespace GLOO {
AxisNode::AxisNode(int key) : key_(key) {
  PlotAxes();
}

void AxisNode::Update(double delta_time) {
  static bool prev_released = true;
  if (InputManager::GetInstance().IsKeyPressed(key_)) {
    if (prev_released) {
      ToggleAxes();
    }
    prev_released = false;
  } else {
    prev_released = true;
  }
}

void AxisNode::ToggleAxes() {
  x_line_node_ptr_->SetActive(!x_line_node_ptr_->IsActive());
  y_line_node_ptr_->SetActive(!y_line_node_ptr_->IsActive());
  z_line_node_ptr_->SetActive(!z_line_node_ptr_->IsActive());
}

void AxisNode::PlotAxes() {
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
}  // namespace GLOO
