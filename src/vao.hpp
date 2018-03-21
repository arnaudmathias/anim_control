#pragma once
#include <algorithm>
#include <vector>
#include "anim.hpp"
#include "env.hpp"

struct VAO {
  VAO(const std::vector<Vertex>& vertices);
  VAO(const std::vector<Vertex>& vertices,
      const std::vector<unsigned int>& indices);
  VAO(const std::vector<glm::vec3>& positions);
  VAO(const std::vector<glm::vec4>& positions);
  ~VAO();
  void update(const std::vector<glm::vec3>& positions);
  void update(const std::vector<Vertex>& vertices);
  GLuint vao;
  GLsizei vertices_size;
  GLsizei indices_size;

 private:
  void initialize();
  GLuint _vbo;
  GLuint _ebo;
};
