#ifndef GLOO_IMAGE_H_
#define GLOO_IMAGE_H_

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

namespace GLOO {
class Image {
 public:
  Image(size_t width, size_t height) {
    width_ = width;
    height_ = height;
    data_.resize(width_ * height_);
  }

  size_t GetWidth() const {
    return width_;
  }

  size_t GetHeight() const {
    return height_;
  }

  void SetPixel(size_t x, size_t y, const glm::vec3& color) {
    if (x < width_ && y < height_) {
      data_[y * width_ + x] = color;
    } else {
      throw std::runtime_error("Unable to set a pixel outside of image range.");
    }
  }

  const glm::vec3& GetPixel(size_t x, size_t y) const {
    if (x < width_ && y < height_) {
      return data_[y * width_ + x];
    } else {
      std::cout << "(" << x << "," << y << ")" << std::endl;
      throw std::runtime_error("Unable to get a pixel outside of image range.");
    }
  }

  static std::unique_ptr<Image> LoadPNG(const std::string& filename,
                                        bool y_reversed);
  void SavePNG(const std::string& filename) const;
  std::vector<uint8_t> ToByteData() const;
  std::vector<float> ToFloatData() const;

 private:
  std::vector<glm::vec3> data_;
  size_t width_;
  size_t height_;
};
}  // namespace GLOO

#endif
