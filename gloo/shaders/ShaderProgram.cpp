#include "ShaderProgram.hpp"

#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>

#include <gloo/utils.hpp>

namespace GLOO {
ShaderProgram::ShaderProgram(
    const std::unordered_map<GLenum, std::string>& shader_filenames) {
  assert(shader_filenames.count(GL_VERTEX_SHADER) == 1);
  assert(shader_filenames.count(GL_FRAGMENT_SHADER) == 1);
  for (auto& kv : shader_filenames) {
    std::string shader_path = GetShaderGLSLDir() + kv.second;
    std::ifstream ifs(shader_path, std::ifstream::in);
    std::string shader_code(std::istreambuf_iterator<char>{ifs}, {});
    shader_handles_[kv.first] = LoadShader(kv.first, shader_code, shader_path);
  }

  shader_program_ = glCreateProgram();
  GL_CHECK_ERROR();

  for (auto& kv : shader_handles_) {
    GL_CHECK(glAttachShader(shader_program_, kv.second));
  }

  GL_CHECK(glLinkProgram(shader_program_));
  GLint link_status;
  GL_CHECK(glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status));
  if (link_status != GL_TRUE) {
    GLchar err_log_buf[kErrorLogBufferSize];
    GL_CHECK(glGetProgramInfoLog(shader_program_, kErrorLogBufferSize, nullptr,
                                 err_log_buf));
    std::cerr << "Shader linking error: " << err_log_buf << std::endl;
    return;
  }

  // Cleanup after linking.
  for (auto& kv : shader_handles_) {
    GLuint handle = kv.second;
    GL_CHECK(glDetachShader(shader_program_, handle));
    GL_CHECK(glDeleteShader(handle));
  }
}

ShaderProgram::~ShaderProgram() {
  GL_CHECK(glDeleteProgram(shader_program_));
}

void ShaderProgram::Bind() const {
  GL_CHECK(glUseProgram(shader_program_));
}

void ShaderProgram::Unbind() const {
  GL_CHECK(glUseProgram(0));
}

GLint ShaderProgram::GetAttributeLocation(const std::string& name) const {
  GLint loc = glGetAttribLocation(shader_program_, name.c_str());
  GL_CHECK_ERROR();
  return loc;
}

GLuint ShaderProgram::LoadShader(GLenum type,
                                 std::string shader_code,
                                 const std::string& shader_file_name) {
  GLuint shader_handle = glCreateShader(type);
  GL_CHECK_ERROR();
  auto version_pos = shader_code.find("#version");
  if (version_pos == std::string::npos) {
    throw std::runtime_error("Shader file " + shader_file_name +
                             " has no #version!");
  }
  auto version_end = shader_code.find('\n', version_pos);
  std::string version =
      shader_code.substr(version_pos, version_end + 1 - version_pos);
  shader_code = shader_code.substr(version_end + 1);
  std::vector<const char*> codes = {version.c_str(), shader_code.c_str()};

  codes.insert(codes.begin() + 1, "#define ASSIGNMENT_5_STARTER\n");
  GL_CHECK(glShaderSource(shader_handle, (GLsizei)codes.size(), codes.data(),
                          nullptr));
  GL_CHECK(glCompileShader(shader_handle));

  GLint compile_status;
  GL_CHECK(glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status));
  if (compile_status != GL_TRUE) {
    char err_log_buf[kErrorLogBufferSize];
    GL_CHECK(glGetShaderInfoLog(shader_handle, kErrorLogBufferSize, nullptr,
                                err_log_buf));
    std::cerr << "Shader compilation error: " << err_log_buf << std::endl;
    return 0;
  }

  return shader_handle;
}

void ShaderProgram::SetUniform(const std::string& name,
                               const glm::mat4& value) const {
  GLint loc = glGetUniformLocation(shader_program_, name.c_str());
  GL_CHECK_ERROR();
  GL_CHECK(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value)));
}

void ShaderProgram::SetUniform(const std::string& name,
                               const glm::mat3& value) const {
  GLint loc = glGetUniformLocation(shader_program_, name.c_str());
  GL_CHECK_ERROR();
  GL_CHECK(glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value)));
}

void ShaderProgram::SetUniform(const std::string& name,
                               const glm::vec3& value) const {
  GLint loc = glGetUniformLocation(shader_program_, name.c_str());
  GL_CHECK_ERROR();
  GL_CHECK(glUniform3fv(loc, 1, glm::value_ptr(value)));
}

void ShaderProgram::SetUniform(const std::string& name, float value) const {
  GLint loc = glGetUniformLocation(shader_program_, name.c_str());
  GL_CHECK_ERROR();
  GL_CHECK(glUniform1f(loc, value));
}

void ShaderProgram::SetUniform(const std::string& name, int value) const {
  GLint loc = glGetUniformLocation(shader_program_, name.c_str());
  GL_CHECK_ERROR();
  GL_CHECK(glUniform1i(loc, value));
}
}  // namespace GLOO
